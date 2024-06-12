#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class FlipRelationalOperator : public Mutator, 
                               public clang::RecursiveASTVisitor<FlipRelationalOperator> {
public:
  using Mutator::Mutator;

  bool VisitBinaryOperator(BinaryOperator* binaryOp) {
    if (binaryOp->isRelationalOp()) {
      TheOperators.push_back(binaryOp);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    BinaryOperator* binaryOp = randElement(TheOperators);

    // Map the operator to its inverse
    std::map<BinaryOperatorKind, BinaryOperatorKind> flipMap = {
      {BO_LT, BO_GE},
      {BO_GT, BO_LE},
      {BO_LE, BO_GT},
      {BO_GE, BO_LT},
      {BO_EQ, BO_NE},
      {BO_NE, BO_EQ}
    };

    // Perform the replacement in the BinaryOperator
    getRewriter().ReplaceText(binaryOp->getOperatorLoc(), 
                              BinaryOperator::getOpcodeStr(flipMap[binaryOp->getOpcode()]));

    return true;
  }

private:
  std::vector<BinaryOperator*> TheOperators;
};

static RegisterMutator<FlipRelationalOperator> M("u3.FlipRelationalOperator", "This mutator selects a relational operator in the code (like <, >, ==, !=, >=, <=) and flips it to its inverse operator.");
