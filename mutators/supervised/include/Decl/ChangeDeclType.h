#ifndef YSMUT_DECL_CHANGEDECLTYPE_H
#define YSMUT_DECL_CHANGEDECLTYPE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <llvm/ADT/DenseSet.h>

#include "Mutator.h"

namespace ysmut {

class ChangeDeclType : public Mutator,
                       public clang::RecursiveASTVisitor<ChangeDeclType> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitDecl(clang::Decl *D);

private:
  std::vector<clang::VarDecl *> TheVars;
  llvm::DenseSet<clang::QualType, llvm::DenseMapInfo<clang::QualType>> TheTypes;
};

} // namespace ysmut

#endif
