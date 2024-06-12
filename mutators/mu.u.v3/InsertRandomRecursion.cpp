// InsertRandomRecursion.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

// this is mutator
class InsertRandomRecursion : public Mutator,
                      public clang::RecursiveASTVisitor<InsertRandomRecursion> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    if (FD->isThisDeclarationADefinition() && FD->hasBody())
        TheFuncs.push_back(FD);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFuncs.empty()) return false;

    clang::FunctionDecl *FD = randElement(TheFuncs);

    // Perform mutation
    auto newFuncCall = FD->getNameInfo().getAsString() + "();\n";
    addStringBeforeFunctionDecl(FD, newFuncCall);

    return true;
  }

private:
  std::vector<clang::FunctionDecl *> TheFuncs;
};

static RegisterMutator<InsertRandomRecursion> M("u3.InsertRandomRecursion", "Inserts a recursion call statement at a random position within the function body.");
