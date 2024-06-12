#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class DecayFunctionReturnType : public Mutator,
                                public clang::RecursiveASTVisitor<DecayFunctionReturnType> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    auto RetType = FD->getReturnType();
    if (RetType->isPointerType() && RetType->getPointeeType()->isPointerType()) {
      TheFuncs.push_back(FD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFuncs.empty()) return false;

    FunctionDecl *FD = randElement(TheFuncs);

    // Get the decayed type (from int** to int*)
    auto elementType = FD->getReturnType()->getPointeeType()->getPointeeType();
    QualType decayedType = getASTContext().getPointerType(elementType);

    // Perform the replacement in the function declaration
    replaceFunctionReturn(FD, decayedType);

    return true;
  }

  void replaceFunctionReturn(FunctionDecl *FD, QualType NewType) {
    std::string NewTypeStr = NewType.getAsString();

    SourceRange FuncTypeRange = FD->getReturnTypeSourceRange();
    getRewriter().ReplaceText(FuncTypeRange, NewTypeStr);
  }

private:
  std::vector<FunctionDecl *> TheFuncs;
};

static RegisterMutator<DecayFunctionReturnType> M("u3.DecayFunctionReturnType", "This mutator selects a function returning a pointer to pointer and changes its return type to a pointer type, effectively decaying the pointer to pointer to a pointer.");
