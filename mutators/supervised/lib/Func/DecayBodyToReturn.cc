#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Func/DecayBodyToReturn.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<DecayBodyToReturn> M("decay-body-to-return",
    "Remove all stmts inside a function's body except for DeclStmt and "
    "ReturnStmt.");

bool DecayBodyToReturn::VisitFunctionDecl(FunctionDecl *FD) {
  if (isMutationSite(FD)) TheFuncs.push_back(FD);
  return true;
}

void DecayBodyToReturn::handleStmt(Stmt *S) {
  if (isa<CompoundStmt>(S)) {
    for (Stmt *child : S->children()) {
      // Recursively handle the children
      if (child) handleStmt(child);
    }
  } else if (!isa<DeclStmt>(S) && !isa<ReturnStmt>(S)) {
    getRewriter().RemoveText(S->getSourceRange());
  }
}

bool DecayBodyToReturn::mutate() {
  TraverseAST(getASTContext());
  if (TheFuncs.empty()) return false;

  FunctionDecl *func = randElement(TheFuncs);

  // Ensure the function has a body
  Stmt *body = func->getBody();
  if (!body) return false;

  handleStmt(body);
  return true;
}
