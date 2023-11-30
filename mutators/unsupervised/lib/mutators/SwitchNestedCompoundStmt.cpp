#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Lex/Lexer.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class SwitchNestedCompoundStmt : public Mutator,
                                 public clang::RecursiveASTVisitor<SwitchNestedCompoundStmt> {

private:
  std::vector<CompoundStmt*> TheCompoundStmts;

public:
  using Mutator::Mutator;

  bool VisitCompoundStmt(CompoundStmt *stmt) {
    TheCompoundStmts.push_back(stmt);
    return true;
  }

  std::vector<CompoundStmt*> findNestedCompoundStmts(CompoundStmt* stmt) {
    std::vector<CompoundStmt*> nestedStmts;
    for (auto* S : stmt->body()) {
      if (auto* nestedStmt = dyn_cast<CompoundStmt>(S)) {
        nestedStmts.push_back(nestedStmt);
      }
    }
    return nestedStmts;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    CompoundStmt *outerStmt = nullptr;
    CompoundStmt *innerStmt = nullptr;

    for (auto stmt : TheCompoundStmts) {
      auto nestedStmts = findNestedCompoundStmts(stmt);
      if (!nestedStmts.empty()) {
        outerStmt = stmt;
        innerStmt = randElement(nestedStmts);
        break;
      }
    }

    if (!outerStmt || !innerStmt) {
      llvm::errs() << "No suitable CompoundStmts found for mutation.\n";
      return false;
    }

    // get the source text of the inner statements without braces
    std::string outerText = getSourceText(SourceRange(outerStmt->body_front()->getBeginLoc(), outerStmt->body_back()->getEndLoc()));
    std::string innerText = getSourceText(SourceRange(innerStmt->body_front()->getBeginLoc(), innerStmt->body_back()->getEndLoc()));

    // perform rewrite
    getRewriter().ReplaceText(SourceRange(outerStmt->body_front()->getBeginLoc(), outerStmt->body_back()->getEndLoc()), innerText);
    getRewriter().ReplaceText(SourceRange(innerStmt->body_front()->getBeginLoc(), innerStmt->body_back()->getEndLoc()), outerText);

    return true;
  }

  std::string getSourceText(SourceRange range) {
    return Lexer::getSourceText(
      CharSourceRange::getTokenRange(range),
      getASTContext().getSourceManager(), 
      getASTContext().getLangOpts()
    ).str();
  }
};

static RegisterMutator<SwitchNestedCompoundStmt> X(
    "SwitchNestedCompoundStmt", "Switch bodies of two nested CompoundStmts");