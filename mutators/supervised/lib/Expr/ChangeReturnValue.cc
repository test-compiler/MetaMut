#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

#include "Expr/ChangeReturnValue.h"
#include "MutatorManager.h"

using namespace ysmut;

static RegisterMutator<ChangeReturnValue> M(
    "change-return-value", "Change a function's return value.");

bool ChangeReturnValue::VisitReturnStmt(clang::ReturnStmt *RS) {
  if (isMutationSite(RS)) TheReturns.push_back(RS);
  return true;
}

bool ChangeReturnValue::mutate() {
  TraverseAST(getASTContext());
  if (TheReturns.empty()) return false;

  clang::ReturnStmt *ret = randElement(TheReturns);
  clang::Expr *retval = ret->getRetValue();
  if (!retval) return false;

  // If the return value is an integer literal, add 1 to the value
  if (clang::IntegerLiteral *IL =
          clang::dyn_cast<clang::IntegerLiteral>(retval)) {
    llvm::APInt val = IL->getValue();
    val += getManager().randint(-100, 100);
    getRewriter().ReplaceText(IL->getSourceRange(), val.toString(10, true));
  } else {
    getRewriter().ReplaceText(retval->getSourceRange(), "{0}");
  }

  return true;
}
