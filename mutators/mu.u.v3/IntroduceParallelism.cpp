#include "Mutator.h"
#include "MutatorManager.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"

using namespace clang;

class IntroduceParallelism
    : public Mutator,
      public RecursiveASTVisitor<IntroduceParallelism> {
public:
  using Mutator::Mutator;

  bool VisitForStmt(clang::ForStmt *For) {
    LoopStack.push_back(For);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (LoopStack.empty())
      return false;

    ForStmt *TargetFor = randElement(LoopStack);

    std::string pragmaString = "#pragma omp parallel for\n";
    getRewriter().InsertTextBefore(TargetFor->getBeginLoc(), pragmaString);

    return true;
  }

private:
  std::vector<clang::ForStmt *> LoopStack;
};

static RegisterMutator<IntroduceParallelism> M("u3.IntroduceParallelism", 
    "Identifies 'for' loops and transforms them to equivalent parallel 'for' loops using OpenMP");
