#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class IncreasePointerLevel : public Mutator,
                      public clang::RecursiveASTVisitor<IncreasePointerLevel> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    QualType QT = VD->getType().getNonReferenceType();
    if (QT->isPointerType() && !QT->isFunctionPointerType() && !QT->isMemberFunctionPointerType() && !QT->isArrayType()) {
      pointerDecls.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (pointerDecls.empty()) {
      return false;
    }

    VarDecl *VD = randElement(pointerDecls);
    QualType oldType = VD->getType();
    QualType newType = getASTContext().getPointerType(oldType);

    if (!replaceVarDeclarator(VD, newType)) {
      return false;
    }

    return true;
  }

private:
  std::vector<VarDecl*> pointerDecls;
};

static RegisterMutator<IncreasePointerLevel> M("u3.IncreasePointerLevel", "Increase the pointer level by one in a variable declaration");
