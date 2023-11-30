#include "MutatorManager.h"
#include "Stmt/DuplicateStatement.h"
#include <algorithm>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <random>

using namespace ysmut;

static RegisterMutator<DuplicateStatement> M(
    "duplicate-stmt", "Duplicate a random non-declaration statement.");

bool DuplicateStatement::VisitCompoundStmt(clang::CompoundStmt *CS) {
  for (clang::Stmt *S : CS->body()) {
    if (isMutationSite(S))
      if (!clang::isa<clang::DeclStmt>(S)) { Stmts.push_back(S); }
  }
  return true;
}

bool DuplicateStatement::mutate() {
  TraverseAST(getASTContext());

  if (Stmts.empty()) return false;

  auto stmt = randElement(Stmts);

  // Store the original source code
  std::string originalSource =
      getRewriter().getRewrittenText(stmt->getSourceRange());

  // Duplicate the statement
  std::string duplicateSource = originalSource + ";" + originalSource;

  // Replace the statement with its duplicate
  getRewriter().ReplaceText(stmt->getSourceRange(), duplicateSource);

  return true;
}
