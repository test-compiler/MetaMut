#ifndef YSMUT_STMT_MODIFYFORLOOPINCREMENT_H
#define YSMUT_STMT_MODIFYFORLOOPINCREMENT_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <random>

#include "Mutator.h"

namespace ysmut {

class ModifyForLoopIncrement : public Mutator,
                               public clang::RecursiveASTVisitor<ModifyForLoopIncrement> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitForStmt(clang::ForStmt *FS);

private:
  std::vector<clang::ForStmt *> TheLoops;
};

} // namespace ysmut

#endif
