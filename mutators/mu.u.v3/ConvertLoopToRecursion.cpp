#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;

class ConvertLoopToRecursion : public Mutator,
                               public RecursiveASTVisitor<ConvertLoopToRecursion> {
public:
  using Mutator::Mutator;

  bool VisitForStmt(ForStmt *FS) {
    TheLoops.push_back(FS);
    return true;
  }

  bool VisitWhileStmt(WhileStmt *WS) {
    TheLoops.push_back(WS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLoops.empty())
      return false;

    Stmt *loop = randElement(TheLoops);
    std::string funcName = generateUniqueName("recursiveFunction");
    
    Rewriter funcRewriter(getASTContext().getSourceManager(), getASTContext().getLangOpts());
    Rewriter callRewriter(getASTContext().getSourceManager(), getASTContext().getLangOpts());

    if (ForStmt *forLoop = dyn_cast<ForStmt>(loop)) {
      Expr *cond = forLoop->getCond();
      Stmt *body = forLoop->getBody();
      std::string insertText = "void " + funcName + "()\n{\nif(" + getSourceText(cond) + ") {\n" + getSourceText(body) + "\n" + funcName + "();\n}\n}\n";
      
      funcRewriter.InsertTextBefore(loop->getBeginLoc(), insertText);
      callRewriter.ReplaceText(loop->getSourceRange(), funcName + "();");
    }
    else if (WhileStmt *whileLoop = dyn_cast<WhileStmt>(loop)) {
      Expr *cond = whileLoop->getCond();
      Stmt *body = whileLoop->getBody();
      std::string insertText = "void " + funcName + "()\n{\nif(" + getSourceText(cond) + ") {\n" + getSourceText(body) + "\n" + funcName + "();\n}\n}\n";
      
      funcRewriter.InsertTextBefore(loop->getBeginLoc(), insertText);
      callRewriter.ReplaceText(loop->getSourceRange(), funcName + "();");
    }
    
    getRewriter() = funcRewriter;
    getRewriter() = callRewriter;

    return true;
  }

private:
  std::vector<Stmt *> TheLoops;
};

static RegisterMutator<ConvertLoopToRecursion> M("u3.ConvertLoopToRecursion", "Transform a loop into an equivalent recursive function");
