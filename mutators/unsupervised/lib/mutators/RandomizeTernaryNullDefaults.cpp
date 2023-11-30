#include "Mutator.h"
#include "MutatorManager.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;
using namespace ysmut;

class RandomizeTernaryNullDefaults : public Mutator,
                      public clang::RecursiveASTVisitor<RandomizeTernaryNullDefaults> {
public:
  RandomizeTernaryNullDefaults(const char *name, const char *desc)
      : Mutator(name, desc) {}

  void setContextAndRewriter(clang::ASTContext* Context, clang::Rewriter& OurRewriter) {
    this->Context = Context;
    this->OurRewriter = &OurRewriter;
  }

  bool VisitConditionalOperator(ConditionalOperator *CO) {
    if (CO && CO->getLHS()->isNullPointerConstant(*Context, Expr::NPC_ValueDependentIsNotNull)) {
      TheDefaults.push_back(CO->getRHS());
    }
    return true;
  }

  bool mutate() override {
    // Check if the context and the translation unit declaration are valid
    if (Context && Context->getTranslationUnitDecl()) {
      // Traverse the AST and collect ConditionalOperators
      TraverseDecl(Context->getTranslationUnitDecl());
    } else {
      llvm::errs() << "Context or TranslationUnitDecl is NULL\n";
      return false;
    }

    // If there are no ConditionalOperators to mutate, return false
    if (TheDefaults.empty()) {
      llvm::errs() << "No ConditionalOperators to mutate\n";
      return false;
    }

    // Randomly select a ConditionalOperator to mutate
    Expr *target = randElement(TheDefaults);
    if (!target) {
      llvm::errs() << "No target selected for mutation\n";
      return false;
    }
    
    // Generate a random new value and replace the old value
    std::string newValue = generateNewValue(target->getType());
    OurRewriter->ReplaceText(target->getSourceRange(), newValue);

    return true;
  }

private:
  clang::ASTContext* Context;
  clang::Rewriter* OurRewriter;
  std::vector<Expr *> TheDefaults;

  Expr* randElement(std::vector<Expr *> &v){
    if (v.empty()) return nullptr;
    int randomIndex = std::rand() % v.size();
    return v[randomIndex];
  }

  std::string generateNewValue(QualType type) {
    if (type->isIntegerType()) {
      int randomNum = std::rand() % 100; 
      return std::to_string(randomNum);
    } else if (type->isAnyCharacterType()) {
      int randomChar = 'a' + (std::rand() % 26); 
      return "'" + std::string(1, static_cast<char>(randomChar)) + "'";
    }

    return "";
  }
};

static RegisterMutator<RandomizeTernaryNullDefaults> X(
  "RandomizeTernaryNullDefaults", 
  "Randomize the default values used in ternary expressions when null pointers are detected."
);