// InverseUnaryOperator.cpp
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

class InverseUnaryOperator
    : public Mutator,
      public clang::RecursiveASTVisitor<InverseUnaryOperator> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    UnaryOperator *expr = randElement(TheOperators);

    // perform rewrite
    SourceLocation startLoc = expr->getOperatorLoc();
    SourceLocation endLoc = getLocForEndOfToken(expr->getSubExpr()->getEndLoc());
    std::string exprText = getSourceText(expr).str();
    std::string newExprText = expr->getOpcodeStr(expr->getOpcode()).str() + "(" + expr->getOpcodeStr(expr->getOpcode()).str() + exprText + ");";
    getRewriter().ReplaceText(SourceRange(startLoc, endLoc), newExprText);

    return true;
  }

  bool VisitUnaryOperator(clang::UnaryOperator *UO) {
    if (UO->isIncrementDecrementOp()) return true; // skip ++ and --
    TheOperators.push_back(UO);
    return true;
  }

private:
  std::vector<clang::UnaryOperator *> TheOperators;
};

static RegisterMutator<InverseUnaryOperator> X(
    "InverseUnaryOperator", "Inverses a Unary Operator");