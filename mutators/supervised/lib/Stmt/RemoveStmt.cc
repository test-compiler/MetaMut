#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

#include "MutatorManager.h"
#include "Stmt/RemoveStmt.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<RemoveStmt> M("remove-stmt", "Remove a statement.");

bool RemoveStmt::VisitStmt(Stmt *S) {
  if (isMutationSite(S)) TheStatements.push_back(S);
  return true;
}

bool RemoveStmt::mutate() {
  TraverseAST(getASTContext());
  if (TheStatements.empty()) return false;

  Stmt *stmt = randElement(TheStatements);

  // perform rewrite
  SourceRange StmtRange = stmt->getSourceRange();
  getRewriter().ReplaceText(StmtRange, "");

  return true;
}
