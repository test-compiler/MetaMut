#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class AddConditionToLoop : public Mutator,
                           public clang::RecursiveASTVisitor<AddConditionToLoop> {
public:
  using Mutator::Mutator;
  
  bool mutate() override {
    // Step 1, Traverse the AST
    TraverseAST(getASTContext());

    // Step 3, Randomly select a mutation instance
    if (TheLoops.empty()) 
      return false;

    Stmt *SelectedLoop = randElement(TheLoops);
    
    // Step 4, Checking mutation validity
    // We assume all loops collected are valid for mutation
    // Step 5, Perform mutation
    std::string newCond = " && rand() % 2 == 0"; // This is the additional condition we add
    SourceLocation loc = getLoopConditionEnd(SelectedLoop);
    getRewriter().InsertTextAfter(loc, newCond);
    
    // Step 6, Return true if changed
    return true;
  }

  bool VisitForStmt(ForStmt *FS) {
    TheLoops.push_back(FS);
    return true;
  }

  bool VisitWhileStmt(WhileStmt *WS) {
    TheLoops.push_back(WS);
    return true;
  }

  bool VisitDoStmt(DoStmt *DS) {
    TheLoops.push_back(DS);
    return true;
  }

private:
  std::vector<Stmt *> TheLoops;

  SourceLocation getLoopConditionEnd(Stmt *Loop) {
    if (auto *For = dyn_cast<ForStmt>(Loop)) 
      return getLocForEndOfToken(For->getCond()->getEndLoc());
    else if (auto *While = dyn_cast<WhileStmt>(Loop))
      return getLocForEndOfToken(While->getCond()->getEndLoc());
    else if (auto *Do = dyn_cast<DoStmt>(Loop))
      return getLocForEndOfToken(Do->getCond()->getEndLoc());
    else
      return SourceLocation(); // This should never happen
  }
};

static RegisterMutator<AddConditionToLoop> M("u3.AddConditionToLoop", "This mutator identifies a loop (for, while or do-while) in the code and adds an additional condition to it, effectively altering the loop's control flow");
