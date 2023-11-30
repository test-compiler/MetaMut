#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class InsertRandomArrayIndexMutation
    : public Mutator,
      public clang::RecursiveASTVisitor<InsertRandomArrayIndexMutation> {

public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheSubscripts.empty()) return false;

    // Pick a random ArraySubscriptExpr
    ArraySubscriptExpr *expr = randElement(TheSubscripts);

    // Calculate new index
    int newIndex = calculateNewIndex(expr);

    // Perform rewrite
    getRewriter().ReplaceText(expr->getIdx()->getSourceRange(), std::to_string(newIndex));

    return true;
  }

  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE) {
    if (ASE->getIdx()->getType()->isIntegerType()) {
      TheSubscripts.push_back(ASE);
    }
    return true;
  }

private:
  std::vector<clang::ArraySubscriptExpr *> TheSubscripts;

  int calculateNewIndex(ArraySubscriptExpr* expr) {
    // Check if the index is a constant integer
    if (expr->getIdx()->isIntegerConstantExpr(getASTContext())) {
      // If it is, use its value
      return expr->getIdx()->EvaluateKnownConstInt(getASTContext()).getExtValue();
    } else {
      // If it's not, just return 0 as a default value
      return 0;
    }
  }
};

static RegisterMutator<InsertRandomArrayIndexMutation> M(
    "InsertRandomArrayIndexMutation", "Mutate an array index randomly.");