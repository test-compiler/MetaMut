// InsertRandomTypeConversion.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class InsertRandomTypeConversion
    : public Mutator,
      public clang::RecursiveASTVisitor<InsertRandomTypeConversion> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheExprs.empty()) return false;

    Expr *expr = randElement(TheExprs);

    // Create the new type conversion
    std::string newType = generateCompatibleType(expr->getType());
    std::string newExpr = "(" + newType + ")" + getSourceText(expr).str();

    // Perform the rewrite
    getRewriter().ReplaceText(expr->getSourceRange(), newExpr);

    return true;
  }

  bool VisitDeclStmt(clang::DeclStmt *DS) {
    for (auto it = DS->decl_begin(); it != DS->decl_end(); ++it) {
      if (VarDecl *VD = dyn_cast<VarDecl>(*it)) {
        if (VD->hasInit()) {
          TheExprs.push_back(VD->getInit());
        }
      }
    }
    return true;
  }

  bool VisitReturnStmt(clang::ReturnStmt *RS) {
    if (RS->getRetValue()) {
      TheExprs.push_back(RS->getRetValue());
    }
    return true;
  }

private:
  std::vector<clang::Expr *> TheExprs;

  std::string generateCompatibleType(QualType oldType) {
    // This is a simple example. You may want to add more types here
    // or make this function more sophisticated depending on your needs.
    if (oldType->isIntegerType()) {
      return "double";
    } else if (oldType->isRealFloatingType()) {
      return "int";
    } else {
      return "int";
    }
  }
};

static RegisterMutator<InsertRandomTypeConversion> X(
    "InsertRandomTypeConversion", 
    "Insert a random type conversion to a compatible type.");