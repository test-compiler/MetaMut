#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class ReplaceVariableWithGlobal : public Mutator,
                                  public clang::RecursiveASTVisitor<ReplaceVariableWithGlobal> {
public:
    using Mutator::Mutator;

    bool VisitVarDecl(VarDecl *VD) {
        if (!global_var && VD->isFileVarDecl()) {
            global_var = VD;
        } else if (global_var && VD->isLocalVarDecl() && VD->getType() == global_var->getType()) {
            local_vars.push_back(VD);
        }
        return true;
    }

    bool VisitDeclRefExpr(DeclRefExpr *DRE) {
        if (mutating && DRE->getDecl() == local_var) {
            getRewriter().ReplaceText(DRE->getSourceRange(), global_var->getNameAsString());
        }
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());

        if (local_vars.empty() || !global_var) {
            return false;
        }

        local_var = randElement(local_vars);
        mutating = true;
        TraverseAST(getASTContext());
        mutating = false;
        
        return true;
    }

private:
    std::vector<VarDecl *> local_vars;
    VarDecl *global_var = nullptr; // Initialize to null
    VarDecl *local_var = nullptr;
    bool mutating = false;
};

static RegisterMutator<ReplaceVariableWithGlobal> M("u3.ReplaceVariableWithGlobal", "Replaces a local variable in a function with a global variable of the same type.");
