#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;

class ReplaceScalarWithArray : public Mutator,
                               public clang::RecursiveASTVisitor<ReplaceScalarWithArray> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    if (!VD->getType()->isScalarType())
      return true;
    TheVars.push_back(VD);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty())
      return false;

    VarDecl *var = randElement(TheVars);

    QualType newType = getASTContext().getConstantArrayType(
        var->getType(), llvm::APInt(32, 1), nullptr, ArrayType::Normal, 0);

    replaceVarDeclarator(var, newType);
    
    if (var->hasInit()) {
      SourceLocation endLoc = getLocForEndOfToken(var->getEndLoc());
      std::string initializer = getSourceText(var->getInit());
      std::string newInitializer = "{" + initializer + "}";
      getRewriter().ReplaceText(SourceRange(var->getInit()->getBeginLoc(), endLoc), newInitializer + ";");
    }

    for (DeclRefExpr *Ref : TheVarRefs[var]) {
        getRewriter().ReplaceText(Ref->getSourceRange(), "(" + getSourceText(Ref) + ")[0]");
    }
    
    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *Ref) {
    if (VarDecl *VD = dyn_cast<VarDecl>(Ref->getDecl())) {
      TheVarRefs[VD].push_back(Ref);
    }
    return true;
  }

private:
  std::vector<VarDecl *> TheVars;
  std::map<VarDecl *, std::vector<DeclRefExpr *>> TheVarRefs;
};

static RegisterMutator<ReplaceScalarWithArray> M("u3.ReplaceScalarWithArray", "Transform a scalar variable into an array of size one");
