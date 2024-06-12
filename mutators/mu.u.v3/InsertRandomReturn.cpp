#include "Mutator.h"
#include "MutatorManager.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Sema/Sema.h>

using namespace clang;

class InsertRandomReturn : public Mutator,
                           public clang::RecursiveASTVisitor<InsertRandomReturn> {
public:
    using Mutator::Mutator;

    bool VisitFunctionDecl(FunctionDecl *FD) {
        if (!FD->doesThisDeclarationHaveABody()) return true; // only mutate function definitions with a body
        if (FD->getReturnType()->isVoidType()) return true; // do not mutate void functions
        TheFuncs.push_back(FD);
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (TheFuncs.empty()) return false;

        FunctionDecl *selectedFunc = randElement(TheFuncs);
        Stmt *funcBody = selectedFunc->getBody();
        CompoundStmt *compoundStmt = dyn_cast<CompoundStmt>(funcBody);
        if(!compoundStmt) return false; // function body is not a compound statement

        auto stmts = compoundStmt->body();
        if(std::distance(stmts.begin(), stmts.end()) <= 1) return false; // function body contains 1 or less statements

        unsigned stmtToReplace = randIndex(std::distance(stmts.begin(), stmts.end()) - 1); // -1 to avoid replacing the existing return statement
        Stmt *stmt = *(stmts.begin() + stmtToReplace);

        std::string replacementText = "return " + generateDefaultValue(selectedFunc->getReturnType()) + ";\n" + getSourceText(stmt);
        getRewriter().ReplaceText(stmt->getSourceRange(), replacementText);
  
        return true;
    }

private:
    std::vector<FunctionDecl *> TheFuncs;
  
    std::string generateDefaultValue(QualType type) {
        if (type->isIntegerType())
            return "0";
        else if (type->isRealFloatingType())
            return "0.0";
        else if (type->isPointerType())
            return "NULL";
        else if (type->isBooleanType())
            return "false";
        else
            return "{}";
    }
};

static RegisterMutator<InsertRandomReturn> M("u3.InsertRandomReturn",
    "Identifies a non-void function and randomly inserts a return statement in the middle of its body");
