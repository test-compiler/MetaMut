// TransformPostfixToPrefix.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class TransformPostfixToPrefix
    : public Mutator,
      public clang::RecursiveASTVisitor<TransformPostfixToPrefix> {
public:
  using Mutator::Mutator;

  bool VisitUnaryOperator(clang::UnaryOperator *UO) {
    if (UO->isPostfix()) {
      TheOperators.push_back(UO);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheOperators.empty()) return false;

    UnaryOperator *expr = randElement(TheOperators);

    // perform rewrite
    std::string NewOp = expr->isIncrementOp() ? "++" : "--";
    getRewriter().InsertTextBefore(expr->getSubExpr()->getBeginLoc(), NewOp);
    getRewriter().ReplaceText(
        expr->getOperatorLoc(),
        Lexer::MeasureTokenLength(expr->getOperatorLoc(),
        getRewriter().getSourceMgr(), getASTContext().getLangOpts()),
        "");

    return true;
  }

private:
  std::vector<clang::UnaryOperator *> TheOperators;
};

static RegisterMutator<TransformPostfixToPrefix> M("u3.TransformPostfixToPrefix", "Convert a postfix increment or decrement operation into an equivalent prefix operation.");
