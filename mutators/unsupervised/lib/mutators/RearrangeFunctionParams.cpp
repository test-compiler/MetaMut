#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;
using namespace ysmut;

class RearrangeFunctionParams : public Mutator,
                                public RecursiveASTVisitor<RearrangeFunctionParams> {
public:
  RearrangeFunctionParams(const char *name, const char *desc) : Mutator(name, desc) {}

  bool mutate() override {
    TraverseDecl(getASTContext().getTranslationUnitDecl());

    if (funcDecls.empty()) {
      return false;
    }

    for (FunctionDecl *funcDecl : funcDecls) {
      if (funcDecl->param_size() < 2) {
        continue;
      }

      // Get the parameters in a vector and shuffle them
      std::vector<ParmVarDecl *> params(funcDecl->param_begin(), funcDecl->param_end());
      std::shuffle(params.begin(), params.end(), getManager().getRandomGenerator());

      // Rewrite the function declaration with rearranged parameters
      std::string newDecl = getRewriter().getRewrittenText(getSourceRange(funcDecl));
      unsigned start = newDecl.find('(');
      unsigned end = newDecl.find(')');
      std::string newParamList;

      for (auto param = params.begin(); param != params.end(); ++param) {
        if (param != params.begin()) {
          newParamList += ", ";
        }
        newParamList += (*param)->getType().getAsString() + " " + (*param)->getNameAsString();
      }

      newDecl.replace(start + 1, end - start - 1, newParamList);
      getRewriter().ReplaceText(getSourceRange(funcDecl), newDecl);
    }

    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *funcDecl) {
    funcDecls.push_back(funcDecl);
    return true;
  }

private:
  std::vector<FunctionDecl *> funcDecls;

  SourceRange getSourceRange(FunctionDecl *funcDecl) {
    return SourceRange(funcDecl->getBeginLoc(),
                       getLocForEndOfToken(funcDecl->getBody()->getBeginLoc().getLocWithOffset(-1)));
  }
};

static RegisterMutator<RearrangeFunctionParams> M(
    "RearrangeFunctionParams", "Rearranges the parameters of function declarations randomly.");