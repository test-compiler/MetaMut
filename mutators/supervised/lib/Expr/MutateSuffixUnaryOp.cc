#include "Expr/MutateSuffixUnaryOp.h"
#include "MutatorManager.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;
using namespace ysmut;

static RegisterMutator<MutateSuffixUnaryOp> M(
    "mutate-suffix-unaryop", "Change a UnaryOperator's suffix operator.");

bool MutateSuffixUnaryOp::VisitUnaryOperator(UnaryOperator *UO) {
  if (UO->isPostfix() && isMutationSite(UO)) TheUnaryOps.push_back(UO);
  return true;
}

bool MutateSuffixUnaryOp::mutate() {
  TraverseAST(getASTContext());
  if (TheUnaryOps.empty()) return false;

  UnaryOperator *unaryOp = randElement(TheUnaryOps);
  UnaryOperatorKind oldOp = unaryOp->getOpcode();

  // Only handle postfix increment and decrement
  if (oldOp != UO_PostInc && oldOp != UO_PostDec) return false;

  // Change the operator
  UnaryOperatorKind newOp = (oldOp == UO_PostInc) ? UO_PostDec : UO_PostInc;
  std::string newOpStr = (newOp == UO_PostInc) ? "++" : "--";

  getRewriter().ReplaceText(unaryOp->getSourceRange(),
      getSourceText(unaryOp->getSubExpr()).str() + newOpStr);

  return true;
}
