#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class MutateLoopBounds
    : public Mutator,
      public clang::RecursiveASTVisitor<MutateLoopBounds> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLoops.empty()) return false;

    Stmt *loop = randElement(TheLoops);

    ForStmt *forLoop = dyn_cast<ForStmt>(loop);
    WhileStmt *whileLoop = dyn_cast<WhileStmt>(loop);

    if (forLoop) {
      mutateForLoop(forLoop);
    } else if (whileLoop) {
      mutateWhileLoop(whileLoop);
    }

    return true;
  }

  bool VisitForStmt(clang::ForStmt *FS) {
    TheLoops.push_back(FS);
    return true;
  }

  bool VisitWhileStmt(clang::WhileStmt *WS) {
    TheLoops.push_back(WS);
    return true;
  }

private:
  std::vector<clang::Stmt *> TheLoops;

  void mutateForLoop(ForStmt *FS) {
    // Generate a random number between 0 and 2
    int choice = randIndex(3);
    
    if (choice == 0) {
      // Mutate initialization
      std::string newInit = generateUniqueName("init");
      getRewriter().ReplaceText(FS->getInit()->getSourceRange(), newInit);
    } else if (choice == 1) {
      // Mutate condition
      std::string newCond = generateUniqueName("cond");
      getRewriter().ReplaceText(FS->getCond()->getSourceRange(), newCond);
    } else {
      // Mutate increment
      std::string newInc = generateUniqueName("inc");
      getRewriter().ReplaceText(FS->getInc()->getSourceRange(), newInc);
    }
  }

  void mutateWhileLoop(WhileStmt *WS) {
    // Generate a random number between 0 and 1
    int choice = randIndex(2);
    
    if (choice == 0) {
      // Mutate condition
      std::string newCond = generateUniqueName("cond");
      getRewriter().ReplaceText(WS->getCond()->getSourceRange(), newCond);
    } else {
      // Mutate body
      std::string newBody = generateUniqueName("body");
      getRewriter().ReplaceText(WS->getBody()->getSourceRange(), newBody);
    }
  }
};

static RegisterMutator<MutateLoopBounds> M(
    "MutateLoopBounds", "Modify the loop control variable's initialization, condition, or increment/decrement statement.");