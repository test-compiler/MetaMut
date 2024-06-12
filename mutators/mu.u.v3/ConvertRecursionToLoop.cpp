#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class ConvertRecursionToLoop : public Mutator,
                      public RecursiveASTVisitor<ConvertRecursionToLoop> {
public:
  using Mutator::Mutator;

  bool VisitCallExpr(CallExpr *E) {
    if (auto FD = dyn_cast_or_null<FunctionDecl>(E->getDirectCallee())) {
        if (FD == CurrentFunction)
            IsRecursive = true;
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    IsRecursive = false;
    CurrentFunction = FD;
    TraverseStmt(FD->getBody());
    if (IsRecursive)
        RecursiveFunctions.push_back(FD);
    return true;
  }

  bool mutate() override {
    TraverseDecl(getASTContext().getTranslationUnitDecl());
    if (RecursiveFunctions.empty())
        return false;

    FunctionDecl *selectedFunction = randElement(RecursiveFunctions);
    Stmt *functionBody = selectedFunction->getBody();
    std::string funcName = selectedFunction->getNameInfo().getAsString();
    std::stringstream newFuncBody;

    newFuncBody << "{\n";
    newFuncBody << "while(1) {\n";

    for (auto it = functionBody->child_begin(); it != functionBody->child_end(); ++it) {
        if (CallExpr *callExpr = dyn_cast<CallExpr>(*it)) {
            if (callExpr->getDirectCallee()->getNameInfo().getAsString() == funcName) {
                newFuncBody << "continue;\n";
            } else {
                newFuncBody << getSourceText(*it) << "\n";
            }
        } else {
            newFuncBody << getSourceText(*it) << "\n";
        }
    }

    newFuncBody << "}\n";
    newFuncBody << "}";

    getRewriter().ReplaceText(functionBody->getSourceRange(), newFuncBody.str());

    return true;
  }

private:
  bool IsRecursive;
  FunctionDecl *CurrentFunction;
  std::vector<FunctionDecl *> RecursiveFunctions;

  std::string getSourceText(const Stmt *stmt) {
    SourceRange range = stmt->getSourceRange();
    range.setEnd(getLocForEndOfToken(range.getEnd()));
    return Lexer::getSourceText(CharSourceRange::getTokenRange(range),
                                getASTContext().getSourceManager(),
                                getASTContext().getLangOpts()).str();
  }
};

static RegisterMutator<ConvertRecursionToLoop> M("u3.ConvertRecursionToLoop", "Identifies a recursive function and attempts to convert it to use loops");
