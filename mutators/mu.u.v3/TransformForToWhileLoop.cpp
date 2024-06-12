#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Sema/Sema.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;

class TransformForToWhileLoop : public Mutator,
                                public clang::RecursiveASTVisitor<TransformForToWhileLoop> {
public:
  using Mutator::Mutator;

  bool VisitForStmt(clang::ForStmt *FS) {
    TheLoops.push_back(FS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheLoops.empty()) return false;

    ForStmt *FS = randElement(TheLoops);
    Stmt *init = FS->getInit();
    Expr *cond = FS->getCond();
    Expr *inc = FS->getInc();
    Stmt *body = FS->getBody();
    
    if (init == nullptr || cond == nullptr || inc == nullptr || body == nullptr)
        return false; // Skip loops that lack any of the parts

    std::string loop = getSourceText(init) + ";\nwhile(" + getSourceText(cond) + ") {\n" + getSourceText(body) + getSourceText(inc) + ";\n}";

    getRewriter().ReplaceText(FS->getSourceRange(), loop);

    return true;
  }

private:
  std::vector<clang::ForStmt *> TheLoops;
};

static RegisterMutator<TransformForToWhileLoop> M("u3.TransformForToWhileLoop", "Transform 'for' loop into equivalent 'while' loop");
