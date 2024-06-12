#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class RemoveRandomFuncParameter : public Mutator,
                                  public clang::RecursiveASTVisitor<RemoveRandomFuncParameter> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctions.empty()) return false;

    FunctionDecl *FD = randElement(TheFunctions);
    unsigned totalNumOfParams = FD->getNumParams();
    if (totalNumOfParams == 0) return false;
    int paramPos = randIndex(totalNumOfParams);

    // Make sure we're not removing function pointers
    if (FD->getParamDecl(paramPos)->getType()->isFunctionPointerType()) return false;

    // Remove the parameter from function declaration
    removeParmFromFuncDecl(FD->getParamDecl(paramPos), totalNumOfParams, paramPos);

    // Remove the argument from all function calls
    for (auto *CE : TheCalls[FD]) {
      removeArgFromExpr(CE, paramPos);
    }

    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD->getNumParams() > 0) {
      TheFunctions.push_back(FD);
    }
    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    if (FunctionDecl *FD = CE->getDirectCallee()) {
      TheCalls[FD].push_back(CE);
    }
    return true;
  }

private:
  std::vector<FunctionDecl *> TheFunctions;
  std::map<FunctionDecl *, std::vector<CallExpr *>> TheCalls;
};

static RegisterMutator<RemoveRandomFuncParameter> M("u3.RemoveRandomFuncParameter",
    "Identifies a random parameter from a function and removes it from both the declaration and all function calls."
);
