#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class InsertRandomContinue : public Mutator,
                             public clang::RecursiveASTVisitor<InsertRandomContinue> {
public:
  using Mutator::Mutator;

  bool VisitStmt(Stmt *S) {
    // Only consider loops
    if (isa<ForStmt>(S) || isa<WhileStmt>(S) || isa<DoStmt>(S)) {
      Loops.push_back(S);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (Loops.empty()) {
      return false;
    }

    Stmt *selectedLoop = randElement(Loops);
    std::string continueStmt = "continue;";

    auto &RGen = getManager().getRandomGenerator();
    auto begin = selectedLoop->child_begin();
    auto end = selectedLoop->child_end();
    unsigned distance = std::distance(begin, end);
    
    // generate a random position within the range
    unsigned Pos = RGen() % distance;

    auto It = selectedLoop->child_begin();
    std::advance(It, Pos);

    // Insert 'continue'
    getRewriter().InsertTextAfter((*It)->getEndLoc(), continueStmt);

    return true;
  }

private:
  std::vector<Stmt *> Loops;
};

static RegisterMutator<InsertRandomContinue> M("u3.InsertRandomContinue", "Insert a random 'continue' statement within a loop.");
