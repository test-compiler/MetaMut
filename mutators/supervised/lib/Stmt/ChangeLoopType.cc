#include "MutatorManager.h"
#include "Stmt/ChangeLoopType.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;
using namespace ysmut;

static RegisterMutator<ChangeLoopType> M(
    "change-looptype", "Change a loop type ('for' to 'while' and vice versa).");

bool ChangeLoopType::VisitForStmt(ForStmt *FS) {
  if (isMutationSite(FS)) TheForLoops.push_back(FS);
  return true;
}

bool ChangeLoopType::VisitWhileStmt(WhileStmt *WS) {
  if (isMutationSite(WS)) TheWhileLoops.push_back(WS);
  return true;
}

bool ChangeLoopType::mutate() {
  TraverseAST(getASTContext());
  if (TheForLoops.empty() && TheWhileLoops.empty()) return false;

  if (!TheForLoops.empty()) {
    // Change a 'for' loop into a 'while' loop
    ForStmt *FS = randElement(TheForLoops);
    std::string whileLoop;
    if (FS->getInit()) whileLoop += getSourceText(FS->getInit()).str() + ";\n";
    whileLoop += "while(" + getSourceText(FS->getCond()).str() + ") {\n";
    if (FS->getBody()) whileLoop += getSourceText(FS->getBody()).str();
    if (FS->getInc()) whileLoop += getSourceText(FS->getInc()).str() + ";\n";
    whileLoop += "}\n";
    getRewriter().ReplaceText(FS->getSourceRange(), whileLoop);
  } else {
    // Change a 'while' loop into a 'for' loop
    WhileStmt *WS = randElement(TheWhileLoops);
    std::string forLoop =
        "for (; " +
        Lexer::getSourceText(
            CharSourceRange::getTokenRange(WS->getCond()->getSourceRange()),
            getASTContext().getSourceManager(), getASTContext().getLangOpts())
            .str() +
        ";) " + getSourceText(WS->getBody()).str();
    getRewriter().ReplaceText(WS->getSourceRange(), forLoop);
  }

  return true;
}
