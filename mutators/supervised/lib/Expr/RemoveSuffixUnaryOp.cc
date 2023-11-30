#include "Expr/RemoveSuffixUnaryOp.h"
#include "MutatorManager.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;
using namespace ysmut;

static RegisterMutator<RemoveSuffixUnaryOp> M(
    "remove-suffix-unaryop", "Remove a UnaryOperator's suffix operator.");

bool RemoveSuffixUnaryOp::VisitUnaryOperator(UnaryOperator *UO) {
  if (UO->isPostfix() && isMutationSite(UO)) TheUnaryOps.push_back(UO);
  return true;
}

bool RemoveSuffixUnaryOp::mutate() {
  TraverseAST(getASTContext());
  if (TheUnaryOps.empty()) return false;

  UnaryOperator *unaryOp = randElement(TheUnaryOps);
  UnaryOperatorKind oldOp = unaryOp->getOpcode();

  // Only handle postfix increment and decrement
  if (oldOp != UO_PostInc && oldOp != UO_PostDec) return false;

  // Remove the operator
  getRewriter().ReplaceText(
      unaryOp->getSourceRange(), getSourceText(unaryOp->getSubExpr()).str());

  return true;
}
