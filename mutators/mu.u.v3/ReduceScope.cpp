#include "Mutator.h"
#include "MutatorManager.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;

class ReduceScope : public Mutator,
                    public RecursiveASTVisitor<ReduceScope> {
public:
  using Mutator::Mutator;

  bool VisitDeclRefExpr(DeclRefExpr *DRE) {
    if(auto VD = dyn_cast<VarDecl>(DRE->getDecl())) {
      if(VD->hasGlobalStorage() && !VD->getType()->isFunctionPointerType()) {
        TheRefs.push_back(DRE);
      }
    } else if (dyn_cast<FunctionDecl>(DRE->getDecl())) {
      TheRefs.push_back(DRE);
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if(FD->hasBody())
      FunctionStmts[FD] = collectStmts(FD->getBody());
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if(TheRefs.empty())
      return false;

    auto target = randElement(TheRefs);
    ValueDecl *oldVar = cast<ValueDecl>(target->getDecl());

    FunctionDecl *FD = getParentFunction(target);
    if(!FD)
      return false;

    std::string newVarName = generateUniqueName(oldVar->getNameAsString());
    std::string initExpr;
    if(auto VD = dyn_cast<VarDecl>(oldVar)){
        initExpr = VD->hasInit() ? " = " + getSourceText(VD->getInit()) : "";
    }
    std::string newDecl = formatAsDecl(oldVar->getType(), newVarName) + initExpr + ";\n";
    getRewriter().InsertTextAfterToken(FD->getBody()->getBeginLoc(), newDecl);

    for(auto &E : TheRefs) {
      FunctionDecl *ParentFD = getParentFunction(E);
      if(E->getDecl() == oldVar && ParentFD == FD) {
        getRewriter().ReplaceText(E->getSourceRange(), newVarName);
      }
    }

    return true;
  }

private:
  std::vector<DeclRefExpr *> TheRefs;
  std::map<FunctionDecl *, std::set<Stmt *> *> FunctionStmts;

  FunctionDecl *getParentFunction(Stmt *S) {
    for (auto it : FunctionStmts) 
      if(it.second->find(S) != it.second->end())
        return it.first;
    return nullptr;
  }

  std::set<Stmt *> *collectStmts(Stmt *S) {
    std::set<Stmt *> *stmts = new std::set<Stmt *>();
    for(auto it = S->child_begin(); it != S->child_end(); ++it) {
      stmts->insert(*it);
      if(Stmt *childStmt = dyn_cast<Stmt>(*it))
        for(auto j : *collectStmts(childStmt))
          stmts->insert(j);
    }
    return stmts;
  }
};

static RegisterMutator<ReduceScope> M("u3.ReduceScope", "Replace global variable usage with a local variable in a specific function.");
