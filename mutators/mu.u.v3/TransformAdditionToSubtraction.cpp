#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class TransformAdditionToSubtraction : public Mutator,
                                       public clang::RecursiveASTVisitor<TransformAdditionToSubtraction> {
public:
  using Mutator::Mutator;

  bool VisitBinaryOperator(clang::BinaryOperator *BO) {
    if (BO->isAdditiveOp() && BO->getOpcode() == BO_Add) {
      Additions.push_back(BO);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (Additions.empty()) return false;

    BinaryOperator *expr = randElement(Additions);

    // Perform rewrite
    SourceLocation OpLoc = expr->getOperatorLoc();
    std::string NewOp = BinaryOperator::getOpcodeStr(BO_Sub).str();
    getRewriter().ReplaceText(
        OpLoc, expr->getOpcodeStr(expr->getOpcode()).size(), NewOp);

    return true;
  }

private:
  std::vector<clang::BinaryOperator *> Additions;
};

static RegisterMutator<TransformAdditionToSubtraction> M("u3.TransformAdditionToSubtraction", "Transform addition operation to subtraction operation");
