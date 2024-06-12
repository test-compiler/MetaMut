// InsertRedundantScope.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRedundantScope : public Mutator,
                             public RecursiveASTVisitor<InsertRedundantScope> {
public:
  using Mutator::Mutator;

  bool VisitCompoundStmt(CompoundStmt *CS) {
    TheCompoundStmts.push_back(CS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheCompoundStmts.empty())
      return false;

    CompoundStmt *selectedCS = randElement(TheCompoundStmts);
    std::string newScope = "{ " + getSourceText(selectedCS) + " }";

    getRewriter().ReplaceText(selectedCS->getSourceRange(), newScope);

    return true;
  }

private:
  std::vector<CompoundStmt *> TheCompoundStmts;
};

static RegisterMutator<InsertRedundantScope> M("u3.InsertRedundantScope",
    "Wraps a block of statements in the code with a new scope");
