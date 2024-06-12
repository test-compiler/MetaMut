// WrapArithmeticOperationInAbs.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>
#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class WrapArithmeticOperationInAbs : public Mutator,
                             public clang::RecursiveASTVisitor<WrapArithmeticOperationInAbs> {
public:
  using Mutator::Mutator;

  bool VisitBinaryOperator(clang::BinaryOperator *BO) {
    if (BO->isAdditiveOp() || BO->isMultiplicativeOp()) {
      TheOps.push_back(BO);
    }
    return true;
  }

  bool mutate() override {
    // Step 1: Traverse the AST to collect mutation instances
    TraverseAST(getASTContext());
    if (TheOps.empty())
      return false;

    // Step 3: Randomly select a mutation instance
    BinaryOperator *selectedOp = randElement(TheOps);

    // Step 5: Perform mutation
    SourceRange range = selectedOp->getSourceRange();
    SourceLocation start = range.getBegin();
    SourceLocation end = range.getEnd();
    
    // Wraps the binary operation inside abs function.
    getRewriter().InsertTextBefore(start, "abs(");
    getRewriter().InsertTextAfter(getLocForEndOfToken(end), ")");
    
    return true;
  }

private:
  std::vector<clang::BinaryOperator *> TheOps;
};

static RegisterMutator<WrapArithmeticOperationInAbs> M("u3.WrapArithmeticOperationInAbs", "Wrap arithmetic operation inside an abs() function.");
