#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReducePointerLevel : public Mutator,
                           public clang::RecursiveASTVisitor<ReducePointerLevel> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getType()->isPointerType() && VD->getType()->getPointeeType()->isPointerType()) {
      TheVars.push_back(VD);
    }
    return true;
  }

  bool VisitUnaryOperator(clang::UnaryOperator *UO) {
    if (UO->getOpcode() == UO_Deref && isa<DeclRefExpr>(UO->getSubExpr())) {
      DeclRefExpr *DRE = cast<DeclRefExpr>(UO->getSubExpr());
      if (isa<VarDecl>(DRE->getDecl())) {
        VarDecl *VD = cast<VarDecl>(DRE->getDecl());
        if (VD->getType()->isPointerType() && VD->getType()->getPointeeType()->isPointerType()) {
          TheDerefs.push_back(UO);
        }
      }
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty()) return false;

    VarDecl *VD = randElement(TheVars);

    // Get the underlying type of the pointer-to-pointer
    QualType underlyingType = VD->getType()->getPointeeType()->getPointeeType();

    // Replace the type in the variable declaration
    replaceVarDeclarator(VD, getASTContext().getPointerType(underlyingType));

    // Replace all occurrences of dereferencing the double pointer with dereferencing the single pointer
    for (auto UO : TheDerefs) {
      DeclRefExpr *DRE = cast<DeclRefExpr>(UO->getSubExpr());
      VarDecl *VDeref = cast<VarDecl>(DRE->getDecl());
      if (VDeref == VD) {
        getRewriter().ReplaceText(UO->getSourceRange(), VD->getNameAsString());
      }
    }

    return true;
  }

private:
  std::vector<VarDecl *> TheVars;
  std::vector<UnaryOperator *> TheDerefs;
};

static RegisterMutator<ReducePointerLevel> M("u3.ReducePointerLevel", "Reduce the level of pointer by one in a variable declaration.");
