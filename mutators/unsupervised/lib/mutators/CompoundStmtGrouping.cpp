#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class CompoundStmtGrouping : public Mutator,
                             public RecursiveASTVisitor<CompoundStmtGrouping> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());

    // Check if there are at least two statements to group
    if (TheStatements.size() < 2) return false;

    // Find the start and end locations of the statements to be grouped
    SourceLocation startLoc = TheStatements.front()->getBeginLoc();
    SourceLocation endLoc = TheStatements.back()->getEndLoc();

    // Get the source range of the original statements
    SourceRange originalRange(startLoc, endLoc);

    // Get the source text of the original statements
    std::string originalText = getOriginalText(originalRange);

    // Create a new nested CompoundStmt with the statements to be grouped
    CompoundStmt *newStmt =
        CompoundStmt::Create(getASTContext(), TheStatements, startLoc, endLoc);

    // Get the rewritten source text of the new nested CompoundStmt
    std::string newStmtText = getSourceText(newStmt).str();

    // Add the necessary indentation to the new statement text
    std::string indentedNewStmtText = addIndentation(newStmtText, startLoc);

    // Replace the original statements with the new nested CompoundStmt
    getRewriter().ReplaceText(originalRange, indentedNewStmtText);

    return true;
  }

  bool VisitCompoundStmt(CompoundStmt *CS) {
    // Collect all the statements within the CompoundStmt
    for (auto *stmt : CS->body()) {
      TheStatements.push_back(stmt);
    }
    return true;
  }

private:
  std::vector<Stmt *> TheStatements;

  std::string getOriginalText(SourceRange range) {
    const SourceManager &sourceMgr = getASTContext().getSourceManager();
    const LangOptions &langOpts = getASTContext().getLangOpts();
    return Lexer::getSourceText(CharSourceRange::getTokenRange(range),
                                sourceMgr, langOpts).str();
  }

  std::string addIndentation(const std::string &text, SourceLocation startLoc) {
    const SourceManager &sourceMgr = getASTContext().getSourceManager();

    // Get the indentation level of the original statements
    unsigned indentLevel = sourceMgr.getSpellingColumnNumber(startLoc) - 1;
    std::string indentation(indentLevel, ' ');

    std::string indentedText;
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
      indentedText += indentation + line + "\n";
    }
    return indentedText;
  }
};

static RegisterMutator<CompoundStmtGrouping>
    M("CompoundStmtGrouping", "Group consecutive statements within a CompoundStmt.");