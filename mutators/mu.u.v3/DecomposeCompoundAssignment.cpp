// DecomposeCompoundAssignment.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class DecomposeCompoundAssignment : public Mutator,
                      public RecursiveASTVisitor<DecomposeCompoundAssignment> {
public:
  using Mutator::Mutator;

  bool VisitCompoundAssignOperator(CompoundAssignOperator *CAO) {
    TheCompoundAssignOps.push_back(CAO);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheCompoundAssignOps.empty())
      return false;

    CompoundAssignOperator *selectedCAO = randElement(TheCompoundAssignOps);
    std::string decomposedAssignment = getLHSExpression(selectedCAO) + " = " + getLHSExpression(selectedCAO) + " " + getSimpleOperator(selectedCAO) + " " + getRHSExpression(selectedCAO);

    getRewriter().ReplaceText(selectedCAO->getSourceRange(), decomposedAssignment);

    return true;
  }

private:
  std::vector<CompoundAssignOperator *> TheCompoundAssignOps;

  std::string getLHSExpression(const CompoundAssignOperator *CAO) {
    return Lexer::getSourceText(CharSourceRange::getTokenRange(CAO->getLHS()->getSourceRange()), 
    getASTContext().getSourceManager(), getCompilerInstance().getLangOpts()).str();
  }

  std::string getRHSExpression(const CompoundAssignOperator *CAO) {
    return Lexer::getSourceText(CharSourceRange::getTokenRange(CAO->getRHS()->getSourceRange()), 
    getASTContext().getSourceManager(), getCompilerInstance().getLangOpts()).str();
  }

  std::string getSimpleOperator(const CompoundAssignOperator *CAO) {
    switch (CAO->getOpcode()) {
    case BO_MulAssign:
      return "*";
    case BO_DivAssign:
      return "/";
    case BO_RemAssign:
      return "%";
    case BO_AddAssign:
      return "+";
    case BO_SubAssign:
      return "-";
    case BO_ShlAssign:
      return "<<";
    case BO_ShrAssign:
      return ">>";
    case BO_AndAssign:
      return "&";
    case BO_XorAssign:
      return "^";
    case BO_OrAssign:
      return "|";
    default:
      return "unknown_op";
    }
  }
};

static RegisterMutator<DecomposeCompoundAssignment> M("u3.DecomposeCompoundAssignment",
    "Change compound assignment to simple assignment");
