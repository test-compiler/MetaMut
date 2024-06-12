#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class SplitVariableDeclaration : public Mutator,
                                 public clang::RecursiveASTVisitor<SplitVariableDeclaration> {
public:
  using Mutator::Mutator;

  bool VisitDeclStmt(DeclStmt *DS) {
    if (DS->decl_begin() != DS->decl_end()) {
      DeclStmt::decl_iterator it = DS->decl_begin();
      ++it;
      if (it != DS->decl_end()) {
        TheDecls.push_back(DS);
      }
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheDecls.empty()) return false;

    DeclStmt *decl = randElement(TheDecls);
    std::string replacedText;

    for (auto it = decl->decl_begin(); it != decl->decl_end(); ++it) {
      if (VarDecl *var = dyn_cast<VarDecl>(*it)) {
        std::string oldVarName = var->getNameAsString();
        std::string newVarName = generateUniqueName(oldVarName);
        std::string initializerText = var->hasInit() ? (" = " + getSourceText(var->getInit())) : "";

        replacedText += formatAsDecl(var->getType(), newVarName) + initializerText + ";\n";

        // Using a RecursiveASTVisitor to replace all usages of the old variable name with the new variable name
        ReplaceVarUsageVisitor replaceVarUsageVisitor(oldVarName, newVarName, *this);
        replaceVarUsageVisitor.TraverseDecl(getContainingFunction(var));
      }
    }

    getRewriter().ReplaceText(decl->getSourceRange(), replacedText);

    return true;
  }

private:
  std::vector<DeclStmt *> TheDecls;

  FunctionDecl* getContainingFunction(Decl* decl) {
    DeclContext* ctx = decl->getDeclContext();
    while (ctx && !ctx->isFunctionOrMethod()) {
      ctx = ctx->getParent();
    }
    return ctx ? cast<FunctionDecl>(ctx) : nullptr;
  }

  class ReplaceVarUsageVisitor : public RecursiveASTVisitor<ReplaceVarUsageVisitor> {
    std::string oldVarName;
    std::string newVarName;
    SplitVariableDeclaration& parentMutator;

  public:
    ReplaceVarUsageVisitor(const std::string& oldVarName, const std::string& newVarName, SplitVariableDeclaration& parentMutator) 
      : oldVarName(oldVarName), newVarName(newVarName), parentMutator(parentMutator) {}

    bool VisitDeclRefExpr(DeclRefExpr *declRefExpr) {
      if (declRefExpr->getNameInfo().getAsString() == oldVarName) {
        parentMutator.getRewriter().ReplaceText(declRefExpr->getLocation(), oldVarName.length(), newVarName);
      }
      return true;
    }
  };
};

static RegisterMutator<SplitVariableDeclaration> M("u3.SplitVariableDeclaration", 
    "Identify a variable declaration that declares and initializes multiple variables and split it into multiple single-variable declaration statements."
);
