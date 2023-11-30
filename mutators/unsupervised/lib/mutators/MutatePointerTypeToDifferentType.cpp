// MutatePointerTypeToDifferentType.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class MutatePointerTypeToDifferentType
    : public Mutator,
      public clang::RecursiveASTVisitor<MutatePointerTypeToDifferentType> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (ThePointers.empty()) return false;

    VarDecl *pointerDecl = randElement(ThePointers);
    QualType oldType = pointerDecl->getType();

    auto alternatives = computeValidAlternatives(oldType);
    if (alternatives.empty()) return false;

    QualType newType = alternatives[randIndex(alternatives.size())];

    // Special handling for function pointer types
    std::string newTypeStr;
    if (newType->isFunctionPointerType())
      newTypeStr = formatAsDecl(newType, "");
    else
      newTypeStr = formatAsDecl(newType, pointerDecl->getNameAsString());

    SourceRange typeRange = pointerDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange();
    getRewriter().ReplaceText(typeRange, newTypeStr);
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getType()->isPointerType()) {
      ThePointers.push_back(VD);
    }
    return true;
  }
  
  bool VisitType(clang::Type *T) {
    AllTypes.insert(T->getCanonicalTypeInternal());
    return true;
  }

private:
  std::vector<clang::VarDecl *> ThePointers;
  std::set<clang::QualType> AllTypes;

  std::vector<clang::QualType> computeValidAlternatives(clang::QualType oldType) {
    QualType oldPointeeType = oldType->getPointeeType();

    std::vector<QualType> alternatives;
    for (auto type : AllTypes) {
      if (getASTContext().hasSameUnqualifiedType(oldPointeeType, type))
        continue;

      alternatives.push_back(getASTContext().getPointerType(type));
    }

    return alternatives;
  }
};

static RegisterMutator<MutatePointerTypeToDifferentType> M(
    "MutatePointerTypeToDifferentType", "Change pointer's pointed-to type.");