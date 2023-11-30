#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ModifyPointerTypeToDifferentCompatibleType : public Mutator,
                      public clang::RecursiveASTVisitor<ModifyPointerTypeToDifferentCompatibleType> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (PointerDecls.empty() || CompatibleTypes.empty()) return false;

    VarDecl *ptr = randElement(PointerDecls);
    QualType oldType = ptr->getType()->getPointeeType();
    QualType newType = randElement(CompatibleTypes);
    if (oldType == newType) return false;

    // Replace the old type with the new one in the declaration of the pointer
    auto range = getExpansionRange(ptr->getTypeSourceInfo()->getTypeLoc().getSourceRange());
    std::string replacement = formatAsDecl(newType, ptr->getNameAsString());
    getRewriter().ReplaceText(range, replacement);

    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getType()->isPointerType())
      PointerDecls.push_back(VD);
    else if (VD->getType()->isArrayType() 
             && VD->getType()->getAsArrayTypeUnsafe()->getElementType()->isRealType())
      CompatibleTypes.push_back(VD->getType()->getAsArrayTypeUnsafe()->getElementType());

    return true;
  }

private:
  std::vector<clang::VarDecl *> PointerDecls;
  std::vector<clang::QualType> CompatibleTypes;
};

static RegisterMutator<ModifyPointerTypeToDifferentCompatibleType> M(
    "ModifyPointerTypeToDifferentCompatibleType", "Modifies a random PointerType variable to point to another compatible type available in the current scope.");