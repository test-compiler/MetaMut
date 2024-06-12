#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;

class InverseFunctionBody : public Mutator, public RecursiveASTVisitor<InverseFunctionBody> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD->hasBody()) TheFuncs.push_back(FD);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFuncs.empty()) return false;

    FunctionDecl *func = randElement(TheFuncs);
    Stmt *funcBody = func->getBody();

    if (CompoundStmt *CS = dyn_cast<CompoundStmt>(funcBody)) {
        std::vector<Stmt*> stmts(CS->body_begin(), CS->body_end());
        std::reverse(stmts.begin(), stmts.end());

        std::string reversedBody;
        for (Stmt *stmt : stmts) {
            reversedBody += getSourceText(stmt) + ";\n";
        }

        SourceLocation start = CS->getLBracLoc().getLocWithOffset(1);
        SourceLocation end = CS->getRBracLoc().getLocWithOffset(-1);
        getRewriter().ReplaceText(SourceRange(start, end), reversedBody);
        
        return true;
    }

    return false;
  }

private:
  std::vector<FunctionDecl*> TheFuncs;
};

static RegisterMutator<InverseFunctionBody> M("u3.InverseFunctionBody", "Reverse the order of statements in a function's body");
