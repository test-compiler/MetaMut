#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceOperatorWithMathematicalEquivalent : public Mutator,
                                                  public clang::RecursiveASTVisitor<ReplaceOperatorWithMathematicalEquivalent> {
public:
  using Mutator::Mutator;
  
  bool VisitBinaryOperator(BinaryOperator *BO) {
    if (BO->isMultiplicativeOp()) {
      TheOperators.push_back(BO);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    BinaryOperator *op = randElement(TheOperators);
    Expr *lhs = op->getLHS();
    Expr *rhs = op->getRHS();

    std::string lhsText = getSourceText(lhs);
    std::string rhsText = getSourceText(rhs);

    std::string replaceText = lhsText + " / (1/" + rhsText + ")";

    getRewriter().ReplaceText(op->getSourceRange(), replaceText);

    return true;
  }

private:
  std::vector<BinaryOperator *> TheOperators;
};

static RegisterMutator<ReplaceOperatorWithMathematicalEquivalent> M("u3.ReplaceOperatorWithMathematicalEquivalent", "Replace multiplication operator with division operation having equivalent effect.");
