#include <ctime>
#include <string>
#include <clang/AST/ASTContext.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class WrapFuncCallInTimingFunction : public Mutator,
                                     public clang::RecursiveASTVisitor<WrapFuncCallInTimingFunction> {
public:
    using Mutator::Mutator;

    bool VisitCallExpr(clang::CallExpr *CE) {
        TheFuncCalls.push_back(CE);
        return true;
    }

    bool mutate() override {
        // Step 1, Traverse the AST
        TraverseAST(getASTContext());

        // Step 2, Randomly select a mutation instance
        if (TheFuncCalls.empty())
            return false;

        CallExpr *selectedExpr = randElement(TheFuncCalls);

        // Step 3, Checking mutation validity
        // In this case, we don't have any specific validity condition, so we skip this step.

        // Step 4, Perform mutation
        std::string timingStartCode = "{ clock_t start, end; double time_taken; start = clock(); ";
        std::string timingEndCode = "end = clock(); time_taken = (double)(end - start) / CLOCKS_PER_SEC; printf(\"Execution time: %.5f seconds\\n\", time_taken); }";
        SourceRange srcRange = selectedExpr->getSourceRange();
        getRewriter().ReplaceText(srcRange, timingStartCode + getSourceText(selectedExpr) + ";\n" + timingEndCode);

        FunctionDecl* FD = const_cast<FunctionDecl*>(selectedExpr->getDirectCallee());
        const FunctionDecl* recentFD = dynamic_cast<const FunctionDecl*>(getMostRecentTranslationUnitDecl(FD));
        if(recentFD)
            addStringBeforeFunctionDecl(recentFD, "#include <time.h>\n#include <stdio.h>\n");

        // Step 5, Return true if changed
        return true;
    }

private:
    std::vector<clang::CallExpr *> TheFuncCalls;
};

static RegisterMutator<WrapFuncCallInTimingFunction> M("u3.WrapFuncCallInTimingFunction", 
    "This mutator selects a function call in the code and wraps it with a timing function, effectively measuring and logging the execution time of the selected function call.");
