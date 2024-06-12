#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class TransformMultiDimensionalArrayToSingle : public Mutator,
                                                public clang::RecursiveASTVisitor<TransformMultiDimensionalArrayToSingle> {

public:
  using Mutator::Mutator;

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getType()->isArrayType() && VD->getType()->getAsArrayTypeUnsafe()->getElementType()->isArrayType()) {
        MultiDimensionalArrays.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    // Step 1, Traverse the AST
    TraverseAST(getASTContext());

    // Step 2, Collect mutation instances

    // Step 3, Randomly select a mutation instance
    if(MultiDimensionalArrays.empty()) return false;
    VarDecl *selectedArray = randElement(MultiDimensionalArrays);

    // Step 4, Checking mutation validity

    // Step 5, Perform mutation
    unsigned totalSize = 1;
    const ArrayType *ArrayType = selectedArray->getType()->getAsArrayTypeUnsafe();
    while(ArrayType) {
      const ConstantArrayType *constArrayType = dyn_cast<ConstantArrayType>(ArrayType);
      if (!constArrayType) return false;
      totalSize *= constArrayType->getSize().getZExtValue();
      ArrayType = constArrayType->getElementType()->getAsArrayTypeUnsafe();
    }

    // create new declaration string
    std::string newDecl = formatAsDecl(selectedArray->getType().getCanonicalType().getUnqualifiedType(), selectedArray->getNameAsString() + "[" + std::to_string(totalSize) + "]");
    
    // If the selected array has an initializer, add it to new declaration
    if (selectedArray->hasInit()) {
        newDecl += " = " + getSourceText(selectedArray->getInit());
    }
    
    newDecl += ";"; // Add a semicolon at the end of the declaration

    // Replace the old variable with the new one
    SourceLocation start = selectedArray->getBeginLoc();
    SourceLocation end = getLocForEndOfToken(selectedArray->getEndLoc());
    getRewriter().ReplaceText(SourceRange(start, end), newDecl);

    // Step 6, Return true if changed
    return true;
  }

private:
  std::vector<clang::VarDecl *> MultiDimensionalArrays;
};

static RegisterMutator<TransformMultiDimensionalArrayToSingle> M("u3.TransformMultiDimensionalArrayToSingle", "Transform a multi-dimensional array into a single-dimensional array.");
