#include <algorithm>
#include <random>
#include <vector>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Func/ShuffleParameters.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<ShuffleParameters> M(
    "shuffle-parameters", "Shuffle a function's parameters.");

bool ShuffleParameters::VisitFunctionDecl(FunctionDecl *FD) {
  if (FD->getNumParams() > 1) TheFuncs.push_back(FD);
  return true;
}

bool ShuffleParameters::VisitCallExpr(CallExpr *CE) {
  if (FunctionDecl *FD = CE->getDirectCallee()) {
    if (FD->getNumParams() > 1) FuncToCalls[FD].push_back(CE);
  }
  return true;
}

bool ShuffleParameters::mutate() {
  TraverseAST(getASTContext());
  if (TheFuncs.empty()) return false;

  FunctionDecl *oldFunc = randElement(TheFuncs);

  std::vector<unsigned> indices(oldFunc->getNumParams());
  std::iota(indices.begin(), indices.end(), 0); // Fill with 0, 1, ..., n-1
  std::shuffle(
      indices.begin(), indices.end(), getManager().getRandomGenerator());

  // Shuffle function parameters
  for (unsigned i = 0; i < oldFunc->getNumParams(); ++i) {
    ParmVarDecl *oldParam = oldFunc->getParamDecl(i);
    ParmVarDecl *newParam = oldFunc->getParamDecl(indices[i]);

    // Get the entire declaration of the new parameter
    std::string newParamDecl = getSourceText(newParam).str();
    getRewriter().ReplaceText(oldParam->getSourceRange(), newParamDecl);
  }

  // Shuffle corresponding CallExpr arguments
  for (auto *CE : FuncToCalls[oldFunc]) {
    for (unsigned i = 0; i < CE->getNumArgs(); ++i) {
      Expr *oldArg = CE->getArg(i);
      Expr *newArg = CE->getArg(indices[i]);
      getRewriter().ReplaceText(
          oldArg->getSourceRange(), getSourceText(newArg).str());
    }
  }

  return true;
}
