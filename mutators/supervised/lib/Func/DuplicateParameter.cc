#include <algorithm>
#include <random>
#include <vector>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Func/DuplicateParameter.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<DuplicateParameter> M(
    "duplicate-parameter", "Duplicate a function's parameter.");

bool DuplicateParameter::VisitFunctionDecl(FunctionDecl *FD) {
  if (FD->getNumParams() > 0) TheFuncs.push_back(FD);
  return true;
}

bool DuplicateParameter::VisitCallExpr(CallExpr *CE) {
  if (FunctionDecl *FD = CE->getDirectCallee()) {
    if (FD->getNumParams() > 0) FuncToCalls[FD].push_back(CE);
  }
  return true;
}

bool DuplicateParameter::mutate() {
  TraverseAST(getASTContext());
  if (TheFuncs.empty()) return false;

  FunctionDecl *func = randElement(TheFuncs);

  unsigned index = rand() % func->getNumParams();
  ParmVarDecl *param = func->getParamDecl(index);

  // Generate unique name for the duplicated parameter
  std::string newName = generateUniqueName(param->getNameAsString());

  // Create the new parameter declaration
  std::string newParamDecl;
  llvm::raw_string_ostream ostream(newParamDecl);
  param->getType().print(ostream, getASTContext().getPrintingPolicy(), newName);
  ostream.flush();

  // Add it to the function declaration
  clang::SourceLocation endLoc =
      param->getEndLoc().getLocWithOffset(param->getNameAsString().size());
  getRewriter().InsertTextAfter(endLoc, ", " + newParamDecl);

  // Duplicate corresponding CallExpr argument
  for (auto *CE : FuncToCalls[func]) {
    Expr *arg = CE->getArg(index);
    std::string argStr = getSourceText(arg).str();
    getRewriter().ReplaceText(arg->getSourceRange(), argStr + ", " + argStr);
  }

  return true;
}
