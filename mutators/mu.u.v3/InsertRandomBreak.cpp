#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class InsertRandomBreak : public Mutator,
                          public clang::RecursiveASTVisitor<InsertRandomBreak> {
public:
  using Mutator::Mutator;

  bool VisitStmt(Stmt *S) {
    if (isa<WhileStmt>(S) || isa<ForStmt>(S) || isa<DoStmt>(S)) {
      TheLoops.push_back(S);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLoops.empty()) return false;

    Stmt *selectedLoop = randElement(TheLoops);

    std::vector<Stmt*> compoundStmts;
    for (auto *B : selectedLoop->children()) {
      // Skip null or non-CompoundStmt children
      if (B == nullptr || !isa<CompoundStmt>(B)) continue;
      compoundStmts.push_back(B);
    }
    if (compoundStmts.empty()) return false;

    Stmt *compoundStmt = randElement(compoundStmts);
    SourceLocation insertLocation = compoundStmt->getEndLoc().getLocWithOffset(-1);

    // Insert 'break' at the random location within the compound statement
    getRewriter().InsertText(insertLocation, "break; ", true, true);

    return true;
  }

private:
  std::vector<Stmt*> TheLoops;
};

static RegisterMutator<InsertRandomBreak> M("u3.InsertRandomBreak", "Inserts a random break statement within a loop.");
