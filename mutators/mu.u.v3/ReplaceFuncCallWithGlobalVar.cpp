#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Expr.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class ReplaceFuncCallWithGlobalVar : public Mutator, 
                             public clang::RecursiveASTVisitor<ReplaceFuncCallWithGlobalVar> {
public:
    using Mutator::Mutator;

    bool VisitCallExpr(CallExpr *CE) {
        // Store function calls that return a value
        if (CE->getDirectCallee() && CE->getDirectCallee()->getReturnType().getTypePtr()) {
            TheCalls.push_back(CE);
        }
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheCalls.empty()) return false;

        // Select a random function call
        CallExpr *selectedCall = randElement(TheCalls);

        // Check the return type of the function call
        const FunctionDecl *FD = selectedCall->getDirectCallee();
        QualType returnType = FD->getReturnType();

        // Generate a new global variable of the same type
        std::string newGlobalVar = generateUniqueName("globalVar");
        std::string globalVarDeclaration = formatAsDecl(returnType, newGlobalVar) + ";\n";

        // Add the global variable declaration before the most recent translation unit
        FunctionDecl *nonConstFD = const_cast<FunctionDecl *>(FD);
        const FunctionDecl *mostRecentFuncDecl = static_cast<const FunctionDecl *>(getMostRecentTranslationUnitDecl(nonConstFD));
        addStringBeforeFunctionDecl(mostRecentFuncDecl, globalVarDeclaration);

        // Replace the function call with the new global variable
        getRewriter().ReplaceText(selectedCall->getSourceRange(), newGlobalVar + ";");

        return true;
    }

private:
    std::vector<CallExpr *> TheCalls;
};

static RegisterMutator<ReplaceFuncCallWithGlobalVar> M("u3.ReplaceFuncCallWithGlobalVar", 
    "Identify a function call that returns a value and replace it with a global variable of the same type");
