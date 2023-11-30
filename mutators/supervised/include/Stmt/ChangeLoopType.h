#ifndef YSMUT_STMT_CHANGELOOPTYPE_H
#define YSMUT_STMT_CHANGELOOPTYPE_H

#include <clang/AST/RecursiveASTVisitor.h>
#include "Mutator.h"

namespace ysmut {

class ChangeLoopType : public Mutator,
                       public clang::RecursiveASTVisitor<ChangeLoopType> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitForStmt(clang::ForStmt *FS);
  bool VisitWhileStmt(clang::WhileStmt *WS);

private:
  std::vector<clang::ForStmt *> TheForLoops;
  std::vector<clang::WhileStmt *> TheWhileLoops;
};

} // namespace ysmut

#endif
