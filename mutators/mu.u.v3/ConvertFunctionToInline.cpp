#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <vector>

using namespace clang;

class ConvertFunctionToInline 
    : public Mutator, 
      public clang::RecursiveASTVisitor<ConvertFunctionToInline> {

public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseDecl(getASTContext().getTranslationUnitDecl());
    if (Functions.empty()) return false;

    FunctionDecl *func = randElement(Functions);

    SourceLocation locStart = func->getBeginLoc();
    getRewriter().InsertTextBefore(locStart, "inline ");
    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (!FD->isMain()) {
      Functions.push_back(FD);
    }
    return true;
  }

private:
  std::vector<clang::FunctionDecl *> Functions;
};

static RegisterMutator<ConvertFunctionToInline> M("u3.ConvertFunctionToInline", 
    "This mutator identifies a function in the code and adds the 'inline' keyword to its declaration, effectively suggesting the compiler to replace function calls with the function's body."
);
