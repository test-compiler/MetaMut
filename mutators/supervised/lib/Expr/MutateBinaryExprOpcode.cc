#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Expr/MutateBinaryExprOpcode.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<MutateBinaryExprOpcode> M(
    "mutate-binop", "Change binary expression's operator.");

bool MutateBinaryExprOpcode::VisitBinaryOperator(BinaryOperator *BO) {
  if (isMutationSite(BO)) TheOperators.push_back(BO);
  return true;
}

bool MutateBinaryExprOpcode::mutate() {
  TraverseAST(getASTContext());
  if (TheOperators.empty()) return false;

  BinaryOperator *expr = randElement(TheOperators);

  // perform rewrite
  auto newOps = computeValidAlternatives(expr);
  if (newOps.size() == 0) return false;
  BinaryOperatorKind newOpKind = newOps[randIndex(newOps.size())];
  SourceLocation OpLoc = expr->getOperatorLoc();
  std::string NewOp = BinaryOperator::getOpcodeStr(newOpKind).str();
  getRewriter().ReplaceText(
      OpLoc, expr->getOpcodeStr(expr->getOpcode()).size(), NewOp);

  return true;
}

std::vector<BinaryOperatorKind>
MutateBinaryExprOpcode::computeValidAlternatives(BinaryOperator *op) {
  std::vector<BinaryOperatorKind> validAlternatives;

  Expr *lhs = op->getLHS();
  Expr *rhs = op->getRHS();
  SourceLocation loc = op->getExprLoc();

  auto &sema = getCompilerInstance().getSema();
  for (int i = BO_PtrMemD; i <= BO_Comma; ++i) {
    if (i == BO_PtrMemD || i == BO_PtrMemI || i == BO_Cmp ||
        i == op->getOpcode())
      continue;
    BinaryOperatorKind kind = static_cast<BinaryOperatorKind>(i);
    ExprResult result = sema.CreateBuiltinBinOp(loc, kind, lhs, rhs);

    if (!result.isInvalid()) validAlternatives.push_back(kind);
  }

  return validAlternatives;
}
