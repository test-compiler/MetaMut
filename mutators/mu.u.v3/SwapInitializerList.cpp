// Includes
#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <algorithm>

using namespace clang;

class SwapInitializerList : public Mutator,
                      public clang::RecursiveASTVisitor<SwapInitializerList> {
public:
  using Mutator::Mutator;

  bool VisitInitListExpr(InitListExpr *initList) {
      if (initList->getNumInits() > 1) { // Ensure we have at least two elements to swap
          initLists.push_back(initList);
      }
      return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (initLists.empty()) return false;

    InitListExpr *initList = randElement(initLists);
    unsigned numInits = initList->getNumInits();
    unsigned first = randIndex(numInits);
    unsigned second = randIndex(numInits);
    
    // Ensure the two randomly selected elements are distinct
    while(second == first) {
        second = randIndex(numInits);
    }

    Expr* firstInit = initList->getInit(first);
    Expr* secondInit = initList->getInit(second);
    std::string firstText = getSourceText(firstInit);
    std::string secondText = getSourceText(secondInit);

    getRewriter().ReplaceText(firstInit->getSourceRange(), secondText);
    getRewriter().ReplaceText(secondInit->getSourceRange(), firstText);

    return true;
  }

private:
  std::vector<InitListExpr *> initLists;
};

static RegisterMutator<SwapInitializerList> M("u3.SwapInitializerList", 
    "This mutator selects a variable initialization list (in a constructor or aggregate initialization) and swaps two random elements within it, effectively altering the initialization order.");
