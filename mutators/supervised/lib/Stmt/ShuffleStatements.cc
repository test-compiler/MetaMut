#include "MutatorManager.h"
#include "Stmt/ShuffleStatements.h"
#include <algorithm>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <random>

using namespace ysmut;
using namespace clang;

static RegisterMutator<ShuffleStatements> M(
    "shuffle-stmts", "Shuffle continuous non-declaration statements.");

bool ShuffleStatements::VisitCompoundStmt(CompoundStmt *CS) {
  CurrentGroup.clear();
  for (Stmt *S : CS->body()) {
    if (isa<DeclStmt>(S)) {
      if (CurrentGroup.size() > 1)
        StmtGroups.push_back(std::move(CurrentGroup));
      CurrentGroup.clear();
    } else {
      CurrentGroup.push_back(S);
    }
  }
  if (CurrentGroup.size() > 1) StmtGroups.push_back(std::move(CurrentGroup));
  return true;
}

bool ShuffleStatements::mutate() {
  TraverseAST(getASTContext());

  if (StmtGroups.empty()) return false;

  auto &group = randElement(StmtGroups);

  // Store the original source code
  std::vector<std::string> originalSources;
  for (auto stmt : group) {
    originalSources.push_back(
        getRewriter().getRewrittenText(stmt->getSourceRange()) + ";");
  }

  // Shuffle the statements
  std::shuffle(group.begin(), group.end(), getManager().getRandomGenerator());

  // Replace each statement with its new source code
  for (std::size_t i = 0; i < group.size(); ++i) {
    getRewriter().ReplaceText(group[i]->getSourceRange(), originalSources[i]);
  }

  return true;
}
