#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include "MutatorManager.h"
#include "Type/DuplicateFields.h"

using namespace clang;
using namespace ysmut;

static RegisterMutator<DuplicateFields> M(
    "duplicate-fields", "Duplicate a field of a struct or union type.");

bool DuplicateFields::VisitFieldDecl(FieldDecl *FD) {
  if (isMutationSite(FD)) TheFields.push_back(FD);
  return true;
}

bool DuplicateFields::mutate() {
  TraverseAST(getASTContext());
  if (TheFields.empty()) return false;

  FieldDecl *field = randElement(TheFields);
  RecordDecl *parent = field->getParent();

  // Generate a unique name for the new field
  std::string newName = generateUniqueName(field->getName().str());

  // Prepare the declaration of the new field
  std::string newFieldDecl = formatAsDecl(field->getType(), newName);

  // Insert the new field declaration after the original field
  std::string fieldStr = getSourceText(field).str();
  getRewriter().ReplaceText(
      field->getSourceRange(), fieldStr + ";\n" + newFieldDecl);
  return true;
}
