#ifndef YSMUT_STMT_TOGGLEBREAKCONTINUE_H
#define YSMUT_STMT_TOGGLEBREAKCONTINUE_H

#include "Mutator.h"
#include <clang/AST/RecursiveASTVisitor.h>

namespace ysmut {

class ToggleBreakContinue
    : public Mutator,
      public clang::RecursiveASTVisitor<ToggleBreakContinue> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitBreakStmt(clang::BreakStmt *BS);
  bool VisitContinueStmt(clang::ContinueStmt *CS);

private:
  std::vector<clang::Stmt *> TheStatements;
};

} // namespace ysmut

#endif
