#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class ChangeLoopCounter : public Mutator,
                          public clang::RecursiveASTVisitor<ChangeLoopCounter> {
public:
  using Mutator::Mutator;

  bool VisitForStmt(ForStmt *FS) {
    if (auto *BO = dyn_cast<BinaryOperator>(FS->getInit())) {
      if (auto *DRE = dyn_cast<DeclRefExpr>(BO->getLHS())) {
        TheLoops.push_back(FS);
        TheVars.push_back(DRE->getDecl());
      }
    }
    return true;
  }

  bool VisitDoStmt(DoStmt *DS) {
    if (auto *UO = dyn_cast<UnaryOperator>(DS->getBody())) {
      if (UO->getOpcode() == UO_PostInc || UO->getOpcode() == UO_PreInc) {
        if (auto *DRE = dyn_cast<DeclRefExpr>(UO->getSubExpr())) {
          TheLoops.push_back(DS);
          TheVars.push_back(DRE->getDecl());
        }
      }
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLoops.empty()) return false;

    int idx = randIndex(TheLoops.size());
    Stmt *chosenLoop = TheLoops[idx];
    VarDecl *oldVar = cast<VarDecl>(TheVars[idx]);

    std::string newVarName = generateUniqueName(oldVar->getNameAsString());
    std::string newVarDeclaration = formatAsDecl(oldVar->getType(), newVarName) + " = 0;\n";

    getRewriter().InsertTextBefore(chosenLoop->getBeginLoc(), newVarDeclaration);

    if (auto *FS = dyn_cast<ForStmt>(chosenLoop)) {
      auto *BO = cast<BinaryOperator>(FS->getInit());
      getRewriter().ReplaceText(BO->getLHS()->getSourceRange(), newVarName);
      getRewriter().ReplaceText(FS->getCond()->getSourceRange(), newVarName + " < 5");
      getRewriter().ReplaceText(FS->getInc()->getSourceRange(), newVarName + "++");
    }
    else if (auto *DS = dyn_cast<DoStmt>(chosenLoop)) {
      getRewriter().ReplaceText(DS->getCond()->getSourceRange(), newVarName + " < 5");
      
      if (auto *UO = dyn_cast<UnaryOperator>(DS->getBody())) {
        getRewriter().ReplaceText(UO->getSubExpr()->getSourceRange(), newVarName);
      }
    }

    return true;
  }

private:
  std::vector<Stmt*> TheLoops;
  std::vector<NamedDecl*> TheVars;
};

static RegisterMutator<ChangeLoopCounter> M("u3.ChangeLoopCounter", "This mutator selects a loop in the code and changes the loop counter variable to another eligible variable defined in the scope.");
