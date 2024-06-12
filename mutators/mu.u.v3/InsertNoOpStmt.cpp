#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertNoOpStmt : public Mutator,
                       public clang::RecursiveASTVisitor<InsertNoOpStmt> {
public:
  using Mutator::Mutator;

  bool VisitStmt(Stmt *s) {
    if (isa<CompoundStmt>(s)) {
      TheStmts.push_back(s);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheStmts.empty()) return false;

    Stmt *chosenStmt = randElement(TheStmts);
    SourceLocation endLoc = getLocForEndOfToken(chosenStmt->getEndLoc());
    getRewriter().InsertTextAfter(endLoc, ";");

    return true;
  }

private:
  std::vector<Stmt*> TheStmts;
};

static RegisterMutator<InsertNoOpStmt> M("u3.InsertNoOpStmt", "This mutator selects a random compound statement in the code and inserts a No-op (no operation) statement after it.");
