#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Expr.h>

using namespace clang;

class ReplaceReturnWithExit : public Mutator,
                      public clang::RecursiveASTVisitor<ReplaceReturnWithExit> {
public:
  using Mutator::Mutator;

  bool VisitReturnStmt(ReturnStmt *RS) {
    returnStmts.push_back(RS);
    return true;
  }

  bool mutate() override {
    TraverseDecl(getASTContext().getTranslationUnitDecl());

    if (returnStmts.empty()) {
      return false;
    }

    ReturnStmt *retStmt = randElement(returnStmts);
    
    Expr *returnExpr = retStmt->getRetValue();
    std::string returnExprText = Lexer::getSourceText(CharSourceRange::getTokenRange(returnExpr->getSourceRange()),
                                                      getASTContext().getSourceManager(), LangOptions(), 0).str();
    
    std::string newCode = "exit(" + returnExprText + ")";
    getRewriter().ReplaceText(retStmt->getSourceRange(), newCode);
    
    return true;
  }

private:
  std::vector<ReturnStmt *> returnStmts;
};

static RegisterMutator<ReplaceReturnWithExit> M("u3.ReplaceReturnWithExit", "Replaces a 'return' statement in a function with 'exit', effectively changing the control flow of the program.");
