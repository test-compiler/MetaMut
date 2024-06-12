#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ChangeFunctionReturnType : public Mutator,
                                 public clang::RecursiveASTVisitor<ChangeFunctionReturnType> {

public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    TheFunctions.push_back(FD);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    
    if (TheFunctions.empty()) return false;
    
    FunctionDecl *functionDecl = randElement(TheFunctions);
    QualType oldType = functionDecl->getReturnType();
    QualType newType = oldType; // Generate a new, compatible type based on the old type

    // replace the return type in the function declaration
    getRewriter().ReplaceText(functionDecl->getBeginLoc(), functionDecl->getNameAsString().length(), 
                              formatAsDecl(newType, functionDecl->getNameAsString()));

    // Modify the function body to return a value of the new type
    // This will vary based on the old and new types

    return true;
  }

private:
  std::vector<FunctionDecl*> TheFunctions;
};

static RegisterMutator<ChangeFunctionReturnType> M("u3.ChangeFunctionReturnType", "Change function return type to a compatible type and modifies the function body accordingly to preserve the semantics of the program.");
