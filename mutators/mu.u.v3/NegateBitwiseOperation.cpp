#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class NegateBitwiseOperation : public Mutator,
                               public clang::RecursiveASTVisitor<NegateBitwiseOperation> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    BinaryOperator *expr = randElement(TheOperators);

    // perform rewrite
    SourceLocation endLoc = Lexer::getLocForEndOfToken(expr->getOperatorLoc(), 0, getSourceManager(), getCompilerInstance().getLangOpts());
    getRewriter().InsertTextAfter(endLoc, "~");

    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator *BO) {
    if(BO->isBitwiseOp())
      TheOperators.push_back(BO);
    return true;
  }

private:
  std::vector<clang::BinaryOperator *> TheOperators;
};

static RegisterMutator<NegateBitwiseOperation> M("u3.NegateBitwiseOperation", "Negate the result of a bitwise operation.");
