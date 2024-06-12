#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;

class InsertRedundantCast : public Mutator,
                            public clang::RecursiveASTVisitor<InsertRedundantCast> {
public:
  using Mutator::Mutator;

  bool VisitDeclRefExpr(DeclRefExpr *DRE) {
    TheVars.push_back(DRE);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty()) return false;

    DeclRefExpr *expr = randElement(TheVars);
    SourceLocation start = expr->getBeginLoc();
    std::string CastStr = "(" + expr->getType().getAsString() + ")";
    
    // Insert redundant cast
    getRewriter().InsertTextBefore(start, CastStr);
    return true;
  }

private:
  std::vector<DeclRefExpr *> TheVars;
};

static RegisterMutator<InsertRedundantCast> M("u3.InsertRedundantCast", "Insert a redundant cast to the original type of a variable.");
