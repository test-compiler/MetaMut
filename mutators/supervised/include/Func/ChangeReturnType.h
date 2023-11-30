#ifndef YSMUT_FUNC_CHANGERETURNTYPE_H
#define YSMUT_FUNC_CHANGERETURNTYPE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <llvm/ADT/DenseSet.h>

#include "Mutator.h"

namespace ysmut {

class ChangeReturnType : public Mutator,
                         public clang::RecursiveASTVisitor<ChangeReturnType> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitDecl(clang::Decl *D);

private:
  std::vector<clang::FunctionDecl *> TheFuncs;
  llvm::DenseSet<clang::QualType, llvm::DenseMapInfo<clang::QualType>> TheTypes;
};

} // namespace ysmut

#endif
