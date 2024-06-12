#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class AddVolatileQualifier : public Mutator,
                             public clang::RecursiveASTVisitor<AddVolatileQualifier> {

public:
    using Mutator::Mutator;
    
    bool mutate() override {
        TraverseDecl(getASTContext().getTranslationUnitDecl());
        if (TheVarDecls.empty()) return false;

        VarDecl *vd = randElement(TheVarDecls);
        QualType origType = vd->getType();

        if (origType.isVolatileQualified()) return false; // Skip if already volatile

        QualType newType = origType.withCVRQualifiers(origType.getCVRQualifiers() | Qualifiers::Volatile);

        // Format the new type and the variable identifier into a variable declaration
        std::string newDecl = formatAsDecl(newType, vd->getNameAsString());
        getRewriter().ReplaceText(vd->getSourceRange(), newDecl);

        return true;
    }

    bool VisitVarDecl(VarDecl *VD) {
        TheVarDecls.push_back(VD);
        return true;
    }

private:
    std::vector<VarDecl *> TheVarDecls;
};

static RegisterMutator<AddVolatileQualifier> M("u3.AddVolatileQualifier", 
    "This mutator selects a variable declaration in the code and adds the 'volatile' qualifier to it.");
