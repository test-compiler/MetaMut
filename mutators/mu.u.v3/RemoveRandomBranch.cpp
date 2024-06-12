#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/ASTMatchers/ASTMatchers.h>

using namespace clang;

class RemoveRandomBranch : public Mutator,
                      public clang::RecursiveASTVisitor<RemoveRandomBranch> {
public:
  using Mutator::Mutator;

  bool VisitIfStmt(IfStmt *IF) {
    TheStmts.push_back(IF);
    return true;
  }

  bool VisitSwitchStmt(SwitchStmt *SS) {
    TheStmts.push_back(SS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheStmts.empty()) return false;

    Stmt *chosenStmt = randElement(TheStmts);
    SourceRange range;

    if (IfStmt *chosenIf = dyn_cast<IfStmt>(chosenStmt)) {
      // If it is an IfStmt, choose a random branch (true/false) to remove
      bool removeThenBranch = randBool();
      Stmt *branchToRemove = removeThenBranch ? chosenIf->getThen() : chosenIf->getElse();
      
      if (branchToRemove == nullptr) return false;  // Ensure we have a valid branch to remove
      range = branchToRemove->getSourceRange();

      // If removing the 'then' branch and there is an 'else' branch, remove the 'else' keyword
      if (removeThenBranch && chosenIf->getElse()) {
        auto elseLoc = chosenIf->getElseLoc();
        getRewriter().RemoveText(elseLoc, 4); // Remove 'else'
      }
      // If removing the 'else' branch, remove the entire 'if' statement
      else if (!removeThenBranch) {
        range = chosenIf->getSourceRange();
      }
    } 
    else if (SwitchStmt *chosenSwitch = dyn_cast<SwitchStmt>(chosenStmt)) {
      // If it is a SwitchStmt, choose a random case to remove
      std::vector<CaseStmt*> caseStmts;
      for (Stmt *child : chosenSwitch->children()) {
        if (CaseStmt *caseStmt = dyn_cast<CaseStmt>(child)) {
          caseStmts.push_back(caseStmt);
        }
      }
      if (caseStmts.empty()) return false;  // Ensure we have cases to remove
      CaseStmt *caseToRemove = randElement(caseStmts);
      range = caseToRemove->getSourceRange();
    } 
    else {
      return false;  // Unrecognized Stmt type
    }

    getRewriter().RemoveText(getExpansionRange(range));

    return true;
  }

private:
  std::vector<Stmt*> TheStmts;
};

static RegisterMutator<RemoveRandomBranch> M("u3.RemoveRandomBranch", "This mutator selects a conditional statement (if-else or switch-case) and removes a random branch from it.");
