#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Expr/ChangeDeclRef.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<ChangeDeclRef> M(
    "change-declref", "Change a DeclRefExpr's referenced variable.");

bool ChangeDeclRef::TraverseFunctionDecl(clang::FunctionDecl *FD) {
  TheVars.push_back(TheVars.back());
  VisitorTy::TraverseFunctionDecl(FD);
  TheVars.pop_back();
  return true;
}

bool ChangeDeclRef::TraverseCompoundStmt(CompoundStmt *CS) {
  if (!TheVars.empty()) {
    // Push a copy of the current set of variables
    TheVars.push_back(TheVars.back());
  } else {
    // Create the first set of variables
    TheVars.push_back({});
  }

  VisitorTy::TraverseCompoundStmt(CS);
  TheVars.pop_back();
  return true;
}

bool ChangeDeclRef::VisitDeclRefExpr(DeclRefExpr *DRE) {
  if (isMutationSite(DRE)) {
    TheRefs.push_back(DRE);
    ExprVarsMap[DRE] = TheVars.back();
  }
  return true;
}

bool ChangeDeclRef::VisitVarDecl(VarDecl *VD) {
  if (!TheVars.empty()) { TheVars.back().push_back(VD); }
  return true;
}

bool ChangeDeclRef::mutate() {
  TheVars.push_back({});
  TraverseAST(getASTContext());
  if (TheRefs.empty() || TheVars.empty()) return false;

  DeclRefExpr *ref = randElement(TheRefs);
  if (!isa<VarDecl>(ref->getDecl()))
    return false; // Ensure the Decl is actually a VarDecl

  VarDecl *oldVar = cast<VarDecl>(ref->getDecl());
  if (ExprVarsMap[ref].size() == 0) return false;

  VarDecl *newVar = randElement(ExprVarsMap[ref]);
  if (oldVar == newVar) return false;

  // Replace the old variable with the new one
  getRewriter().ReplaceText(ref->getSourceRange(), newVar->getNameAsString());

  return true;
}
