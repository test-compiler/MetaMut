// InsertRandomUnaryOperator.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class InsertRandomUnaryOperator
    : public Mutator,
      public clang::RecursiveASTVisitor<InsertRandomUnaryOperator> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLiterals.empty()) return false;

    Expr *literal = randElement(TheLiterals);

    // perform rewrite
    std::string newOp = getRandomValidUnaryOperator();
    getRewriter().InsertTextBefore(
        literal->getBeginLoc(), newOp + " ");

    return true;
  }

  bool VisitIntegerLiteral(clang::IntegerLiteral *IL) {
    TheLiterals.push_back(IL);
    return true;
  }

  bool VisitFloatingLiteral(clang::FloatingLiteral *FL) {
    TheLiterals.push_back(FL);
    return true;
  }

private:
  std::vector<clang::Expr *> TheLiterals;

  std::string getRandomValidUnaryOperator() {
    std::vector<std::string> validOperators = {
      "-", "+", "!", "~"
    };

    return validOperators[rand() % validOperators.size()];
  }
};

static RegisterMutator<InsertRandomUnaryOperator> M(
    "InsertRandomUnaryOperator", "Insert a unary operator before a literal.");