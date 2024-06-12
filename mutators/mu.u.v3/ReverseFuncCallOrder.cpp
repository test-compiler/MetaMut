#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReverseFuncCallOrder : public Mutator, public RecursiveASTVisitor<ReverseFuncCallOrder> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(CallExpr* e) {
    callExpressions.push_back(e);
    return true;  // to continue traversal
  }

  bool mutate() override {
    auto& ctx = getASTContext();
    auto tuDecl = ctx.getTranslationUnitDecl();
    callExpressions.clear();

    // Step 1: Collect all function calls
    TraverseDecl(tuDecl);

    // If there are less than 2 function calls, return false
    if (callExpressions.size() < 2) {
      return false;
    }

    // Step 2: Randomly select a pair of calls to swap
    unsigned int firstCallIndex = randIndex(callExpressions.size());
    unsigned int secondCallIndex;
    do {
      secondCallIndex = randIndex(callExpressions.size());
    } while (firstCallIndex == secondCallIndex);

    CallExpr* firstCall = callExpressions[firstCallIndex];
    CallExpr* secondCall = callExpressions[secondCallIndex];

    // Step 3: Perform the swap
    std::string firstCallSource = getSourceText(firstCall);
    std::string secondCallSource = getSourceText(secondCall);

    getRewriter().ReplaceText(firstCall->getSourceRange(), secondCallSource);
    getRewriter().ReplaceText(secondCall->getSourceRange(), firstCallSource);

    return true;
  }

private:
  std::vector<CallExpr*> callExpressions;
};

static RegisterMutator<ReverseFuncCallOrder> M("u3.ReverseFuncCallOrder", "Swaps the order of execution of two consecutive function calls.");
