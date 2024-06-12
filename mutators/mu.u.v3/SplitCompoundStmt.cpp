#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class SplitCompoundStmt : public Mutator,
                          public clang::RecursiveASTVisitor<SplitCompoundStmt> {
public:
  using Mutator::Mutator;

  bool VisitCompoundStmt(CompoundStmt *CS) {
    if(CS->size() > 1) // Only consider CompoundStmt with more than one statement.
      TheStmts.push_back(CS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheStmts.empty()) return false;

    CompoundStmt *chosenStmt = randElement(TheStmts);
    std::string allStmts;

    for (Stmt *s : chosenStmt->body()) {
        if (isa<DeclStmt>(s) || isa<CallExpr>(s))
            continue; // Skip function declarations and calls

        std::string stmtCode = Lexer::getSourceText(CharSourceRange::getTokenRange(s->getSourceRange()),
                                                     getASTContext().getSourceManager(), LangOptions(), nullptr).str();
        // Wrap each statement into its own compound statement
        allStmts += "{ " + stmtCode + "; }\n";
    }

    // Replace the CompoundStmt with the allStmts string, effectively splitting the CompoundStmt into individual compound statements.
    SourceLocation startLoc = chosenStmt->getLBracLoc();
    SourceLocation endLoc = chosenStmt->getRBracLoc();

    getRewriter().ReplaceText(SourceRange(startLoc, endLoc), allStmts);

    return true;
  }

private:
  std::vector<CompoundStmt*> TheStmts;
};

static RegisterMutator<SplitCompoundStmt> M("u3.SplitCompoundStmt", "This mutator selects a compound statement (i.e., a block of statements enclosed in {}) and splits it into multiple individual compound statements.");
