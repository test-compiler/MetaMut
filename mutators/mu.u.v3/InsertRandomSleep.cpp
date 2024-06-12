#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRandomSleep : public Mutator, public clang::RecursiveASTVisitor<InsertRandomSleep> {
public:
    using Mutator::Mutator;

    bool VisitFunctionDecl(FunctionDecl *FD) {
        if (FD->hasBody() && FD->getNameAsString() != "main") {
            TheFuncs.push_back(FD);
        }
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheFuncs.empty())
            return false;

        FunctionDecl *selectedFunc = randElement(TheFuncs);
        Stmt *firstStmt = *(llvm::cast<CompoundStmt>(selectedFunc->getBody())->body_begin());

        if (firstStmt) {
            std::string sleepCall = "sleep(1);\n";
            getRewriter().InsertTextBefore(firstStmt->getBeginLoc(), sleepCall);
            return true;
        }

        return false;
    }

private:
    std::vector<FunctionDecl*> TheFuncs;
};

static RegisterMutator<InsertRandomSleep> M("u3.InsertRandomSleep", "Insert a call to the sleep function at the start of a random function");
