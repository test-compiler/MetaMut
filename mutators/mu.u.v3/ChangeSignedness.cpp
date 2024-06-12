#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ChangeSignedness : public Mutator, public clang::RecursiveASTVisitor<ChangeSignedness> {
public:
    using Mutator::Mutator;
    
    bool VisitVarDecl(clang::VarDecl *VD) {
        if (VD->getType().getCanonicalType()->isIntegerType())
            TheVars.push_back(VD);
        return true;
    }
    
    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheVars.empty()) return false;

        VarDecl *selectedVar = randElement(TheVars);

        QualType origType = selectedVar->getType();
        QualType newType;
        ASTContext &astContext = getASTContext();
        if (origType->isSignedIntegerType()) {
            newType = getCorrespondingUnsignedType(origType, astContext);
        } else if (origType->isUnsignedIntegerType()) {
            newType = getCorrespondingSignedType(origType, astContext);
        } else {
            return false;
        }
        
        getRewriter().ReplaceText(selectedVar->getSourceRange(), newType.getAsString() + " " + selectedVar->getNameAsString());
        return true;
    }

private:
    std::vector<VarDecl *> TheVars;

    QualType getCorrespondingUnsignedType(QualType origType, ASTContext &astContext) {
        if (origType == astContext.IntTy) return astContext.UnsignedIntTy;
        if (origType == astContext.LongTy) return astContext.UnsignedLongTy;
        if (origType == astContext.LongLongTy) return astContext.UnsignedLongLongTy;
        // add more types if needed
        return origType;
    }

    QualType getCorrespondingSignedType(QualType origType, ASTContext &astContext) {
        if (origType == astContext.UnsignedIntTy) return astContext.IntTy;
        if (origType == astContext.UnsignedLongTy) return astContext.LongTy;
        if (origType == astContext.UnsignedLongLongTy) return astContext.LongLongTy;
        // add more types if needed
        return origType;
    }
};

static RegisterMutator<ChangeSignedness> M("u3.ChangeSignedness", "Change an integer variable's signedness from signed to unsigned, or vice versa.");
