#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class ReplaceForWithWhileLoop : public Mutator, 
                                public clang::RecursiveASTVisitor<ReplaceForWithWhileLoop> {
public:
  using Mutator::Mutator;

  bool VisitForStmt(ForStmt *FS) {
    TheLoops.push_back(FS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheLoops.empty())
      return false;

    ForStmt *fs = randElement(TheLoops);
    Stmt *init = fs->getInit();
    Stmt *body = fs->getBody();
    Expr *cond = fs->getCond();
    Expr *inc = fs->getInc();

    // Get loop's initialization, body, condition, and increment expression source text
    std::string initText = getSourceText(init);
    std::string bodyText = getSourceText(body);
    std::string condText = getSourceText(cond);
    std::string incText = getSourceText(inc);

    // Remove the trailing semicolon from the body text
    if (!bodyText.empty() && bodyText.back() == ';') {
        bodyText.pop_back();
    }

    // Construct a while loop with the same semantics
    std::string whileLoop = initText + "; while (" + condText + ") { " + bodyText + " " + incText + "; }";

    // Replace for loop with equivalent while loop
    getRewriter().ReplaceText(fs->getSourceRange(), whileLoop);
    
    return true;
  }

private:
  std::vector<ForStmt*> TheLoops;
};

static RegisterMutator<ReplaceForWithWhileLoop> M("u3.ReplaceForWithWhileLoop", "Replace a 'for' loop with equivalent 'while' loop");
