#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class ReplaceVarWithDefault : public Mutator,
                      public clang::RecursiveASTVisitor<ReplaceVarWithDefault> {

public:
  using Mutator::Mutator;
  
  bool VisitDeclRefExpr(DeclRefExpr *DRE) {
    if (isa<VarDecl>(DRE->getDecl()) && 
        !isa<FunctionType>(DRE->getDecl()->getType().getTypePtr())) {
        if(const RecordType *RT = dyn_cast<RecordType>(DRE->getDecl()->getType().getTypePtr())){
            if(RT->getDecl()->isUnion()){
                // Skip unions
                return true;
            }
        }
        TheVars.push_back(DRE);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheVars.empty())
      return false;

    DeclRefExpr *DRE = randElement(TheVars);

    VarDecl *VD = cast<VarDecl>(DRE->getDecl());
    QualType type = VD->getType();

    std::string defaultValue;

    if (type->isIntegerType()) {
      defaultValue = "0";
    } else if (type->isRealFloatingType()) {
      defaultValue = "0.0";
    } else if (type->isPointerType()) {
      defaultValue = "NULL";
    } else if (type->isBooleanType()) {
      defaultValue = "false";
    } else if (type->isStructureOrClassType()) {
      defaultValue = "{}";
    } else {
      // if we reach here, we do not handle this type yet
      return false;
    }

    getRewriter().ReplaceText(DRE->getSourceRange(), defaultValue);
    
    return true;
  }

private:
  std::vector<DeclRefExpr*> TheVars;
};

static RegisterMutator<ReplaceVarWithDefault> M("u3.ReplaceVarWithDefault", "Replace a variable with a default value of its type");
