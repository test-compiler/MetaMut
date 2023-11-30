#ifndef YSMUT_TYPE_REMOVEUNUSEDFIELDS_H
#define YSMUT_TYPE_REMOVEUNUSEDFIELDS_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class RemoveUnusedFields
    : public Mutator,
      public clang::RecursiveASTVisitor<RemoveUnusedFields> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFieldDecl(clang::FieldDecl *FD);
  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);

private:
  std::vector<clang::FieldDecl *> UnusedFields;
  std::set<const clang::FieldDecl *> UsedFields;
};

} // namespace ysmut

#endif
