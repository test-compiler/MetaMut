#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class InsertRedundantPointerCheck
    : public Mutator,
      public clang::RecursiveASTVisitor<InsertRedundantPointerCheck> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getType()->isPointerType() && dyn_cast<FunctionDecl>(VD->getDeclContext())) {
      ThePointers.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (ThePointers.empty()) return false;

    VarDecl *ptr = randElement(ThePointers);
    std::string check = "if(" + ptr->getName().str() + " != NULL) {\n";
    auto *FD = dyn_cast<FunctionDecl>(ptr->getDeclContext());
    if (!FD || !FD->getBody()) return false;

    std::string originalStmt = getSourceText(FD->getBody());

    getRewriter().ReplaceText(FD->getBody()->getSourceRange(), "{\n" + check + originalStmt + "}\n}");

    return true;
  }

private:
  std::vector<clang::VarDecl *> ThePointers;
};

static RegisterMutator<InsertRedundantPointerCheck> M("u3.InsertRedundantPointerCheck", 
    "Inserts redundant NULL pointer check conditions before the pointer usage");
