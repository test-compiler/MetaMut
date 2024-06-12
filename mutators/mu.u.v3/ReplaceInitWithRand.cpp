#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceInitWithRand : public Mutator, public clang::RecursiveASTVisitor<ReplaceInitWithRand> {
public:
    using Mutator::Mutator;

    bool VisitVarDecl(VarDecl *VD) {
        if (VD->hasInit()) {
            TheVars.push_back(VD);
        }
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheVars.empty()) return false;
        
        VarDecl *selectedVar = randElement(TheVars);

        std::string newVal;
        if (selectedVar->getType().getAsString() == "int") {
            newVal = std::to_string(getManager().getRandomGenerator()());
        } else if (selectedVar->getType().getAsString() == "char") {
            newVal = "'" + std::to_string(getManager().getRandomGenerator()() % 128) + "'";
        } else if (selectedVar->getType().getAsString() == "float") {
            newVal = std::to_string(getManager().getRandomGenerator()() / 100.0f);
        } else {
            return false;
        }

        getRewriter().ReplaceText(selectedVar->getInit()->getSourceRange(), newVal);

        return true;
    }

private:
    std::vector<VarDecl*> TheVars;
};

static RegisterMutator<ReplaceInitWithRand> M("u3.ReplaceInitWithRand", "Identify a variable initialization and replace the initial value with a random value");
