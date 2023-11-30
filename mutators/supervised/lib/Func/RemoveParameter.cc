#include <algorithm>
#include <random>
#include <vector>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Sema/Sema.h>

#include "Func/RemoveParameter.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<RemoveParameter> M(
    "remove-parameter", "Remove a function's parameter.");

bool RemoveParameter::VisitFunctionDecl(FunctionDecl *FD) {
  if (FD->getNumParams() > 0) TheFuncs.push_back(FD);
  return true;
}

bool RemoveParameter::VisitCallExpr(CallExpr *CE) {
  if (FunctionDecl *FD = CE->getDirectCallee()) {
    if (FD->getNumParams() > 0) FuncToCalls[FD].push_back(CE);
  }
  return true;
}

bool RemoveParameter::mutate() {
  TraverseAST(getASTContext());
  if (TheFuncs.empty()) return false;

  std::shuffle(
      TheFuncs.begin(), TheFuncs.end(), getManager().getRandomGenerator());
  for (FunctionDecl *func : TheFuncs) {
    unsigned i = randIndex(func->getNumParams());
    ParmVarDecl *param = func->getParamDecl(i);

    // Remove the parameter from function declaration
    SourceLocation Start =
        (i > 0 && i + 1 == func->getNumParams())
            ? getLocForEndOfToken(func->getParamDecl(i - 1)->getEndLoc())
            : param->getBeginLoc();
    SourceLocation End = i + 1 < func->getNumParams()
                             ? getLocForEndOfToken(param->getEndLoc())
                             : param->getEndLoc();
    if (func->isVariadic() && i + 1 == func->getNumParams())
      End = getLocForEndOfToken(param->getEndLoc());
    getRewriter().RemoveText(SourceRange(Start, End));

    // insert definition to body
    if (func->getBody())
      getRewriter().InsertTextAfter(
          getLocForEndOfToken(func->getBody()->getBeginLoc()),
          getSourceText(param).str() + ";");

    // Remove the corresponding argument from each call expression
    for (CallExpr *CE : FuncToCalls[func]) {
      Expr *arg = CE->getArg(i);
      SourceLocation argStart =
          (i > 0 && i + 1 == CE->getNumArgs())
              ? getLocForEndOfToken(CE->getArg(i - 1)->getEndLoc())
              : arg->getBeginLoc();
      SourceLocation argEnd = i + 1 < CE->getNumArgs()
                                  ? getLocForEndOfToken(arg->getEndLoc())
                                  : arg->getEndLoc();
      getRewriter().RemoveText(SourceRange(argStart, argEnd));
    }

    return true; // Stop after removing one unused parameter
  }

  return false;
}
