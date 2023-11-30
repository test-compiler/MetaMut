#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ReplaceArrayWithPointer : public Mutator,
                      public clang::RecursiveASTVisitor<ReplaceArrayWithPointer> {
  
  using VisitorTy = clang::RecursiveASTVisitor<ReplaceArrayWithPointer>;

public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheArrays.empty()) return false;

    // Select a random VarDecl of ArrayType
    VarDecl *array = randElement(TheArrays);

    // Generate new pointer declaration
    QualType arrayType = array->getType()->getAsArrayTypeUnsafe()->getElementType().getUnqualifiedType();
    std::string pointerType;
    llvm::raw_string_ostream stream(pointerType);
    arrayType.print(stream, getASTContext().getPrintingPolicy());

    std::string pointerDecl = stream.str() + "** " + array->getNameAsString() + " = " + array->getNameAsString() + ";";
    getRewriter().ReplaceText(array->getSourceRange(), pointerDecl);

    // Replace all array access expressions with pointer dereference
    for (auto &arrayAccess : ArrayAccessMap[array]) {
      Expr::EvalResult result;
      if (arrayAccess->getIdx()->EvaluateAsInt(result, getASTContext())) {
        std::string pointerAccess = "*( " + array->getNameAsString() + " + " + std::to_string(result.Val.getInt().getZExtValue()) + ")";
        getRewriter().ReplaceText(arrayAccess->getSourceRange(), pointerAccess);
      }
    }

    return true;
  }

  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE) {
    if (isMutationSite(ASE)) {
      TheAccesses.push_back(ASE);
      if (DeclRefExpr *base = dyn_cast<DeclRefExpr>(ASE->getBase()->IgnoreParenImpCasts()->IgnoreImplicit())) {
        ArrayAccessMap[dyn_cast<VarDecl>(base->getDecl())].push_back(ASE);
      }
    }
    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    if (isMutationSite(VD) && VD->getType()->isArrayType()) {
      TheArrays.push_back(VD);
    }
    return true;
  }

private:
  std::map<clang::VarDecl *, std::vector<clang::ArraySubscriptExpr *>> ArrayAccessMap;
  std::vector<clang::VarDecl *> TheArrays;
  std::vector<clang::ArraySubscriptExpr *> TheAccesses;
};

static RegisterMutator<ReplaceArrayWithPointer> M(
    "ReplaceArrayWithPointer", "Change an ArrayDecl to PointerDecl and modify all relevant array access expressions to pointer dereference.");