#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ChangeMathOpToEquivalentFunc : public Mutator,
                      public clang::RecursiveASTVisitor<ChangeMathOpToEquivalentFunc> {
public:
    using Mutator::Mutator;
    
    bool mutate() override {
        TraverseDecl(getASTContext().getTranslationUnitDecl());
        
        if (TheOps.empty()) {
            return false;
        }
        
        // Pick a random operator
        BinaryOperator* op = randElement(TheOps);
        
        // Create the replacement string
        std::string replacement;
        
        if (op->getOpcode() == BO_Add) {
            replacement = "add(" + getSourceText(op->getLHS()) + ", " + getSourceText(op->getRHS()) + ")";
        } else if (op->getOpcode() == BO_Sub) {
            replacement = "sub(" + getSourceText(op->getLHS()) + ", " + getSourceText(op->getRHS()) + ")";
        } else if (op->getOpcode() == BO_Mul) {
            replacement = "mul(" + getSourceText(op->getLHS()) + ", " + getSourceText(op->getRHS()) + ")";
        } else if (op->getOpcode() == BO_Div) {
            replacement = "div(" + getSourceText(op->getLHS()) + ", " + getSourceText(op->getRHS()) + ")";
        } else {
            return false; // Shouldn't get here, but just in case
        }
        
        // Perform the replacement
        getRewriter().ReplaceText(op->getSourceRange(), replacement);
        
        return true;
    }
    
    bool VisitBinaryOperator(BinaryOperator* Op) {
        // Only consider arithmetic operations
        if (Op->isAdditiveOp() || Op->isMultiplicativeOp()) {
            TheOps.push_back(Op);
        }
        
        return true;
    }
    
private:
    std::vector<BinaryOperator*> TheOps;
};

static RegisterMutator<ChangeMathOpToEquivalentFunc> M("u3.ChangeMathOpToEquivalentFunc", "Replaces a mathematical operator with an equivalent math library function call");
