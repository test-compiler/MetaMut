#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRandomDelay
    : public Mutator,
      public clang::RecursiveASTVisitor<InsertRandomDelay> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseDecl(getASTContext().getTranslationUnitDecl());
    if (TheFunctions.empty()) return false;

    FunctionDecl *func = randElement(TheFunctions);
    
    // Create the delay string
    std::string delayStr = "{\n#include <unistd.h>\nsleep(1);\n";
    
    // Insert the sleep at the beginning of the function body
    getRewriter().ReplaceText(func->getBody()->getSourceRange(), delayStr + getSourceText(func->getBody()) + "}");

    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (FD->hasBody())
      TheFunctions.push_back(FD);
    return true;
  }

private:
  std::vector<clang::FunctionDecl *> TheFunctions;
};

static RegisterMutator<InsertRandomDelay> M("u3.InsertRandomDelay", "Inserts a random sleep delay at the beginning of a function body");
