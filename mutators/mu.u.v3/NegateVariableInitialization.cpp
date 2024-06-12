// NegateVariableInitialization.cpp
#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;

class NegateVariableInitialization : public Mutator,
                                    public RecursiveASTVisitor<NegateVariableInitialization> {
public:
    using Mutator::Mutator;
    
    bool VisitVarDecl(VarDecl *VD) {
        if(VD->hasInit()) {
            TheVarDecls.push_back(VD);
        }
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheVarDecls.empty()) return false;

        VarDecl *VD = randElement(TheVarDecls);
        Expr *init = VD->getInit();

        if (!init || !init->isIntegerConstantExpr(getASTContext())) return false;

        auto initRange = getExpansionRange(init->getSourceRange());
        std::string newInit = "-" + getSourceText(init);
        getRewriter().ReplaceText(initRange, newInit);

        return true;
    }

private:
    std::vector<VarDecl*> TheVarDecls;
};

static RegisterMutator<NegateVariableInitialization> M("u3.NegateVariableInitialization", 
    "This mutator selects a variable initialization in the code and negates the value during its initialization, effectively changing the initial value of the variable to its negative counterpart.");
