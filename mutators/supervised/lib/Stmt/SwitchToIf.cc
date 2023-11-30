#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"
#include "Stmt/SwitchToIf.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<SwitchToIf> M("switch-to-if",
    "Convert a switch statement to a series of if-else statements.");

bool SwitchToIf::VisitSwitchStmt(SwitchStmt *SS) {
  if (isMutationSite(SS)) TheSwitches.push_back(SS);
  return true;
}

bool SwitchToIf::mutate() {
  TraverseAST(getASTContext());
  if (TheSwitches.empty()) return false;

  SwitchStmt *sw = randElement(TheSwitches);

  std::string switchCondition = getSourceText(sw->getCond()).str();

  std::string ifElseText;
  bool isModified = false;
  for (auto *S : sw->getBody()->children()) {
    if (CaseStmt *CS = llvm::dyn_cast<CaseStmt>(S)) {
      ifElseText += "if (";
      ifElseText += switchCondition;
      ifElseText += " == ";
      ifElseText += getSourceText(CS->getLHS()).str();
      ifElseText += ") {\n";
      ifElseText += getSourceText(CS->getSubStmt()).str();
      ifElseText += ";";
      ifElseText += "\n} else ";
      isModified = true;
    } else if (DefaultStmt *DS = llvm::dyn_cast<DefaultStmt>(S)) {
      ifElseText += "{\n";
      ifElseText += getSourceText(DS->getSubStmt()).str();
      ifElseText += ";";
      ifElseText += "\n}";
      isModified = true;
    }
  }

  if (ifElseText.size() >= 5 &&
      ifElseText.substr(ifElseText.length() - 5) == "else ") {
    isModified = true;
    ifElseText =
        ifElseText.substr(0, ifElseText.length() - 5); // Remove the last "else"
  }

  getRewriter().ReplaceText(sw->getSourceRange(), ifElseText);

  return isModified;
}
