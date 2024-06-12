// DecayMultidimensionalArray.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class DecayMultidimensionalArray 
    : public Mutator,
      public clang::RecursiveASTVisitor<DecayMultidimensionalArray> {
public:
  using Mutator::Mutator;

  // Visit each VarDecl (variable declaration) in the AST
  bool VisitVarDecl(VarDecl* VD) {
    QualType type = VD->getType();

    // We're interested in VarDecls which are of constant array type
    if (const ConstantArrayType* cat = dyn_cast<ConstantArrayType>(type)) {
      // Only consider multi-dimensional arrays
      if (cat->getElementType()->isConstantArrayType()) {
        multiDimensionalArrayVarDecls.push_back(VD);
      }
    }
    return true;
  }

  bool mutate() override {
    // Traverse the AST to collect multi-dimensional array VarDecls
    TraverseDecl(getASTContext().getTranslationUnitDecl());

    // If no suitable VarDecls found, return false indicating no mutation done
    if (multiDimensionalArrayVarDecls.empty()) {
      return false;
    }

    // Randomly select a multi-dimensional array VarDecl
    VarDecl* VD = randElement(multiDimensionalArrayVarDecls);

    // Prepare the new type string. For simplicity, we only handle decay to one level
    QualType type = VD->getType().getCanonicalType();
    const ConstantArrayType* cat = dyn_cast<ConstantArrayType>(type);
    std::string newType = formatAsDecl(cat->getElementType(), "{0}") + "*";

    // Perform mutation: Replace the old type with the new one
    replaceVarDeclarator(VD, getASTContext().getPointerType(cat->getElementType()));

    // Return true indicating mutation done
    return true;
  }

private:
  std::vector<VarDecl*> multiDimensionalArrayVarDecls;
};

static RegisterMutator<DecayMultidimensionalArray> M("u3.DecayMultidimensionalArray", "Decays a multi-dimensional array by one level.");
