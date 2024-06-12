#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class DuplicateStmt : public Mutator,
                      public clang::RecursiveASTVisitor<DuplicateStmt> {
public:
  using Mutator::Mutator;

  bool VisitStmt(Stmt *S) {
    TheStmts.push_back(S);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheStmts.empty()) return false;

    Stmt *S = randElement(TheStmts);
    
    std::string StmtStr = getSourceText(S);

    getRewriter().InsertTextAfter(S->getEndLoc(), StmtStr);

    return true;
  }

private:
  std::vector<Stmt*> TheStmts;
};

static RegisterMutator<DuplicateStmt> M("u3.DuplicateStmt", "This mutator selects a statement in the code and duplicates it, effectively executing the statement twice in a row.");
