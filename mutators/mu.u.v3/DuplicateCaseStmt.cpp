#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class DuplicateCaseStmt : public Mutator,
                          public clang::RecursiveASTVisitor<DuplicateCaseStmt> {
public:
  using Mutator::Mutator;

  bool VisitSwitchStmt(SwitchStmt *SS) {
    CurrentSwitchStmt = SS;  // Update the current SwitchStmt
    return true;  // Continue traversal
  }

  bool VisitCaseStmt(CaseStmt *CS) {
    TheStmts.push_back(std::make_pair(CS, CurrentSwitchStmt));
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheStmts.empty()) return false;

    auto [chosenCaseStmt, parentSwitchStmt] = randElement(TheStmts);

    // Generate a unique case value and print statement
    unsigned uniqueCaseValue = getManager().getRandomGenerator()();
    std::string uniquePrintStatement = "printf(\"Unique case: " + std::to_string(uniqueCaseValue) + "\\n\");";

    // Add the new case statement just before the closing brace of the SwitchStmt parent
    CompoundStmt *parentCompoundStmt = cast<CompoundStmt>(parentSwitchStmt->getBody());
    std::string newCaseStmt = "\ncase " + std::to_string(uniqueCaseValue) + ":\n" + uniquePrintStatement + "\nbreak;";
    getRewriter().InsertTextBefore(parentCompoundStmt->getRBracLoc(), newCaseStmt);

    return true;
  }

private:
  std::vector<std::pair<CaseStmt*, SwitchStmt*>> TheStmts;
  SwitchStmt* CurrentSwitchStmt = nullptr;  // For keeping track of the parent SwitchStmt
};

static RegisterMutator<DuplicateCaseStmt> M("u3.DuplicateCaseStmt", "This mutator selects a case statement within a switch block and duplicates it, appending the duplicate case statement to the end of the switch block.");
