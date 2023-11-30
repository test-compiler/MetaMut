#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class TransformConditionalOperatorToIf 
    : public Mutator,
      public clang::RecursiveASTVisitor<TransformConditionalOperatorToIf> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheCondOperators.empty()) return false;

    ConditionalOperator *condOp = randElement(TheCondOperators);
    Expr *cond = condOp->getCond();
    Expr *trueExpr = condOp->getTrueExpr();
    Expr *falseExpr = condOp->getFalseExpr();

    // Perform rewrite
    std::string NewStmt = std::string("int max;\n") + 
                          "if (" + getSourceText(cond).str() + ") {\n" +
                          "max = " + getSourceText(trueExpr).str() + ";\n" +
                          "} else {\n" +
                          "max = " + getSourceText(falseExpr).str() + ";\n" +
                          "}\n";

    getRewriter().ReplaceText(condOp->getSourceRange(), NewStmt);

    return true;
  }

  bool VisitConditionalOperator(ConditionalOperator *CO) {
    TheCondOperators.push_back(CO);
    return true;
  }

private:
  std::vector<ConditionalOperator *> TheCondOperators;
};

static RegisterMutator<TransformConditionalOperatorToIf> X(
    "TransformConditionalOperatorToIf", "Transform a ConditionalOperator into an IfStmt");