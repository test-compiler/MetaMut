// SwapRandomUnaryOperands.cpp
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

class SwapRandomUnaryOperands
    : public Mutator,
      public clang::RecursiveASTVisitor<SwapRandomUnaryOperands> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    UnaryOperator* expr = randElement(TheOperators);

    // perform rewrite
    BinaryOperator* subExpr = cast<BinaryOperator>(expr->getSubExpr());
    Expr* lhs = subExpr->getLHS();
    Expr* rhs = subExpr->getRHS();

    std::string lhsStr = getSourceText(lhs).str();
    std::string rhsStr = getSourceText(rhs).str();

    SourceRange lhsRange = getExpansionRange(lhs->getSourceRange());
    SourceRange rhsRange = getExpansionRange(rhs->getSourceRange());

    getRewriter().ReplaceText(lhsRange, rhsStr);
    getRewriter().ReplaceText(rhsRange, lhsStr);

    return true;
  }

  bool VisitUnaryOperator(clang::UnaryOperator* UO) {
    if (isa<BinaryOperator>(UO->getSubExpr())) {
      TheOperators.push_back(UO);
    }
    return true;
  }

private:
  std::vector<clang::UnaryOperator*> TheOperators;
};

static RegisterMutator<SwapRandomUnaryOperands> X(
    "SwapRandomUnaryOperands", "Swap operands of a random UnaryOperator's BinaryOperator sub-expression.");