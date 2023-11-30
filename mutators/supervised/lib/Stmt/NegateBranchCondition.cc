#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include "MutatorManager.h"
#include "Stmt/NegateBranchCondition.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<NegateBranchCondition> M(
    "negate-branch-cond", "Negate the condition of a branch statement.");

bool NegateBranchCondition::VisitIfStmt(IfStmt *IS) {
  if (isMutationSite(IS)) TheIfStmts.push_back(IS);
  return true;
}

bool NegateBranchCondition::mutate() {
  TraverseAST(getASTContext());
  if (TheIfStmts.empty()) return false;

  IfStmt *stmt = randElement(TheIfStmts);

  // perform rewrite
  SourceRange CondRange = stmt->getCond()->getSourceRange();
  std::string NewCond =
      "!(" +
      Lexer::getSourceText(CharSourceRange::getTokenRange(CondRange),
          getASTContext().getSourceManager(), getASTContext().getLangOpts())
          .str() +
      ")";

  getRewriter().ReplaceText(CondRange, NewCond);

  return true;
}
