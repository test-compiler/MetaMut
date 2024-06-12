// Include required headers
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

// The class definition
class InvertFunctionCallOrder : public Mutator,
                                public clang::RecursiveASTVisitor<InvertFunctionCallOrder> {
public:
  using Mutator::Mutator;

  // This function is called by clang::RecursiveASTVisitor for every CallExpr node
  bool VisitCallExpr(clang::CallExpr *E) {
    TheCalls.push_back(E);
    return true;
  }

  // This function is used to perform the mutation
  bool mutate() override {
    // Step 1, Traverse the AST
    TraverseAST(getASTContext());

    // Step 3, Randomly select a mutation instance
    if (TheCalls.size() < 2) return false; // ensure we have at least two function calls to swap
    unsigned index = randIndex(TheCalls.size() - 1);

    // Step 4, Checking mutation validity (Not required in this case)

    // Step 5, Perform mutation
    clang::CallExpr *firstCall = TheCalls[index];
    clang::CallExpr *secondCall = TheCalls[index + 1];

    clang::SourceRange firstRange = firstCall->getSourceRange();
    clang::SourceRange secondRange = secondCall->getSourceRange();

    std::string firstSourceText = getSourceText(firstCall);
    std::string secondSourceText = getSourceText(secondCall);

    getRewriter().ReplaceText(firstRange, secondSourceText);
    getRewriter().ReplaceText(secondRange, firstSourceText);

    // Step 6, Return true if changed
    return true;
  }

private:
  std::vector<clang::CallExpr *> TheCalls;
};

// Register the mutator so that it can be used
static RegisterMutator<InvertFunctionCallOrder> M("u3.InvertFunctionCallOrder", "Swap two consecutive function calls");
