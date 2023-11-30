#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Decl/AddInitialization.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<AddInitialization> M("add-initialization",
    "Randomly add initialization to an uninitialized VarDecl.");

bool AddInitialization::TraverseFunctionDecl(clang::FunctionDecl *FD) {
  return VisitorTy::TraverseStmt(FD->getBody()); // skip parm
}

bool AddInitialization::VisitVarDecl(VarDecl *VD) {
  if (isMutationSite(VD) && !VD->hasInit()) TheVars.push_back(VD);
  return true;
}

bool AddInitialization::mutate() {
  TraverseAST(getASTContext());
  if (TheVars.empty()) return false;

  VarDecl *selectedVD = randElement(TheVars);
  int initValue = 0;

  // Get the end location of the VarDecl name
  SourceLocation varEndLoc = getLocForEndOfToken(selectedVD->getLocation());
  TypeSourceInfo *TSI = selectedVD->getTypeSourceInfo();

  if (TSI) varEndLoc = getLocForEndOfToken(TSI->getTypeLoc().getEndLoc());

  // Insert the initialization after the VarDecl name
  getRewriter().InsertTextAfter(
      varEndLoc, " = {" + std::to_string(initValue) + "}");

  return true;
}
