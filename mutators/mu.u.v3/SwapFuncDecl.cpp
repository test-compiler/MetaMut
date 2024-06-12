#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class SwapFuncDecl : public Mutator,
                     public RecursiveASTVisitor<SwapFuncDecl> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->isThisDeclarationADefinition()) 
      return true; // Ignore function declarations that are not definitions

    TheFuncs.push_back(FD);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFuncs.size() < 2) return false; // At least two functions are required for swapping

    // Randomly select two functions
    FunctionDecl *FD1 = randElement(TheFuncs);
    FunctionDecl *FD2 = randElement(TheFuncs);
    while (FD1 == FD2) {
      FD2 = randElement(TheFuncs); // Ensure we have two different functions
    }
  
    // Replace the two function declarations
    std::string FD1Text = getSourceText(FD1);
    std::string FD2Text = getSourceText(FD2);
    
    getRewriter().ReplaceText(FD1->getSourceRange(), FD2Text);
    getRewriter().ReplaceText(FD2->getSourceRange(), FD1Text);

    return true;
  }

private:
  std::vector<FunctionDecl*> TheFuncs;
};

static RegisterMutator<SwapFuncDecl> M("u3.SwapFuncDecl", "This mutator selects two function declarations in the code and swaps their positions.");
