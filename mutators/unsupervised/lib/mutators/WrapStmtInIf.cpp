#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class WrapStmtInIf : public Mutator,
                     public clang::RecursiveASTVisitor<WrapStmtInIf> {

public:
    using Mutator::Mutator;

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheStmts.empty())
            return false;

        // select random statement
        Stmt *stmt = randElement(TheStmts);

        // generate unique name for the boolean variable
        std::string boolVarName = generateUniqueName("cond");

        // prepare new code
        std::string newCode = "bool " + boolVarName + " = rand() % 2;\n"
                              "if (" + boolVarName + ") " +
                              getSourceText(stmt).str();

        // perform rewrite
        getRewriter().ReplaceText(stmt->getSourceRange(), newCode);

        // add includes at the top of the function
        if (CurrentFunction) {
            getRewriter().InsertTextBefore(getMostRecentTranslationUnitDecl(CurrentFunction)->getBeginLoc(),
                                           "#include <stdbool.h>\n#include <stdlib.h>\n");
        }

        return true;
    }

    bool VisitStmt(clang::Stmt *S) {
        if (!isa<IfStmt>(S) && !isa<ForStmt>(S) && !isa<WhileStmt>(S) &&
            !isa<DoStmt>(S) && !isa<SwitchStmt>(S) && !isa<CompoundStmt>(S)) {
            TheStmts.push_back(S);
        }
        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *FD) {
        CurrentFunction = FD;
        return true;
    }

private:
    FunctionDecl *CurrentFunction = nullptr;
    std::vector<clang::Stmt *> TheStmts;
};

static RegisterMutator<WrapStmtInIf> M(
    "WrapStmtInIf", "Wrap a random non-control-flow statement with an if-statement with a random boolean condition.");