#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class InsertNullPointer : public Mutator,
                          public clang::RecursiveASTVisitor<InsertNullPointer> {
public:
    using Mutator::Mutator;
    
    bool VisitDeclStmt(DeclStmt *DS) {
        for (auto it = DS->decl_begin(); it != DS->decl_end(); ++it) {
            if (VarDecl *VD = dyn_cast<VarDecl>(*it)) {
                if (VD->getType()->isPointerType()) {
                    TheStmts.push_back(DS);
                    break;  // No need to continue if we found a pointer in this DeclStmt.
                }
            }
        }
        return true;
    }

    bool mutate() override {
        TraverseDecl(getASTContext().getTranslationUnitDecl());
        if (TheStmts.empty()) return false;

        DeclStmt *ds = randElement(TheStmts);

        // Need to find the VarDecl again...
        for (auto it = ds->decl_begin(); it != ds->decl_end(); ++it) {
            if (VarDecl *VD = dyn_cast<VarDecl>(*it)) {
                if (VD->getType()->isPointerType()) {
                    std::string newStmt = "\n" + VD->getNameAsString() + " = NULL;";

                    addStringAfterStmt(ds, newStmt);

                    return true;
                }
            }
        }

        // This should never happen, unless our VisitDeclStmt or the AST changed between calls.
        return false;
    }

private:
    std::vector<DeclStmt *> TheStmts;
};

static RegisterMutator<InsertNullPointer> M("u3.InsertNullPointer", 
    "This mutator selects a pointer variable in the code and adds a new assignment statement that sets the pointer to NULL.");
