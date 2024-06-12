#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;

class ReplaceDynamicAllocationWithStatic
    : public Mutator,
      public clang::RecursiveASTVisitor<ReplaceDynamicAllocationWithStatic> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(CallExpr *CE) {
    if (!FDStack.empty() && CE->getDirectCallee() && CE->getArg(0)) {
      auto callee = CE->getDirectCallee();
      if (callee->getName().equals("malloc")) {
        TheMallocs.push_back(std::make_pair(FDStack.back(), CE));
      } else if (callee->getName().equals("calloc")) {
        TheCallocs.push_back(std::make_pair(FDStack.back(), CE));
      }
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    FDStack.push_back(FD);
    return true;
  }

  bool TraverseFunctionDecl(FunctionDecl *FD) {
    FDStack.push_back(FD);
    RecursiveASTVisitor<ReplaceDynamicAllocationWithStatic>::TraverseFunctionDecl(FD);
    FDStack.pop_back();
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheMallocs.empty() && TheCallocs.empty()) return false;

    // Randomly select an operation
    if (randBool()) {
      if (!TheMallocs.empty()) {
        auto pair = randElement(TheMallocs);
        replaceWithStatic(pair.first, pair.second, "malloc");
      }
    } else {
      if (!TheCallocs.empty()) {
        auto pair = randElement(TheCallocs);
        replaceWithStatic(pair.first, pair.second, "calloc");
      }
    }

    return true;
  }

private:
  std::vector<std::pair<FunctionDecl *, CallExpr *>> TheMallocs;
  std::vector<std::pair<FunctionDecl *, CallExpr *>> TheCallocs;
  std::vector<FunctionDecl *> FDStack;

  void replaceWithStatic(FunctionDecl *FD, CallExpr *CE, const std::string &func) {
    // We replace malloc with variable/array declaration
    auto sizeExpr = CE->getArg(0);
    auto srcRange = getExpansionRange(CE->getSourceRange());
    std::string varName = generateUniqueName("staticAlloc");
    
    std::string varDecl = func == "malloc" 
        ? "int " + varName + ";\n" 
        : "int " + varName + "[" + getSourceText(sizeExpr) + "];\n";

    // Now we add the declaration string at the beginning of the function body
    addStringBeforeFunctionDecl(FD, varDecl);
    
    getRewriter().ReplaceText(srcRange, varName);
  }
};

static RegisterMutator<ReplaceDynamicAllocationWithStatic> M("u3.ReplaceDynamicAllocationWithStatic",
    "This mutator identifies dynamic memory allocation in the code (malloc, calloc) and replaces it with static allocation (array, variable declaration), effectively changing the dynamic memory usage to static.");
