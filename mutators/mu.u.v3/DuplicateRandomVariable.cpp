#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class DuplicateRandomVariable : public Mutator,
                                public clang::RecursiveASTVisitor<DuplicateRandomVariable> {
public:
    using Mutator::Mutator;

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheVars.empty()) return false;

        VarDecl *targetVar = randElement(TheVars);

        std::string newVarName = generateUniqueName(targetVar->getNameAsString());

        std::string newVarDecl;
        if (targetVar->getType()->isFunctionPointerType()) {
            auto funcTypePtr = targetVar->getType()->getPointeeType()->getAs<FunctionProtoType>();
            auto returnType = funcTypePtr->getReturnType().getAsString();

            std::string params;
            for (unsigned int i = 0; i < funcTypePtr->getNumParams(); ++i) {
                if (i != 0) params += ",";
                params += funcTypePtr->getParamType(i).getAsString();
            }

            newVarDecl = returnType + " (*" + newVarName + ")(" + params + ")";
        } else {
            auto* tagDecl = targetVar->getType()->getAsTagDecl();
            if (tagDecl && isa<RecordDecl>(tagDecl)) {
                newVarDecl = targetVar->getType().getAsString() + " " + newVarName;
            } else {
                newVarDecl = formatAsDecl(targetVar->getType(), newVarName);
            }
        }
        newVarDecl += " = " + targetVar->getNameAsString() + ";";

        addStringAfterVarDecl(targetVar, "; " + newVarDecl);

        auto &Usages = VarUsagesMap[targetVar];
        for (auto &Usage : Usages) {
            if (randBool()) {
                getRewriter().ReplaceText(Usage->getSourceRange(), newVarName);
            }
        }

        return true;
    }

    bool VisitVarDecl(VarDecl *VD) {
        if (VD->isLocalVarDecl()) {
            TheVars.push_back(VD);
        }
        return true;
    }

    bool VisitDeclRefExpr(DeclRefExpr *DRE) {
        if (auto *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
            VarUsagesMap[VD].push_back(DRE);
        }
        return true;
    }

private:
    std::vector<VarDecl*> TheVars;
    std::map<VarDecl*, std::vector<DeclRefExpr*>> VarUsagesMap;
};

static RegisterMutator<DuplicateRandomVariable> M("u3.DuplicateRandomVariable",
    "This mutator identifies a random variable in a function and duplicates it, replacing some of the usages of the original variable with the duplicate.");
