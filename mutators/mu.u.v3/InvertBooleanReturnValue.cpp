#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InvertBooleanReturnValue
    : public Mutator,
      public clang::RecursiveASTVisitor<InvertBooleanReturnValue> {
public:
  using Mutator::Mutator;

  bool VisitReturnStmt(ReturnStmt *RS) {
    if (RS->getRetValue()) {
      if (auto binOp = dyn_cast<BinaryOperator>(RS->getRetValue()->IgnoreParenImpCasts())) {
        auto opcode = binOp->getOpcode();
        if (opcode == BO_EQ || opcode == BO_NE || opcode == BO_LT || opcode == BO_GT ||
            opcode == BO_LE || opcode == BO_GE) {
          TheReturnStmts.push_back(RS);
        }
      }
    }
    return true;
  }

  bool mutate() override {
    auto root = getCompilerInstance().getASTContext().getTranslationUnitDecl();
    TraverseDecl(root);

    if (TheReturnStmts.empty()) return false;

    ReturnStmt *SelectedStmt = randElement(TheReturnStmts);
    Expr *OriginalExpr = SelectedStmt->getRetValue();
    std::string NewSource = "!(" + getSourceText(OriginalExpr) + ")";
    
    getRewriter().ReplaceText(OriginalExpr->getSourceRange(), NewSource);

    return true;
  }

private:
  std::vector<ReturnStmt *> TheReturnStmts;
};

static RegisterMutator<InvertBooleanReturnValue> M("u3.InvertBooleanReturnValue", "Inverts the return value of a function returning a boolean.");
