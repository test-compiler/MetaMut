// ToggleVarDeclInitialization.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ToggleVarDeclInitialization : public Mutator,
                                    public clang::RecursiveASTVisitor<ToggleVarDeclInitialization> {
public:
  using Mutator::Mutator;
  
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty()) return false;

    VarDecl *var = randElement(TheVars);
    
    if (var->hasInit()) {
      // Remove initialization
      getRewriter().ReplaceText(var->getSourceRange(), var->getNameAsString());
    } else {
      // Add default initialization
      std::string defaultInit = " = 0";
      getRewriter().InsertTextAfter(getLocForEndOfToken(var->getEndLoc()), defaultInit);
    }

    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    TheVars.push_back(VD);
    return true;
  }

private:
  std::vector<clang::VarDecl *> TheVars;
};

static RegisterMutator<ToggleVarDeclInitialization> M(
    "ToggleVarDeclInitialization", "Toggle variable declaration initialization.");