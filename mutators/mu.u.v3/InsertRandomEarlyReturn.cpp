#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class InsertRandomEarlyReturn
    : public Mutator,
      public clang::RecursiveASTVisitor<InsertRandomEarlyReturn> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    // Check if function return type is void
    if (FD->getReturnType().getAsString() == "void") {
        TheStmts.push_back(FD->getBody());
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheStmts.empty()) return false;

    Stmt *stmt = randElement(TheStmts);

    // Perform rewrite
    auto it = stmt->child_begin();
    std::advance(it, randIndex(std::distance(stmt->child_begin(), stmt->child_end())));
    SourceLocation Loc = getLocForEndOfToken((*it)->getEndLoc());
    getRewriter().InsertTextAfter(Loc, "\nreturn;");

    return true;
  }

private:
  std::vector<clang::Stmt *> TheStmts;
};

static RegisterMutator<InsertRandomEarlyReturn> M("u3.InsertRandomEarlyReturn", "Insert a return statement at a random position within a function with void return type.");
