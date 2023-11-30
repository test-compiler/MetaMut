#ifndef YSMUT_EXPR_RANDOMIZESTRINGLITERAL_H
#define YSMUT_EXPR_RANDOMIZESTRINGLITERAL_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <string>

#include "Mutator.h"

namespace ysmut {

class RandomizeStringLiteral : public Mutator, public clang::RecursiveASTVisitor<RandomizeStringLiteral> {

public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitStringLiteral(clang::StringLiteral *SL);

private:
  std::vector<clang::StringLiteral *> TheStringLiterals;
  std::string mutateString(const std::string& oldString);
  std::string escape(const std::string &s);
};

} // namespace ysmut

#endif
