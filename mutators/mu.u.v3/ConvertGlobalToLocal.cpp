#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ConvertGlobalToLocal
    : public Mutator,
      public clang::RecursiveASTVisitor<ConvertGlobalToLocal> {

public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    LastFunction = FD;
    return true;
  }

  bool VisitVarDecl(VarDecl *VD) {
    if(VD->hasGlobalStorage() && !VD->isStaticLocal()) {
      GlobalVars.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (GlobalVars.empty() || LastFunction == nullptr) return false;

    // Randomly select a global variable
    VarDecl *VD = randElement(GlobalVars);
    
    // Format the new local declaration
    std::string NewVarDecl = formatAsDecl(VD->getType(), VD->getNameAsString()) + ";";
  
    // Add new local variable at the beginning of the function
    bool added = addStringBeforeFunctionDecl(LastFunction, NewVarDecl);
  
    return added;
  }
  
private:
  std::vector<VarDecl*> GlobalVars;
  FunctionDecl* LastFunction = nullptr;
};

static RegisterMutator<ConvertGlobalToLocal> M("u3.ConvertGlobalToLocal", "Converts a global variable to a local variable in a function");
