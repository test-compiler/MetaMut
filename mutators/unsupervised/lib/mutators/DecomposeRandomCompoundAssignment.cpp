#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class DecomposeRandomCompoundAssignment
    : public Mutator,
      public clang::RecursiveASTVisitor<DecomposeRandomCompoundAssignment> {
  
public:
  using Mutator::Mutator;
  
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheOperators.empty()) return false;

    CompoundAssignOperator* op = randElement(TheOperators);
    std::string lhs = getRewriter().getRewrittenText(op->getLHS()->getSourceRange());
    std::string rhs = getRewriter().getRewrittenText(op->getRHS()->getSourceRange());
    std::string newExpr = lhs + " = " + lhs + " " + BinaryOperator::getOpcodeStr(op->getOpcode()).str().substr(0, 1) + " " + rhs + ";";
    
    getRewriter().ReplaceText(op->getSourceRange(), newExpr);

    return true;
  }

  bool VisitCompoundAssignOperator(CompoundAssignOperator* CAO) {
    TheOperators.push_back(CAO);
    return true;
  }

private:
  std::vector<CompoundAssignOperator*> TheOperators;
};

static RegisterMutator<DecomposeRandomCompoundAssignment> M(
    "DecomposeRandomCompoundAssignment", 
    "Decompose a CompoundAssignOperator into separate binary operation and assignment.");