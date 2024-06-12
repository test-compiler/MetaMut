#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class DecayFunctionParameterType : public Mutator,
                                   public clang::RecursiveASTVisitor<DecayFunctionParameterType> {
public:
  using Mutator::Mutator;

  bool VisitParmVarDecl(ParmVarDecl* PVD) {
    if (dyn_cast<ConstantArrayType>(PVD->getOriginalType().getTypePtr())) {
      TheArrayParms.push_back(PVD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheArrayParms.empty()) return false;

    ParmVarDecl* arrayParm = randElement(TheArrayParms);

    // Decay the array type to a pointer type
    QualType arrayType = arrayParm->getOriginalType();
    QualType pointerType = getASTContext().getPointerType(arrayType->getAsArrayTypeUnsafe()->getElementType());

    // Get the string representation of the new declaration
    std::string newDeclStr = formatAsDecl(pointerType, arrayParm->getName().str());

    // Use getRewriter to replace the old declaration with the new one
    SourceRange oldDeclRange = arrayParm->getSourceRange();
    getRewriter().ReplaceText(oldDeclRange, newDeclStr);

    return true;
  }

private:
  std::vector<ParmVarDecl*> TheArrayParms;
};

static RegisterMutator<DecayFunctionParameterType> M("u3.DecayFunctionParameterType", "This mutator takes a function parameter declared with an array type and decays it to a pointer type.");
