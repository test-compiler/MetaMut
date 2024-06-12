#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class SwapFuncCallArgs : public Mutator,
                      public RecursiveASTVisitor<SwapFuncCallArgs> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(CallExpr *CE) {
    if (CE->getNumArgs() > 1)
      TheCalls.push_back(CE);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheCalls.empty()) return false;

    CallExpr *CE = randElement(TheCalls);
    unsigned int argPos1 = randIndex(CE->getNumArgs());
    unsigned int argPos2;
    do {
        argPos2 = randIndex(CE->getNumArgs());
    } while (argPos2 == argPos1);
    
    // Swap the two arguments in all occurrences of the function call
    for(auto call : TheCalls) {
      if(call->getDirectCallee() == CE->getDirectCallee()) {
        getRewriter().ReplaceText(call->getArg(argPos1)->getSourceRange(), getSourceText(call->getArg(argPos2)));
        getRewriter().ReplaceText(call->getArg(argPos2)->getSourceRange(), getSourceText(call->getArg(argPos1)));
      }
    }

    return true;
  }

private:
  std::vector<CallExpr*> TheCalls;
};

static RegisterMutator<SwapFuncCallArgs> M("u3.SwapFuncCallArgs", "This mutator selects a function call with more than one argument and swaps the order of two random arguments in all its occurrences.");
