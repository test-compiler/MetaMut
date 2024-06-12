#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ModifyFuncCallToNullArg
    : public Mutator,
      public clang::RecursiveASTVisitor<ModifyFuncCallToNullArg> {

public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (FuncCalls.empty()) return false;

    CallExpr *expr = randElement(FuncCalls);

    // Choose a random argument
    unsigned i = randIndex(expr->getNumArgs());

    // perform rewrite
    SourceRange argRange = getExpansionRange(expr->getArg(i)->getSourceRange());
    getRewriter().ReplaceText(argRange, "NULL");

    return true;
  }

  bool VisitCallExpr(clang::CallExpr *E) {
    // Consider only functions with arguments
    if (E->getNumArgs() > 0) {
      FuncCalls.push_back(E);
    }
    return true;
  }

private:
  std::vector<clang::CallExpr *> FuncCalls;
};

static RegisterMutator<ModifyFuncCallToNullArg> M("u3.ModifyFuncCallToNullArg", 
    "This mutator selects a function call in the code and replaces one of its arguments with a null value");
