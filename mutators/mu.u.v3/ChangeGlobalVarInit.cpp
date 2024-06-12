#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

#include <random>

using namespace clang;

class ChangeGlobalVarInit : public Mutator,
                            public clang::RecursiveASTVisitor<ChangeGlobalVarInit> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    if(VD->hasGlobalStorage() && VD->hasInit()) {
      TheVars.push_back(VD);
    }
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if(TheVars.empty()) {
      return false;
    }

    VarDecl *VD = randElement(TheVars);

    // Generate new initialization based on the variable type
    std::string newInit = generateNewInitialization(VD->getType());

    // Replace the old init with the new one
    SourceRange oldInitRange = VD->getInit()->getSourceRange();
    getRewriter().ReplaceText(oldInitRange, newInit);
    
    return true;
  }

private:
  std::vector<VarDecl*> TheVars;

  std::string generateNewInitialization(QualType type) {
    std::mt19937 &rng = getManager().getRandomGenerator();
    if(type->isIntegerType()) {
      std::uniform_int_distribution<int> distr;
      return std::to_string(distr(rng));
    } else if(type->isRealFloatingType()) {
      std::uniform_real_distribution<double> distr;
      return std::to_string(distr(rng));
    } else if(type->isPointerType()) {
      return "NULL";
    } else if(type->isBooleanType()) {
      return rng() % 2 ? "true" : "false";
    } else if(type->isStructureType()) {
      return "{}";
    } else {
      // We do not handle this type yet
      return "0";
    }
  }
};

static RegisterMutator<ChangeGlobalVarInit> M("u3.ChangeGlobalVarInit", "Change global variable's initialization to a different compatible value");
