#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Sema/Sema.h>
#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ReduceVariableScope : public Mutator, 
                            public clang::RecursiveASTVisitor<ReduceVariableScope> {
public:
  using Mutator::Mutator;
  
  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->hasGlobalStorage()) {
      GlobalVars.push_back(VD);
    }
    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    Funcs.push_back(FD);
    return true;
  }

  bool mutate() override {
    // Step 1: Traverse the AST
    TraverseAST(getASTContext());

    // Step 2: Check if there are eligible global variables and functions
    if (GlobalVars.empty() || Funcs.empty()) return false;

    // Step 3: Randomly select a global variable and a function
    VarDecl *oldVar = randElement(GlobalVars);
    FunctionDecl *func = randElement(Funcs);

    // Step 4: Generate a unique name for the new local variable
    std::string newVarName = generateUniqueName(oldVar->getNameAsString());

    // Step 5: Create the declaration for the new local variable
    QualType QT = oldVar->getType();
    std::string VarDeclStr = formatAsDecl(QT, newVarName);

    // Check if the old variable has an initial value
    if (oldVar->hasInit()) {
      VarDeclStr += " = " + getSourceText(oldVar->getInit()) + ";";
    } else {
      VarDeclStr += ";";
    }

    // Step 6: Add the declaration for the new variable at the start of the function
    Stmt *body = func->getBody();
    getRewriter().ReplaceText(body->getSourceRange(), "{\n" + VarDeclStr + "\n" + getSourceText(body) + "\n}");

    // Step 7: Replace all uses of the old variable in the function with the new variable
    for (DeclRefExpr *ref : DeclRefs[oldVar]) {
      if (ref->getDecl() == oldVar && getASTContext().getSourceManager().isPointWithin(ref->getExprLoc(), func->getBody()->getBeginLoc(), func->getBody()->getEndLoc())) {
        getRewriter().ReplaceText(ref->getSourceRange(), newVarName);
      }
    }
    
    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *DRE) {
    if (VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
      DeclRefs[VD].push_back(DRE);
    }
    return true;
  }

private:
  std::vector<VarDecl*> GlobalVars;
  std::vector<FunctionDecl*> Funcs;
  std::map<VarDecl*, std::vector<DeclRefExpr*>> DeclRefs;
};

static RegisterMutator<ReduceVariableScope> M("u3.ReduceVariableScope", "Replace a global variable with a local one in a specific function");
