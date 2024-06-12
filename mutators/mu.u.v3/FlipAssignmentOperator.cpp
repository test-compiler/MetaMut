#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class FlipAssignmentOperator : public Mutator,
                               public clang::RecursiveASTVisitor<FlipAssignmentOperator> {
public:
  using Mutator::Mutator;

  bool VisitBinaryOperator(BinaryOperator *BO) {
    // Ignore BinaryOperator that is not assignment operator
    if (BO->isAssignmentOp() && (BO->getOpcode() == BO_AddAssign || BO->getOpcode() == BO_SubAssign || BO->getOpcode() == BO_MulAssign || BO->getOpcode() == BO_DivAssign)) {
      TheOperators.push_back(BO);
    }
    return true;  // Continue traversal
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    BinaryOperator *chosenOperator = randElement(TheOperators);
    BinaryOperatorKind oldKind = chosenOperator->getOpcode();
    BinaryOperatorKind newKind;

    // Determine the new opcode
    switch(oldKind) {
      case BO_AddAssign: newKind = BO_SubAssign; break;
      case BO_SubAssign: newKind = BO_AddAssign; break;
      case BO_MulAssign: newKind = BO_DivAssign; break;
      case BO_DivAssign: newKind = BO_MulAssign; break;
      default: return false;
    }

    // Replace the old operator with the new one
    std::string newOperator = BinaryOperator::getOpcodeStr(newKind).str();
    getRewriter().ReplaceText(chosenOperator->getOperatorLoc(), BinaryOperator::getOpcodeStr(oldKind).size(), newOperator);
    
    return true;
  }

private:
  std::vector<BinaryOperator*> TheOperators;
};

static RegisterMutator<FlipAssignmentOperator> M("u3.FlipAssignmentOperator", "This mutator selects an assignment operator in the code and flips it to its compatible assignment counterpart (like += becomes -=, *= becomes /=, and vice versa).");
