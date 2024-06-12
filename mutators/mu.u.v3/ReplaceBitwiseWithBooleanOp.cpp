#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class ReplaceBitwiseWithBooleanOp : public Mutator,
                                    public RecursiveASTVisitor<ReplaceBitwiseWithBooleanOp> {
public:
  using Mutator::Mutator;
  
  bool VisitBinaryOperator(BinaryOperator *BO) {
    if (BO->isBitwiseOp()) {
      TheBitwiseOps.push_back(BO);
    }
    return true;
  }
  
  bool VisitUnaryOperator(UnaryOperator *UO) {
    if (UO->getOpcode() == UO_Not) {
      TheUnaryOps.push_back(UO);
    }
    return true;
  }
  
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheBitwiseOps.empty() && TheUnaryOps.empty())
      return false;
    
    if (randBool()) {
      if (!TheBitwiseOps.empty()) {
        BinaryOperator *BO = randElement(TheBitwiseOps);
        BinaryOperatorKind newOp = BO->getOpcode() == BO_And ? BO_LAnd : BO_LOr;
        getRewriter().ReplaceText(BO->getOperatorLoc(), BinaryOperator::getOpcodeStr(newOp));
      }
    } else {
      if (!TheUnaryOps.empty()) {
        UnaryOperator *UO = randElement(TheUnaryOps);
        getRewriter().ReplaceText(UO->getOperatorLoc(), "!");
      }
    }
    return true;
  }

private:
  std::vector<BinaryOperator *> TheBitwiseOps;
  std::vector<UnaryOperator *> TheUnaryOps;
};

static RegisterMutator<ReplaceBitwiseWithBooleanOp> M("u3.ReplaceBitwiseWithBooleanOp", 
    "This mutator identifies a bitwise operation in the code, where a boolean operation could equivalently be used, and converts the bitwise operation to its boolean counterpart. This includes converting & to &&, | to ||, ~ to ! etc.");
