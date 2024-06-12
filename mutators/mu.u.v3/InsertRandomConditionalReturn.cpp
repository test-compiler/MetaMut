#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRandomConditionalReturn : public Mutator,
                          public clang::RecursiveASTVisitor<InsertRandomConditionalReturn> {
public:
    using Mutator::Mutator;
    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheFunctions.empty()) return false;

        FunctionDecl *selectedFunc = randElement(TheFunctions);
        CompoundStmt* funcBody = dyn_cast<CompoundStmt>(selectedFunc->getBody());
        if(!funcBody) return false;
        
        // Get a default value for the return type
        std::string defaultValue = getDefaultValueForType(selectedFunc->getReturnType(), getASTContext());
        
        // Create a random conditional
        std::string condition = generateRandomCondition();

        // Insert a conditional return statement
        std::string newStmt = "if (" + condition + ") return " + defaultValue + ";\n";
        getRewriter().InsertTextBefore(funcBody->getLBracLoc().getLocWithOffset(1), newStmt);
        
        return true;
    }

    bool VisitFunctionDecl(clang::FunctionDecl *FD) {
        // We're only interested in non-void functions
        if(!FD->getReturnType()->isVoidType()){
            TheFunctions.push_back(FD);
        }
        return true;
    }

private:
    std::vector<FunctionDecl *> TheFunctions;

    std::string getDefaultValueForType(QualType type, ASTContext &context) {
        if (type->isIntegerType()) return "0";
        if (type->isRealFloatingType()) return "0.0";
        if (type->isBooleanType()) return "false";
        if (type->isPointerType()) return "NULL";
        return "{}";
    }

    std::string generateRandomCondition() {
        // Simple generation of a random condition for illustration purposes
        // For a more sophisticated version, this could be based on the variables in scope
        if (randBool()) return "rand() % 2 == 0";
        return "rand() % 3 == 1";
    }

};

static RegisterMutator<InsertRandomConditionalReturn> M("u3.InsertRandomConditionalReturn",
    "Randomly inserts a conditional return statement in a function's body"
);
