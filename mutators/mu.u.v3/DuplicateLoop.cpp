#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class DuplicateLoop : public Mutator,
                      public RecursiveASTVisitor<DuplicateLoop> {
public:
  using Mutator::Mutator;

  bool VisitForStmt(ForStmt* FS) {
    Loops.push_back(FS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (Loops.empty()) return false;

    ForStmt* selectedLoop = randElement(Loops);
    std::string loopText = getSourceText(selectedLoop);

    addStringAfterStmt(selectedLoop, loopText);

    return true;
  }

private:
  std::vector<ForStmt*> Loops;  // Store the loops
};

static RegisterMutator<DuplicateLoop> M("u3.DuplicateLoop", "This mutator selects a loop in a function and duplicates it, effectively executing the loop twice in a row");
