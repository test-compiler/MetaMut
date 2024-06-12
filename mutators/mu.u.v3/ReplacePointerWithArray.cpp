#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Expr.h>
#include <clang/Lex/Lexer.h>

using namespace clang;

class ReplacePointerWithArray : public Mutator,
                                public RecursiveASTVisitor<ReplacePointerWithArray> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    // Only consider simple pointer declarations, exclude dynamic memory allocation
    if (VD->getType()->isPointerType() && !isa<ParmVarDecl>(VD) && !(VD->hasInit() && isa<CallExpr>(VD->getInit()))) {
      PointerDecls.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (PointerDecls.empty()) return false;

    VarDecl *oldDecl = randElement(PointerDecls);
    
    // Get type of elements that the pointer points to
    QualType elementType = oldDecl->getType()->getPointeeType();
    
    // Construct new type as an array type
    ASTContext &Ctxt = getASTContext();
    QualType newType = Ctxt.getConstantArrayType(elementType, llvm::APInt(32, 5, false), nullptr, ArrayType::Normal, 0);

    // Format new declaration
    std::string newDeclaration = formatAsDecl(newType, oldDecl->getNameAsString());

    // Replace VarDecl's declarator
    replaceVarDeclarator(oldDecl, newType);

    return true;
  }

private:
  std::vector<VarDecl*> PointerDecls;
};

static RegisterMutator<ReplacePointerWithArray> M("u3.ReplacePointerWithArray", "Replace a pointer with an equivalent array");
