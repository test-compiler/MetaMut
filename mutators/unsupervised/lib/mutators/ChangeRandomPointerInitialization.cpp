// ChangeRandomPointerInitialization.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ChangeRandomPointerInitialization : public Mutator,
                      public clang::RecursiveASTVisitor<ChangeRandomPointerInitialization> {
  
  using VisitorTy = clang::RecursiveASTVisitor<ChangeRandomPointerInitialization>;

public:
  using Mutator::Mutator;
  
  bool mutate() override {
    TraverseAST(getASTContext());
    if (ThePointerDecls.empty() || ThePointerVars.empty()) return false;

    VarDecl *declToChange = randElement(ThePointerDecls);

    VarDecl *newVar = randElement(ThePointerVars);
    if (declToChange == newVar) return false;

    // Replace the old variable initialization with the new one
    getRewriter().ReplaceText(declToChange->getInit()->getSourceRange(), newVar->getNameAsString());

    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getType()->isPointerType()) {
      ThePointerDecls.push_back(VD);
      if (VD->hasInit()) { 
        ThePointerVars.push_back(VD); 
      }
    }
    return true;
  }

private:
  std::vector<clang::VarDecl *> ThePointerDecls;
  std::vector<clang::VarDecl *> ThePointerVars;
};

static RegisterMutator<ChangeRandomPointerInitialization> X(
    "ChangeRandomPointerInitialization", 
    "Randomly selects a PointerType variable initialization and changes its initialized value to another compatible pointer variable or null.");