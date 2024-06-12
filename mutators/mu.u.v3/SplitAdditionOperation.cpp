#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class SplitAdditionOperation : public Mutator, 
                               public clang::RecursiveASTVisitor<SplitAdditionOperation> {
public:
  using Mutator::Mutator;

  bool VisitBinaryOperator(BinaryOperator *BO) {
    if (BO->isAdditiveOp() && BO->getLHS() && BO->getRHS()) {
      AddOperations.push_back(BO);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (AddOperations.empty())
      return false;

    BinaryOperator *selectedBO = randElement(AddOperations);
    bool selectLeftOperand = randBool();

    Expr *selectedOperand = selectLeftOperand ? selectedBO->getLHS() : selectedBO->getRHS();
    Expr *remainingOperand = selectLeftOperand ? selectedBO->getRHS() : selectedBO->getLHS();

    // Build the new addition operation
    std::string newCode = "(" + getSourceText(selectedOperand) + " + " + getSourceText(remainingOperand) + ")";
    getRewriter().ReplaceText(selectedBO->getSourceRange(), newCode);

    return true;
  }

private:
  std::vector<BinaryOperator *> AddOperations;
};

static RegisterMutator<SplitAdditionOperation> M("u3.SplitAdditionOperation", 
    "Identifies an addition operation involving multiple operands, selects one operand, and isolates it into a separate addition operation.");
