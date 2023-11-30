// ReplaceBinaryOperatorWithEquivalent.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ReplaceBinaryOperatorWithEquivalent
    : public Mutator,
      public clang::RecursiveASTVisitor<ReplaceBinaryOperatorWithEquivalent> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    BinaryOperator *expr = randElement(TheOperators);

    // perform rewrite
    auto newOps = computeValidAlternatives(expr);
    if (newOps.empty()) return false;

    auto newOp = newOps[randIndex(newOps.size())];
    SourceLocation OpLoc = expr->getOperatorLoc();
    getRewriter().ReplaceText(
        OpLoc, expr->getOpcodeStr(expr->getOpcode()).size(), newOp);

    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator *BO) {
    TheOperators.push_back(BO);
    return true;
  }

private:
  std::vector<clang::BinaryOperator *> TheOperators;

  std::vector<std::string> computeValidAlternatives(
      clang::BinaryOperator *op) {
    std::vector<std::string> validAlternatives;

    Expr *lhs = op->getLHS();
    Expr *rhs = op->getRHS();
    SourceLocation loc = op->getExprLoc();

    switch(op->getOpcode()) {
    case BO_Add:
      validAlternatives.push_back("-(-" + getSourceText(rhs).str() + ")");
      break;
    case BO_Mul:
      validAlternatives.push_back("/(1/" + getSourceText(rhs).str() + ")");
      break;
    default:
      break;
    }

    return validAlternatives;
  }
};

static RegisterMutator<ReplaceBinaryOperatorWithEquivalent> M(
    "ReplaceBinaryOperatorWithEquivalent", "Replace binary operator with an equivalent operation.");