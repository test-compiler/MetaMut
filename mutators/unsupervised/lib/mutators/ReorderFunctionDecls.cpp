#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ReorderFunctionDecls : public Mutator, public RecursiveASTVisitor<ReorderFunctionDecls> {
public:
  using Mutator::Mutator;

  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFuncs.size() < 2) return false;

    // Randomly select two different function declarations
    int idx1 = randIndex(TheFuncs.size());
    int idx2;
    do {
        idx2 = randIndex(TheFuncs.size());
    } while(idx1 == idx2);

    FunctionDecl *func1 = TheFuncs[idx1];
    FunctionDecl *func2 = TheFuncs[idx2];

    // Get function source code
    std::string func1Code = getSourceText(func1).str();
    std::string func2Code = getSourceText(func2).str();

    // Swap the function declarations
    getRewriter().ReplaceText(func1->getSourceRange(), func2Code);
    getRewriter().ReplaceText(func2->getSourceRange(), func1Code);

    // If the function is used before its definition, add a prototype at the beginning of the file
    if (isUsedBeforeDefinition(func1))
        getRewriter().InsertTextBefore(getMostRecentTranslationUnitDecl(func1)->getBeginLoc(), getPrototype(func1) + ";\n");
    if (isUsedBeforeDefinition(func2))
        getRewriter().InsertTextBefore(getMostRecentTranslationUnitDecl(func2)->getBeginLoc(), getPrototype(func2) + ";\n");

    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->isThisDeclarationADefinition()) return true;

    // Only consider function if definition is within main file (not in an included file)
    SourceManager &SM = getASTContext().getSourceManager();
    if (!SM.isInMainFile(FD->getBeginLoc())) return true;

    TheFuncs.push_back(FD);
    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    if (FunctionDecl *FD = CE->getDirectCallee())
      FuncUses[FD].insert(CE);
    return true;
  }

private:
  std::vector<FunctionDecl *> TheFuncs;
  std::map<FunctionDecl *, std::set<const CallExpr *>> FuncUses;

  // Returns true if the function is used before its definition
  bool isUsedBeforeDefinition(FunctionDecl *FD) {
    SourceManager &SM = getASTContext().getSourceManager();
    for (auto use : FuncUses[FD]) {
      if (SM.isBeforeInTranslationUnit(use->getBeginLoc(), FD->getBeginLoc()))
        return true;
    }
    return false;
  }

  // Returns the prototype of a function
  std::string getPrototype(FunctionDecl *FD) {
    std::string prototype = FD->getReturnType().getAsString();
    prototype += " " + FD->getNameAsString() + "(";
    for (auto param = FD->param_begin(); param != FD->param_end(); ++param) {
      if (param != FD->param_begin()) prototype += ", ";
      prototype += (*param)->getType().getAsString();
    }
    prototype += ")";
    return prototype;
  }
};

static RegisterMutator<ReorderFunctionDecls> M("ReorderFunctionDecls", "Randomly reorder two function declarations in the translation unit, adding a prototype at the beginning of the file if necessary.");