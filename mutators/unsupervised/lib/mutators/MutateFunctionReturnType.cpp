// MutateFunctionReturnType.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class MutateFunctionReturnType : public Mutator,
                                 public clang::RecursiveASTVisitor<MutateFunctionReturnType> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctions.empty()) return false;

    FunctionDecl *func = randElement(TheFunctions);
    if (func->getReturnType().getAsString() == "void") return false;

    // Replace the return type with double
    SourceRange returnTypeRange = getExpansionRange(func->getReturnTypeSourceRange());
    getRewriter().ReplaceText(returnTypeRange, "double");

    // Modify return statements
    for (auto retStmt : TheReturnStmts[func]) {
      Expr *retExpr = retStmt->getRetValue();
      std::string newExpr = "(double)(" + getSourceText(retExpr).str() + ")";
      getRewriter().ReplaceText(retExpr->getSourceRange(), newExpr);
    }

    return true;
  }

  bool TraverseFunctionDecl(FunctionDecl *FD) {
    if (FD->hasBody() && !FD->getReturnType().getAsString().empty()) {
      CurrentFunction = FD;
      TheFunctions.push_back(FD);
    }
    return RecursiveASTVisitor<MutateFunctionReturnType>::TraverseFunctionDecl(FD);
  }

  bool VisitReturnStmt(ReturnStmt *RS) {
    if (CurrentFunction) {
      TheReturnStmts[CurrentFunction].push_back(RS);
    }
    return true;
  }

private:
  std::vector<FunctionDecl *> TheFunctions;
  std::map<FunctionDecl*, std::vector<ReturnStmt*>> TheReturnStmts;
  FunctionDecl* CurrentFunction = nullptr;
};

static RegisterMutator<MutateFunctionReturnType> M(
    "MutateFunctionReturnType", "Change return type of a non-void function to double and modify return statements.");