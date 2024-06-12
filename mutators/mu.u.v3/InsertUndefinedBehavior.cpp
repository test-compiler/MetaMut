#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertUndefinedBehavior 
    : public Mutator,
      public clang::RecursiveASTVisitor<InsertUndefinedBehavior> {
public:
  using Mutator::Mutator;
  
  bool VisitCompoundStmt(CompoundStmt *S) {
    for (Stmt *child : S->body()) {
      TheStmts.push_back(child);
    }
    return true;
  }

  bool mutate() override {
    // Traverse the AST
    TraverseAST(getASTContext());
    if (TheStmts.empty()) return false;

    // Randomly select a statement
    Stmt *selectedStmt = randElement(TheStmts);
    SourceLocation startLoc = selectedStmt->getBeginLoc();

    // Insert an operation causing undefined behavior
    bool shouldDivideByZero = randBool();
    if (shouldDivideByZero) {
      // Insert a division by zero operation
      getRewriter().InsertTextBefore(startLoc, "int zero = 0; int undefined = 1 / zero;\n");
    } else {
      // Insert a null pointer dereference operation
      getRewriter().InsertTextBefore(startLoc, "int *nullPtr = 0; int undefined = *nullPtr;\n");
    }

    return true;
  }

private:
  std::vector<Stmt *> TheStmts;
};

static RegisterMutator<InsertUndefinedBehavior> M("u3.InsertUndefinedBehavior", "Inserts an operation causing undefined behavior before a statement.");
