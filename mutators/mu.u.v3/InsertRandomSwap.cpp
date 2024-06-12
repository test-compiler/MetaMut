#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class InsertRandomSwap : public Mutator,
                         public clang::RecursiveASTVisitor<InsertRandomSwap> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD->hasBody()) {
      TheFunctions.push_back(FD);
    }
    return true;
  }

  bool VisitVarDecl(VarDecl *VD) {
    if (VD->isLocalVarDecl()) {
      auto FD = dyn_cast_or_null<FunctionDecl>(VD->getDeclContext());
      if (FD) {
        FunctionVariablesMap[FD].push_back(VD);
      }
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheFunctions.empty()) return false;

    FunctionDecl *SelectedFunc = randElement(TheFunctions);
    auto &Vars = FunctionVariablesMap[SelectedFunc];

    if (Vars.size() < 2) return false;

    VarDecl *Var1 = nullptr;
    VarDecl *Var2 = nullptr;

    for(auto &Var : Vars){
        if(!Var->getType()->isFunctionPointerType()){
            if(!Var1){
                Var1 = Var;
            }else if(Var1 != Var){
                Var2 = Var;
                break;
            }
        }
    }
    
    if (!Var1 || !Var2) return false;

    std::string swapCode = "{0} tmp = {1}; {1} = {2}; {2} = tmp;";
    swapCode.replace(swapCode.find("{0}"), 3, Var1->getType().getAsString());
    swapCode.replace(swapCode.find("{1}"), 3, Var1->getNameAsString());
    while(swapCode.find("{1}") != std::string::npos){
        swapCode.replace(swapCode.find("{1}"), 3, Var1->getNameAsString());
    }
    while(swapCode.find("{2}") != std::string::npos){
        swapCode.replace(swapCode.find("{2}"), 3, Var2->getNameAsString());
    }

    // Insert the swap code at the end of the function body
    CompoundStmt* compoundStmt = dyn_cast_or_null<CompoundStmt>(SelectedFunc->getBody());
    if (!compoundStmt) return false;

    Stmt* LastStmt = *(std::prev(compoundStmt->body_end()));
    addStringAfterStmt(LastStmt, "\n" + swapCode);

    return true;
  }

private:
  std::vector<FunctionDecl *> TheFunctions;
  std::map<FunctionDecl *, std::vector<VarDecl *>> FunctionVariablesMap;
};

static RegisterMutator<InsertRandomSwap> M("u3.InsertRandomSwap", "This mutator selects a function with at least two variable declarations and inserts code to swap the values of two randomly selected variables, effectively altering the program's control flow and state.");
