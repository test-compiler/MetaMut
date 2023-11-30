#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

#include "MutatorManager.h"
#include "Stmt/GroupStmtsIntoCompound.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<GroupStmtsIntoCompound> M("group-stmts-into-compound",
    "Group a list of continuous stmt into compound stmt.");

bool GroupStmtsIntoCompound::VisitCompoundStmt(CompoundStmt *CS) {
  if (isMutationSite(CS)) TheCompStmts.push_back(CS);
  return true;
}

bool GroupStmtsIntoCompound::mutate() {
  TraverseAST(getASTContext());
  if (TheCompStmts.empty()) return false;

  CompoundStmt *selectedCS = randElement(TheCompStmts);

  if (selectedCS->size() <= 1) return false; // Need at least two stmts to group

  // Randomly select a subrange of the body to group
  size_t startPos = randIndex(selectedCS->size());
  size_t endPos = startPos + randIndex(selectedCS->size() - startPos);

  std::string compoundStmtText = "{\n";
  for (size_t i = startPos; i <= endPos; ++i) {
    Stmt *stmt = *(selectedCS->body_begin() + i);
    SourceRange range = stmt->getSourceRange();

    // Extract the statement text
    std::string stmtText = getSourceText(stmt).str();

    // Remove the statement from the original location
    getRewriter().RemoveText(range);

    compoundStmtText += stmtText + ";\n";
  }
  compoundStmtText += "}\n";

  // Insert the new compound statement before the first statement in the group
  getRewriter().InsertTextBefore(
      (*(selectedCS->body_begin() + startPos))->getBeginLoc(),
      compoundStmtText);

  return true;
}
