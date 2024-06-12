#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InvertUnaryOperator : public Mutator, 
                            public clang::RecursiveASTVisitor<InvertUnaryOperator> {
public:
  using Mutator::Mutator;

  bool VisitUnaryOperator(clang::UnaryOperator *UO) {
    TheOperators.push_back(UO);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheOperators.empty()) {
      return false;
    }

    UnaryOperator *UO = randElement(TheOperators);
    UnaryOperatorKind oldOp = UO->getOpcode();
    UnaryOperatorKind newOp;

    switch (oldOp) {
      case UO_Minus:
        newOp = UO_Plus;
        break;
      case UO_Plus:
        newOp = UO_Minus;
        break;
      case UO_LNot:
        newOp = UO_AddrOf;  // replace 'not' operator by 'address of' operator
        break;
      default:
        return false;
    }

    SourceLocation OpLoc = UO->getOperatorLoc();
    std::string NewOp = UnaryOperator::getOpcodeStr(newOp).str();

    getRewriter().ReplaceText(
        OpLoc, UnaryOperator::getOpcodeStr(oldOp).size(), NewOp);

    return true;
  }

private:
  std::vector<UnaryOperator *> TheOperators;
};

static RegisterMutator<InvertUnaryOperator> M("u3.InvertUnaryOperator", "Invert a unary operator in the code, e.g., change unary minus to unary plus, or logical not to no operation");
