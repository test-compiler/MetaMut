// ModifyArrayDim.cpp
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ModifyArrayDim : public Mutator,
                       public clang::RecursiveASTVisitor<ModifyArrayDim> {

public:
  using Mutator::Mutator;
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheVars.empty()) return false;

    VarDecl *var = randElement(TheVars);

    if (!var->getType()->isArrayType())
      return false; // Ensure the Decl is actually a ArrayType

    // get the array size
    auto *arrayType = dyn_cast_or_null<ConstantArrayType>(var->getType().getTypePtr());
    if (!arrayType) return false;

    // generate a new size randomly
    unsigned int oldSize = arrayType->getSize().getLimitedValue();
    unsigned int newSize = randBool() ? oldSize + 1 : (oldSize > 1 ? oldSize - 1 : 1);

    // Replace the old size with the new size
    SourceRange arraySizeRange = findBracesRange(var->getSourceRange().getEnd());
    getRewriter().ReplaceText(arraySizeRange, "[" + std::to_string(newSize) + "]");

    return true;
  }

  bool VisitVarDecl(clang::VarDecl *VD) {
    TheVars.push_back(VD);
    return true;
  }

private:
  std::vector<clang::VarDecl *> TheVars;
};

static RegisterMutator<ModifyArrayDim> M(
    "ModifyArrayDim", "Modifies a random ArrayType variable's dimension.");