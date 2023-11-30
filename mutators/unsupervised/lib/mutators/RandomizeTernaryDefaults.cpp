#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class RandomizeTernaryDefaults : public Mutator,
                      public clang::RecursiveASTVisitor<RandomizeTernaryDefaults> {

  using VisitorTy = clang::RecursiveASTVisitor<RandomizeTernaryDefaults>;

public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheDefaults.empty()) return false;

    auto *def = randElement(TheDefaults);

    std::string newDefaultValue = generateNewValue(def->getType());
    if (newDefaultValue.empty()) return false;

    getRewriter().ReplaceText(def->getSourceRange(), newDefaultValue);

    return true;
  }

  bool VisitConditionalOperator(ConditionalOperator *CO) {
    if (isa<NullStmt>(CO->getLHS())) {
      TheDefaults.push_back(CO->getRHS());
    }
    return true;
  }

private:
  std::vector<clang::Expr *> TheDefaults;

  std::string generateNewValue(QualType type) {
    if (type->isIntegerType()) {
      int randomNum = getManager().getRandomGenerator()();
      return std::to_string(randomNum % 100);
    } else if (type->isCharType()) {
      return "'a'";
    }

    return "";
  }
};

static RegisterMutator<RandomizeTernaryDefaults> M(
    "RandomizeTernaryDefaults", "Randomize the default values used in ternary expressions.");