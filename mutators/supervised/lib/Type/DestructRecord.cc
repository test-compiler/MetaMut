#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"
#include "Type/DestructRecord.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<DestructRecord> M(
    "destruct-record", "Break struct/union variables into smaller variables.");

bool DestructRecord::TraverseFunctionDecl(clang::FunctionDecl *FD) {
  VisitorTy::TraverseFunctionDecl(FD);
  return true;
}

bool DestructRecord::VisitVarDecl(VarDecl *VD) {
  if (isMutationSite(VD) && VD->getType()->isRecordType()) {
    TheVars.push_back(VD);
  }
  return true;
}

bool DestructRecord::VisitDeclStmt(DeclStmt *DS) {
  for (auto it = DS->decl_begin(); it != DS->decl_end(); ++it) {
    declToStmt[*it] = DS;
  }
  return true;
}

bool DestructRecord::VisitMemberExpr(MemberExpr *ME) {
  if (isMutationSite(ME)) { TheMembers.push_back(ME); }
  return true;
}

bool DestructRecord::mutate() {
  TraverseAST(getASTContext());

  if (TheVars.empty() || TheMembers.empty()) { return false; }

  VarDecl *var = randElement(TheVars);

  // Create new variables for each field in the record
  RecordDecl *record = var->getType()->getAs<RecordType>()->getDecl();
  std::string varName = var->getNameAsString();
  std::map<std::string, std::string> fieldToVar;
  std::string declStr;
  for (auto field : record->fields()) {
    std::string newVarName =
        generateUniqueName(varName + "_" + field->getNameAsString());

    // Generate and insert new variable declaration
    declStr += formatAsDecl(field->getType(), newVarName) + " = " + varName +
               "." + field->getNameAsString() + ";\n";

    fieldToVar[field->getNameAsString()] = newVarName;
  }

  // Insert at the correct location
  clang::DeclStmt *DS = declToStmt[var];
  getRewriter().ReplaceText(
      DS->getSourceRange(), getSourceText(DS).str() + declStr);

  // Replace references to the record's fields with the corresponding smaller
  // variables
  for (auto ME : TheMembers) {
    if (DeclRefExpr *DRE =
            dyn_cast<DeclRefExpr>(ME->getBase()->IgnoreImpCasts())) {
      if (VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
        if (VD == var) {
          std::string fieldName = ME->getMemberDecl()->getNameAsString();
          getRewriter().ReplaceText(
              ME->getSourceRange(), fieldToVar[fieldName]);
        }
      }
    }
  }

  return true;
}
