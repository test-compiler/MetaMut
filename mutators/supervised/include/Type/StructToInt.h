#ifndef YSMUT_TYPE_STRUCTTOINT_H
#define YSMUT_TYPE_STRUCTTOINT_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/TypeLoc.h>

#include "Mutator.h"

namespace ysmut {

class StructToInt : public Mutator,
                    public clang::RecursiveASTVisitor<StructToInt> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitElaboratedTypeLoc(clang::ElaboratedTypeLoc ETL);

private:
  std::vector<const clang::TagDecl *> StructDecls;
  std::map<const clang::TagDecl *, std::vector<clang::TypeLoc>> StructsMap;
};

} // namespace ysmut

#endif
