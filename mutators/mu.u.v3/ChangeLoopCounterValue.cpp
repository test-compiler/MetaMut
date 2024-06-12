#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/Stmt.h>

using namespace clang;

class ChangeLoopCounterValue : public Mutator,
                      public clang::RecursiveASTVisitor<ChangeLoopCounterValue> {

public:
  using Mutator::Mutator;
  
  // Visitor to identify loops and their control variables
  bool VisitForStmt(ForStmt *FS) {
    if(auto *D = dyn_cast_or_null<DeclStmt>(FS->getInit())) {
      if(auto *VD = dyn_cast_or_null<VarDecl>(D->getSingleDecl())) {
        LoopVars.push_back(VD);
      }
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    
    if(LoopVars.empty()) return false;
    
    VarDecl *VD = randElement(LoopVars);
    Expr *Init = VD->getInit();
    
    if(!Init || !Init->isIntegerConstantExpr(getASTContext())) return false;
    
    int NewInitVal = randBool() ? 1 : -1;
    std::string NewInitStr = std::to_string(NewInitVal);
    getRewriter().ReplaceText(Init->getSourceRange(), NewInitStr);
    
    return true;
  }

private:
  std::vector<VarDecl *> LoopVars;
};

static RegisterMutator<ChangeLoopCounterValue> M("u3.ChangeLoopCounterValue", "Change loop counter's initialization value");
