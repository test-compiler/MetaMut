#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class RemoveUnusedFunction : public Mutator, 
                             public clang::RecursiveASTVisitor<RemoveUnusedFunction> {
public:
  using Mutator::Mutator;
  
  bool VisitFunctionDecl(FunctionDecl* FD) {
    if (FD->isUsed()) return true;
    
    UnusedFunctions.push_back(FD);
    return true;
  }
  
  bool mutate() override {
    TraverseAST(getASTContext());
    
    if (UnusedFunctions.empty()) return false;
    
    FunctionDecl* FD = randElement(UnusedFunctions);
    
    getRewriter().RemoveText(FD->getSourceRange());
    
    return true;
  }
  
private:
  std::vector<FunctionDecl*> UnusedFunctions;
};

static RegisterMutator<RemoveUnusedFunction> M("u3.RemoveUnusedFunction", "Remove a function that is defined but never called");
