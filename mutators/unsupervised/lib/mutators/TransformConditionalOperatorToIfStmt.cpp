#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class TransformConditionalOperatorToIfStmt 
    : public Mutator,
      public clang::RecursiveASTVisitor<TransformConditionalOperatorToIfStmt> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheConditionalOperators.empty()) return false;
    
    ConditionalOperator *condOp = randElement(TheConditionalOperators);
    Expr *cond = condOp->getCond();
    Expr *trueExpr = condOp->getTrueExpr();
    Expr *falseExpr = condOp->getFalseExpr();

    // Perform rewrite
    std::string NewStmt = 
        "if (" + getSourceText(cond).str() + ") {\n" +
        "max = " + getSourceText(trueExpr).str() + ";\n" +
        "} else {\n" +
        "max = " + getSourceText(falseExpr).str() + ";\n" +
        "}\n";

    getRewriter().ReplaceText(condOp->getSourceRange(), NewStmt);

    return true;
  }

  bool VisitConditionalOperator(ConditionalOperator *CO) {
    TheConditionalOperators.push_back(CO);
    return true;
  }

private:
  std::vector<ConditionalOperator *> TheConditionalOperators;
};

static RegisterMutator<TransformConditionalOperatorToIfStmt> X(
    "TransformConditionalOperatorToIfStmt", "Transform a ConditionalOperator into an IfStmt");