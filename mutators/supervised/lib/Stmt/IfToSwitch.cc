#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"
#include "Stmt/IfToSwitch.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<IfToSwitch> M("if-to-switch",
    "Convert a series of if-else statements to a switch statement.");

namespace {

class IntegerLiteralFinder : public RecursiveASTVisitor<IntegerLiteralFinder> {
public:
  IntegerLiteralFinder() : TheLiteral(nullptr) {}

  bool VisitIntegerLiteral(IntegerLiteral *IL) {
    if (!TheLiteral) { // store the first IntegerLiteral encountered
      TheLiteral = IL;
      return false; // stop traversing
    }
    return true; // keep traversing
  }

  IntegerLiteral *getTheLiteral() { return TheLiteral; }

private:
  IntegerLiteral *TheLiteral;
};

} // anonymous namespace

bool IfToSwitch::VisitIfStmt(IfStmt *IS) {
  if (isMutationSite(IS)) TheIfs.push_back(IS);
  return true;
}

bool IfToSwitch::mutate() {
  TraverseAST(getASTContext());
  if (TheIfs.empty()) return false;

  IfStmt *ifStmt = randElement(TheIfs);

  std::string switchText = "switch (";
  switchText += getSourceText(ifStmt->getCond()).str();
  switchText += ") {\n";

  for (Stmt *S = ifStmt; S != nullptr;) {
    IfStmt *IS = dyn_cast<IfStmt>(S);
    if (!IS) { // This handles the last else without a condition
      switchText += "default: \n";
      switchText += getSourceText(S).str();
      switchText += ";\n break;\n";
      break;
    } else {
      IntegerLiteralFinder Finder;
      Finder.TraverseStmt(IS->getCond());
      IntegerLiteral *IL = Finder.getTheLiteral();
      if (IL) {
        switchText += "case ";
        switchText += getSourceText(IL).str();
        switchText += ": \n";
        switchText += getSourceText(IS->getThen()).str();
        switchText += ";\n break;\n";
      }
      S = IS->getElse();
    }
  }

  switchText += "}";

  getRewriter().ReplaceText(ifStmt->getSourceRange(), switchText);

  return true;
}
