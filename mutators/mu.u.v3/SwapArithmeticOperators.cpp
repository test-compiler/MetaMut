#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class SwapArithmeticOperators : public Mutator,
                                public RecursiveASTVisitor<SwapArithmeticOperators> {

public:
    using Mutator::Mutator;
    
    bool VisitBinaryOperator(clang::BinaryOperator *BO) {
        if (BO->isMultiplicativeOp() || BO->isAdditiveOp()) {
            TheOperators.push_back(BO);
        }
        return true;
    }

    bool mutate() override {
        TraverseDecl(getASTContext().getTranslationUnitDecl());
        if (TheOperators.empty()) return false;

        BinaryOperator *op = randElement(TheOperators);
        BinaryOperatorKind oldKind = op->getOpcode();
        BinaryOperatorKind newKind;

        switch (oldKind) {
            case BO_Add: newKind = BO_Mul; break;
            case BO_Sub: newKind = BO_Div; break;
            case BO_Mul: newKind = BO_Add; break;
            case BO_Div: newKind = BO_Sub; break;
            default: return false;
        }

        std::string newOpString = BinaryOperator::getOpcodeStr(newKind).str();
        getRewriter().ReplaceText(op->getOperatorLoc(), newOpString);

        return true;
    }

private:
    std::vector<BinaryOperator *> TheOperators;
};

static RegisterMutator<SwapArithmeticOperators> M("u3.SwapArithmeticOperators", 
    "Identifies an arithmetic operation in the code and swaps it with another arithmetic operation (like addition becomes multiplication, subtraction becomes division, and vice versa)");
