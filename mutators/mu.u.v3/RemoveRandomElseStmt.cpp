#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class RemoveRandomElseStmt : public Mutator,
                             public RecursiveASTVisitor<RemoveRandomElseStmt> {
public:
  using Mutator::Mutator;

  bool VisitIfStmt(IfStmt *IS) {
    if (IS->getElse()) {
      TheIfStmts.push_back(IS);
    }
    return true;
  }

  bool mutate() override {
    TraverseDecl(getASTContext().getTranslationUnitDecl());
    if (TheIfStmts.empty()) return false;

    IfStmt *SelectedIfStmt = randElement(TheIfStmts);
    Stmt *elseStmt = SelectedIfStmt->getElse();

    SourceRange elseRange = elseStmt->getSourceRange();
    getRewriter().ReplaceText(elseRange, "");

    return true;
  }

private:
  std::vector<IfStmt *> TheIfStmts;
};

static RegisterMutator<RemoveRandomElseStmt> M("u3.RemoveRandomElseStmt", 
    "This mutator identifies an 'if-else' statement and removes the 'else' branch, effectively changing the control flow of the program."
);
