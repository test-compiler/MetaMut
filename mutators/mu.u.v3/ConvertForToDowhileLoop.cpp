#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class ConvertForToDowhileLoop : public Mutator, public clang::RecursiveASTVisitor<ConvertForToDowhileLoop> {
public:
  using Mutator::Mutator;
  
  bool VisitForStmt(ForStmt *FS) {
    ForLoops.push_back(FS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (ForLoops.empty()) return false;

    ForStmt *forLoop = randElement(ForLoops);
    
    Stmt *initStmt = forLoop->getInit();
    Expr *condExpr = forLoop->getCond();
    Expr *incExpr = forLoop->getInc();
    Stmt *bodyStmt = forLoop->getBody();
    
    std::string initStr = getSourceText(initStmt);
    std::string condStr = getSourceText(condExpr);
    std::string incStr = getSourceText(incExpr);
    std::string bodyStr = getSourceText(bodyStmt);

    // Conversion for 'for' loop to 'do-while' loop
    std::string newLoopStr = initStr + ";\n";
    newLoopStr += "do {\n";
    newLoopStr += bodyStr + ";\n";
    newLoopStr += incStr + ";\n";
    newLoopStr += "} while (" + condStr + ");";

    getRewriter().ReplaceText(forLoop->getSourceRange(), newLoopStr);

    return true;
  }

private:
  std::vector<ForStmt*> ForLoops;
};

static RegisterMutator<ConvertForToDowhileLoop> M("u3.ConvertForToDowhileLoop",
    "This mutator selects a 'for' loop in the code and converts it into an equivalent 'do-while' loop");
