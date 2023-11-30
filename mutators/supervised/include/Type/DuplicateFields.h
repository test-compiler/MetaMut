#ifndef YSMUT_TYPE_DUPLICATEFIELDS_H
#define YSMUT_TYPE_DUPLICATEFIELDS_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class DuplicateFields : public Mutator,
                        public clang::RecursiveASTVisitor<DuplicateFields> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFieldDecl(clang::FieldDecl *FD);

private:
  std::vector<clang::FieldDecl *> TheFields;
};

} // namespace ysmut

#endif
