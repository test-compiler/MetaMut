// ReplaceRandomIntWithCharLiteral.cpp
#include <random>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ReplaceRandomIntWithCharLiteral : public Mutator,
                      public clang::RecursiveASTVisitor<ReplaceRandomIntWithCharLiteral> {

  using VisitorTy = clang::RecursiveASTVisitor<ReplaceRandomIntWithCharLiteral>;

public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheIntegers.empty()) return false;

    IntegerLiteral *intLiteral = randElement(TheIntegers);
    
    // Generate a character literal based on ASCII range 0-127
    std::uniform_int_distribution<int> distribution(0, 127);
    char newChar = static_cast<char>(distribution(getManager().getRandomGenerator()));
    std::string newCharLiteral = "'" + std::string(1, newChar) + "'";

    // Replace the integer literal with the character literal
    getRewriter().ReplaceText(intLiteral->getSourceRange(), newCharLiteral);

    return true;
  }

  bool VisitIntegerLiteral(clang::IntegerLiteral *IL) {
    TheIntegers.push_back(IL);
    return true;
  }

private:
  std::vector<clang::IntegerLiteral *> TheIntegers;
};

static RegisterMutator<ReplaceRandomIntWithCharLiteral> M(
    "ReplaceRandomIntWithCharLiteral", "Randomly replace an integer literal with a character literal from the ASCII range 0-127.");