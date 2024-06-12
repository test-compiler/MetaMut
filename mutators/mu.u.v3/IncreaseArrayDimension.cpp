#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class IncreaseArrayDimension : public Mutator,
                      public clang::RecursiveASTVisitor<IncreaseArrayDimension> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    // Checking that the variable is an array
    if (VD->getType()->isArrayType()) {
      TheArrays.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    // Traverse the AST
    TraverseAST(getASTContext());

    // Randomly select an array instance
    if (TheArrays.empty()) return false;
    VarDecl *chosenArray = randElement(TheArrays);

    // Perform mutation
    const ArrayType *arrayType = chosenArray->getType()->getAsArrayTypeUnsafe();
    QualType elementType = arrayType->getElementType();

    // Create an array size of "1"
    IntegerLiteral *arraySize = IntegerLiteral::Create(getASTContext(), llvm::APInt(32, 1), getASTContext().IntTy, SourceLocation());
    QualType newType = getASTContext().getConstantArrayType(elementType, arraySize->getValue(), arraySize, ArrayType::Normal, 0);

    replaceVarDeclarator(chosenArray, newType);

    return true;
  }

private:
  std::vector<VarDecl*> TheArrays;
};

static RegisterMutator<IncreaseArrayDimension> M("u3.IncreaseArrayDimension", "This mutator selects an array variable declaration and increases its dimension by one, effectively converting it into a multidimensional array.");
