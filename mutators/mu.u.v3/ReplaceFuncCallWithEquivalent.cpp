#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Type.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReplaceFuncCallWithEquivalent : public Mutator,
                      public clang::RecursiveASTVisitor<ReplaceFuncCallWithEquivalent> {

  using VisitorTy = clang::RecursiveASTVisitor<ReplaceFuncCallWithEquivalent>;

public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheCalls.empty() || TheFuncs.empty()) return false;

    CallExpr* callExpr = randElement(TheCalls);
    FunctionDecl* oldFunc = callExpr->getDirectCallee();

    // Removing the line of code that checks for functions with the same name as oldFunc
    // auto it = TheFuncs.find(oldFunc->getNameAsString());
    // if (it == TheFuncs.end()) return false; 

    // Selecting a random function from all the functions collected instead of just equivalent ones
    FunctionDecl* newFunc = randElement(TheFuncs);
    if (oldFunc == newFunc) return false; 

    getRewriter().ReplaceText(callExpr->getCallee()->getSourceRange(), newFunc->getNameAsString());
    
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *CE) {
    if (CE->getDirectCallee()) {
      TheCalls.push_back(CE);
    }
    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (FD->hasBody()) {
      TheFuncs.push_back(FD);
    }
    return true;
  }

private:
  std::vector<clang::CallExpr *> TheCalls;
  std::vector<clang::FunctionDecl*> TheFuncs;  // Changed from a map to a vector
};

static RegisterMutator<ReplaceFuncCallWithEquivalent> M("u3.ReplaceFuncCallWithEquivalent", "Replaces a function call with a call to another function.");
