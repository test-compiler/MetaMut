#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class ReplaceWithZero : public Mutator,
                        public clang::RecursiveASTVisitor<ReplaceWithZero> {
public:
  using Mutator::Mutator;

  bool VisitIntegerLiteral(IntegerLiteral *IL) {
    TheInts.push_back(IL);
    return true;
  }

  bool VisitFloatingLiteral(FloatingLiteral *FL) {
    TheFloats.push_back(FL);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheInts.empty() && TheFloats.empty()) return false;

    if (!TheInts.empty() && (TheFloats.empty() || randBool())) {
      IntegerLiteral *selectedLiteral = randElement(TheInts);
      getRewriter().ReplaceText(selectedLiteral->getSourceRange(), "0");
    } else {
      FloatingLiteral *selectedLiteral = randElement(TheFloats);
      getRewriter().ReplaceText(selectedLiteral->getSourceRange(), "0.0");
    }

    return true;
  }

private:
  std::vector<IntegerLiteral*> TheInts;
  std::vector<FloatingLiteral*> TheFloats;
};

static RegisterMutator<ReplaceWithZero> M("u3.ReplaceWithZero", "Replace a numeric literal with zero.");
