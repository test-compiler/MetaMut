// IncreasePointerLevel.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class IncreasePointerLevel : public Mutator,
                             public clang::RecursiveASTVisitor<IncreasePointerLevel> {
public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty()) return false;

    VarDecl *var = randElement(TheVars);

    // perform rewrite
    QualType newType = getASTContext().getPointerType(var->getType());
    std::string newDecl = formatAsDecl(newType, var->getNameAsString());

    getRewriter().ReplaceText(var->getSourceRange(), newDecl);
    
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (VD->getType()->isPointerType()) {
      TheVars.push_back(VD);
    }
    return true;
  }

private:
  std::vector<clang::VarDecl *> TheVars;
};

static RegisterMutator<IncreasePointerLevel> M(
    "IncreasePointerLevel", "Increase the pointer level of a variable.");