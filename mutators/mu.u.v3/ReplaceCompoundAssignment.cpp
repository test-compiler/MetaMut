#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class ReplaceCompoundAssignment : public Mutator,
                                  public RecursiveASTVisitor<ReplaceCompoundAssignment> {
public:
  using Mutator::Mutator;

  bool VisitCompoundAssignOperator(clang::CompoundAssignOperator *CAO) {
    TheCompoundAssignments.push_back(CAO);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheCompoundAssignments.empty())
      return false;

    CompoundAssignOperator *selectedCAO = randElement(TheCompoundAssignments);
  
    std::string lhs = getSourceText(selectedCAO->getLHS());
    std::string rhs = getSourceText(selectedCAO->getRHS());

    // Calculate the corresponding non-compound binary operator
    BinaryOperatorKind nonCompoundOp = static_cast<BinaryOperatorKind>(selectedCAO->getOpcode() - BO_MulAssign + BO_Mul);
    std::string op = BinaryOperator::getOpcodeStr(nonCompoundOp).str();

    std::string replaceText = lhs + " = " + lhs + " " + op + " " + rhs;
    getRewriter().ReplaceText(selectedCAO->getSourceRange(), replaceText);

    return true;
  }

private:
  std::vector<clang::CompoundAssignOperator *> TheCompoundAssignments;

  std::string getSourceText(const clang::Expr *E) {
    return Lexer::getSourceText(
        CharSourceRange::getTokenRange(E->getSourceRange()), 
        getASTContext().getSourceManager(), 
        getCompilerInstance().getLangOpts()).str();
  }
};

static RegisterMutator<ReplaceCompoundAssignment> M("u3.ReplaceCompoundAssignment", 
    "This mutator identifies a compound assignment operation in the code (like +=, -=, *=, /=, %=, etc.) and replaces it with its non-compound version (like a += b becomes a = a + b)");
