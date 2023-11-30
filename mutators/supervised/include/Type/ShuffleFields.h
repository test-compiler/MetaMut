#ifndef YSMUT_TYPE_SHUFFLEFIELDS_H
#define YSMUT_TYPE_SHUFFLEFIELDS_H

#include "Mutator.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <vector>

namespace ysmut {

class ShuffleFields : public Mutator,
                      public clang::RecursiveASTVisitor<ShuffleFields> {
  using VisitorTy = clang::RecursiveASTVisitor<ShuffleFields>;

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitRecordDecl(clang::RecordDecl *D);

private:
  std::vector<clang::RecordDecl *> Records;
};

} // namespace ysmut

#endif
