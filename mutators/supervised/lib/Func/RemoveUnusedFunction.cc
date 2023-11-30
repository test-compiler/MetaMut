#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Func/RemoveUnusedFunction.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<RemoveUnusedFunction> X(
    "remove-unused-function", "Remove an unused function.");

bool RemoveUnusedFunction::VisitFunctionDecl(FunctionDecl *FD) {
  // Initialize each function as unused
  if (TheFuncs.find(FD) == TheFuncs.end()) TheFuncs[FD] = false;
  return true;
}

bool RemoveUnusedFunction::VisitCallExpr(CallExpr *CE) {
  // Mark function as used if it's called
  if (FunctionDecl *FD = CE->getDirectCallee()) { TheFuncs[FD] = true; }
  return true;
}

bool RemoveUnusedFunction::mutate() {
  TraverseAST(getASTContext());
  // Filter out used functions
  std::vector<FunctionDecl *> UnusedFuncs;
  for (auto it = TheFuncs.begin(); it != TheFuncs.end(); ++it) {
    if (!it->second) UnusedFuncs.push_back(it->first);
  }
  if (UnusedFuncs.empty()) return false;

  FunctionDecl *unusedFunc = randElement(UnusedFuncs);
  getRewriter().ReplaceText(unusedFunc->getSourceRange(), "");

  return true;
}
