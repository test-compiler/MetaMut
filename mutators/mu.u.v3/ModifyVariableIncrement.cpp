#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Expr.h>

using namespace clang;

class ModifyVariableIncrement : public Mutator,
                                public clang::RecursiveASTVisitor<ModifyVariableIncrement> {
public:
  using Mutator::Mutator;

  bool VisitUnaryOperator(UnaryOperator *UO) {
    if (UO->isIncrementOp()) {
      IncrementOperators.push_back(UO);
    }
    return true;
  }

  bool VisitBinaryOperator(BinaryOperator *BO) {
    if (BO->isAssignmentOp() && BO->getRHS()->isIntegerConstantExpr(getASTContext())) {
      if (BO->getOpcode() == BO_AddAssign) {
        AddAssignOperators.push_back(BO);
      }
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (IncrementOperators.empty() && AddAssignOperators.empty()) return false;
    
    if (randBool()) {
      if (!IncrementOperators.empty()) {
        UnaryOperator *UO = randElement(IncrementOperators);
        getRewriter().ReplaceText(UO->getOperatorLoc(), "--");
        return true;
      }
    } else {
      if (!AddAssignOperators.empty()) {
        BinaryOperator *BO = randElement(AddAssignOperators);
        getRewriter().ReplaceText(BO->getOperatorLoc(), BO->getOpcodeStr(BO_SubAssign));
        return true;
      }
    }

    return false;
  }

private:
  std::vector<UnaryOperator*> IncrementOperators;
  std::vector<BinaryOperator*> AddAssignOperators;
};

static RegisterMutator<ModifyVariableIncrement> M("u3.ModifyVariableIncrement",
    "Identifies a variable increment operation in the code (like i++ or i+=1) "
    "and modifies it to a decrement operation (like i-- or i-=1), effectively "
    "changing the increment operation to its inverse.");
