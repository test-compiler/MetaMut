#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRandomExit : public Mutator,
                         public clang::RecursiveASTVisitor<InsertRandomExit> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (Stmts.empty()) return false;

    Stmt *stmt = randElement(Stmts);
    
    // Inserting 'exit();' only at end of a statement
    getRewriter().InsertText(getLocForEndOfToken(stmt->getEndLoc()), "exit();\n");
    return true;
  }

  bool VisitStmt(clang::Stmt *S) {
    Stmts.push_back(S);
    return true;
  }

private:
  std::vector<clang::Stmt *> Stmts;
};

static RegisterMutator<InsertRandomExit> M("u3.InsertRandomExit", "Inserts a random 'exit()' statement within a function.");
