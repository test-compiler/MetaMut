#ifndef YSMUT_DECL_CHANGEFIELDDECLTYPE_H
#define YSMUT_DECL_CHANGEFIELDDECLTYPE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <llvm/ADT/DenseSet.h>

#include "Mutator.h"

namespace ysmut {

class ChangeFieldDeclType : public Mutator,
                            public clang::RecursiveASTVisitor<ChangeFieldDeclType> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFieldDecl(clang::FieldDecl *FD);
  bool VisitDecl(clang::Decl *D);

private:
  std::vector<clang::FieldDecl *> TheFields;
  llvm::DenseSet<clang::QualType, llvm::DenseMapInfo<clang::QualType>> TheTypes;
};

} // namespace ysmut

#endif
