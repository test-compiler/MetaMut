#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class NegateLoopCondition : public Mutator,
                            public clang::RecursiveASTVisitor<NegateLoopCondition> {

public:
  using Mutator::Mutator;

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

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLoops.empty()) return false;

    Stmt *selectedLoop = randElement(TheLoops);

    if (ForStmt *FS = dyn_cast<ForStmt>(selectedLoop)) {
        Expr *condition = FS->getCond();
        std::string newCond = "!(" + getSourceText(condition) + ")";
        getRewriter().ReplaceText(condition->getSourceRange(), newCond);
    }
    else if (WhileStmt *WS = dyn_cast<WhileStmt>(selectedLoop)) {
        Expr *condition = WS->getCond();
        std::string newCond = "!(" + getSourceText(condition) + ")";
        getRewriter().ReplaceText(condition->getSourceRange(), newCond);
    }
    else if (DoStmt *DS = dyn_cast<DoStmt>(selectedLoop)) {
        Expr *condition = DS->getCond();
        std::string newCond = "!(" + getSourceText(condition) + ")";
        getRewriter().ReplaceText(condition->getSourceRange(), newCond);
    }

    return true;
  }

private:
  std::vector<Stmt *> TheLoops;
};

static RegisterMutator<NegateLoopCondition> M("u3.NegateLoopCondition", 
    "This mutator selects a loop (for, while or do-while) and negates its condition, effectively inverting the loop's execution path.");
