#ifndef YSMUT_FUNC_CHANGEPARMDECLTYPE_H
#define YSMUT_FUNC_CHANGEPARMDECLTYPE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <llvm/ADT/DenseSet.h>

#include "Mutator.h"

namespace ysmut {

class ChangeParmType : public Mutator,
                       public clang::RecursiveASTVisitor<ChangeParmType> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitParmVarDecl(clang::ParmVarDecl *PVD);
  bool VisitDecl(clang::Decl *D);

private:
  std::vector<clang::ParmVarDecl *> TheParms;
  llvm::DenseSet<clang::QualType, llvm::DenseMapInfo<clang::QualType>> TheTypes;
};

} // namespace ysmut

#endif
