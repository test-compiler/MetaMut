#ifndef YSMUT_FUNC_INLINE_INVERSEINLINEFUNCTION_H
#define YSMUT_FUNC_INLINE_INVERSEINLINEFUNCTION_H

#include "Mutator.h"
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

namespace ysmut {

class InlineFunctionReversal
    : public Mutator,
      public clang::RecursiveASTVisitor<InlineFunctionReversal> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFunctionDecl(clang::FunctionDecl *FD);

private:
  std::vector<clang::FunctionDecl *> TheFunctions;
};

} // namespace ysmut

#endif
