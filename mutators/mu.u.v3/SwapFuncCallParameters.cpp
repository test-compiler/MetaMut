#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class SwapFuncCallParameters : public Mutator, public RecursiveASTVisitor<SwapFuncCallParameters> {
public:
    using Mutator::Mutator;

    bool VisitCallExpr(CallExpr* callExpr) {
        if (callExpr->getNumArgs() < 2) {
            return true; // Skip calls with less than 2 parameters
        }

        Calls.push_back(callExpr);
        return true;
    }

    bool mutate() override {
        Calls.clear();
        TraverseAST(getASTContext());
        
        if (Calls.empty()) {
            return false; // No suitable function call for mutation
        }

        CallExpr* call = randElement(Calls); // Get a random function call
        unsigned numArgs = call->getNumArgs();
        unsigned firstArgIndex = randIndex(numArgs); // Get first parameter index
        unsigned secondArgIndex = randIndex(numArgs); // Get second parameter index
        while (secondArgIndex == firstArgIndex) { // Ensure different indexes
            secondArgIndex = randIndex(numArgs);
        }

        Expr* firstArg = call->getArg(firstArgIndex);
        Expr* secondArg = call->getArg(secondArgIndex);

        // Swap two parameters in the function call
        getRewriter().ReplaceText(firstArg->getSourceRange(), getSourceText(secondArg));
        getRewriter().ReplaceText(secondArg->getSourceRange(), getSourceText(firstArg));

        return true;
    }

private:
    std::vector<CallExpr*> Calls;
};

static RegisterMutator<SwapFuncCallParameters> M("u3.SwapFuncCallParameters", "Swap the order of two random parameters in a function call");
