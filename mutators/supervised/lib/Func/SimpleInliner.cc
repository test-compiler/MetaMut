#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

#include "Func/SimpleInliner.h"
#include "MutatorManager.h"

using namespace ysmut;

static RegisterMutator<SimpleInliner> M(
    "simple-inliner", "Inline function calls.");

bool SimpleInliner::VisitCallExpr(clang::CallExpr *CE) {
  if (isMutationSite(CE))
    TheCalls.push_back({clang::dyn_cast<clang::CallExpr>(CE), CurrentParent});
  return true;
}

bool SimpleInliner::TraverseCompoundStmt(clang::CompoundStmt *CS) {
  clang::Stmt *OldParent = CurrentParent;
  for (auto I = CS->body_begin(), E = CS->body_end(); I != E; ++I) {
    CurrentParent = *I;
    TraverseStmt(*I);
    CurrentParent = OldParent;
  }
  return true;
}

bool SimpleInliner::mutate() {
  TraverseAST(getASTContext());
  if (TheCalls.empty()) return false;

  auto [call, parent] = randElement(TheCalls);
  auto func = call->getDirectCallee();
  if (!func && !func->getBody())
    return false; // Can't handle function pointers for now

  // Build a mapping from function arguments to call arguments
  std::string parmsDecl;
  for (size_t i = 0; i < func->param_size() && i < call->getNumArgs(); ++i) {
    parmsDecl += formatAsDecl(func->getParamDecl(i)->getType(),
        func->getParamDecl(i)->getNameAsString());
    parmsDecl += " = ";
    parmsDecl += getSourceText(call->getArg(i)).str();
    parmsDecl += ";\n";
  }

  // Replace function parameters with call arguments in the function body
  std::string funcBody = getSourceText(func->getBody()).str();

  // Replace 'return ' with 'retval = '
  std::string retval = generateUniqueName("retval");
  std::string retvalDecl = formatAsDecl(func->getReturnType(), retval) + ";\n";
  size_t pos = 0;
  while ((pos = funcBody.find("return", pos)) != std::string::npos) {
    if (std::isalnum(funcBody[pos - 1]) ||
        std::isalnum(funcBody[pos + strlen("return")])) {
      pos += strlen("return");
    } else {
      funcBody.replace(pos, 7, retval + " = ");
      pos += retval.length() + 3;
    }
  }

  // Replace the parent statement of the function call with the modified
  // function body
  getRewriter().InsertText(
      parent->getBeginLoc(), retvalDecl + "{\n" + parmsDecl + funcBody + "}");
  getRewriter().ReplaceText(call->getSourceRange(), retval);
  return true;
}
