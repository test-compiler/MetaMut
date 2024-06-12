#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class ReplaceGlobalWithLocal : public Mutator,
                               public clang::RecursiveASTVisitor<ReplaceGlobalWithLocal> {
public:
  using Mutator::Mutator;

  bool VisitVarDecl(VarDecl *VD) {
    if (VD->hasGlobalStorage()) {
      TheGlobalVars.push_back(VD);
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    TheFunctions.push_back(FD);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheGlobalVars.empty() || TheFunctions.empty()) return false;

    VarDecl *oldVar = randElement(TheGlobalVars);
    FunctionDecl *func = randElement(TheFunctions);

    // Prepare new variable
    std::string newVarName = generateUniqueName(oldVar->getNameAsString());
    std::string newVarDeclaration = formatAsDecl(oldVar->getType(), newVarName) + " = " + oldVar->getNameAsString() + ";";
    
    // Insert new variable declaration at the start of the function body
    addStringAfterStmt(*func->getBody()->child_begin(), newVarDeclaration);

    // Replace global variable references with new local variable
    for (auto it = func->getBody()->child_begin(); it != func->getBody()->child_end(); ++it) {
      Stmt* stmt = *it;
      if (stmt != nullptr) {
        clang::SourceRange range = getExpansionRange(stmt->getSourceRange());
        std::string sourceText = getRewriter().getRewrittenText(range);
        size_t pos = sourceText.find(oldVar->getNameAsString());
        while (pos != std::string::npos) {
          getRewriter().ReplaceText(range.getBegin().getLocWithOffset(pos), 
                oldVar->getNameAsString().length(), newVarName);
          pos = sourceText.find(oldVar->getNameAsString(), pos + newVarName.size());
        }
      }
    }
    return true;
  }

private:
  std::vector<VarDecl *> TheGlobalVars;
  std::vector<FunctionDecl *> TheFunctions;
};

static RegisterMutator<ReplaceGlobalWithLocal> M("u3.ReplaceGlobalWithLocal", "Selects a global variable and replaces its usages in a specific function with a local variable of the same type and initial value");
