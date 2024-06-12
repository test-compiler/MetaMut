#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class AddConstQualifier : public Mutator,
                          public clang::RecursiveASTVisitor<AddConstQualifier> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(clang::VarDecl *VD) {
    // Do not collect if already const
    if (!VD->getType().isConstQualified()) {
      TheVars.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    // Step 1, Traverse the AST
    TraverseAST(getASTContext());

    // No valid variable declarations found
    if (TheVars.empty()) return false;

    // Step 3, Randomly select a mutation instance
    VarDecl *VD = randElement(TheVars);

    // Step 4, Checking mutation validity is done in VisitVarDecl

    // Step 5, Perform mutation
    std::string NewVar = "const " + getSourceText(VD);
    getRewriter().ReplaceText(VD->getSourceRange(), NewVar);

    // Step 6, Return true if changed
    return true;
  }

private:
  std::vector<VarDecl *> TheVars;
};

static RegisterMutator<AddConstQualifier> M("u3.AddConstQualifier", "Add const qualifier to selected variable declarations");
