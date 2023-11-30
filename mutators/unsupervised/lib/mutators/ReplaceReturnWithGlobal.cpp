// ReplaceReturnWithGlobal.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ReplaceReturnWithGlobal
    : public Mutator,
      public clang::RecursiveASTVisitor<ReplaceReturnWithGlobal> {

public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheReturns.empty() || TheGlobals.empty()) return false;

    ReturnStmt *returnStmt = randElement(TheReturns);
    VarDecl *globalVar = randElement(TheGlobals);

    // Replace the return expression with the global variable
    getRewriter().ReplaceText(
        returnStmt->getRetValue()->getSourceRange(), globalVar->getNameAsString());

    return true;
  }

  bool VisitReturnStmt(ReturnStmt *RS) {
    TheReturns.push_back(RS);
    return true;
  }

  bool VisitVarDecl(VarDecl *VD) {
    if (VD->isFileVarDecl() && !VD->isStaticDataMember()) {
      TheGlobals.push_back(VD);
    }
    return true;
  }

private:
  std::vector<ReturnStmt *> TheReturns;
  std::vector<VarDecl *> TheGlobals;
};

static RegisterMutator<ReplaceReturnWithGlobal> M(
    "ReplaceReturnWithGlobal", "Replace a function's return expression with a global variable.");