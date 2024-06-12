#include "Mutator.h"
#include "MutatorManager.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <cassert>

using namespace clang;

class InsertRandomAssert : public Mutator, 
                           public clang::RecursiveASTVisitor<InsertRandomAssert> {

public:
    using Mutator::Mutator;

    bool VisitVarDecl(clang::VarDecl *VD) {
        if (VD->isLocalVarDecl() && VD->getType()->isScalarType()) {
            Vars.push_back(VD);
        }
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());
        if (Vars.empty()) return false;

        auto var = randElement(Vars);
        std::string varName = var->getNameAsString();
        SourceLocation endLoc = getLocForEndOfToken(var->getEndLoc());

        // Ensure the variable declaration is a complete statement
        if (getRewriter().getSourceMgr().getCharacterData(endLoc)[0] != ';')
            getRewriter().InsertText(endLoc.getLocWithOffset(1), ";");

        // Write an assertion checking that the variable's value is not changing unexpectedly.
        std::string assertStmt = "assert(" + varName + " == " + varName + ");\n";
        
        // Insert the assertion after the variable declaration.
        getRewriter().InsertTextAfter(getLocForEndOfToken(endLoc), assertStmt);
        
        return true;
    }

private:
    std::vector<clang::VarDecl*> Vars;
};

static RegisterMutator<InsertRandomAssert> M("u3.InsertRandomAssert", "Inserts a random assert statement checking a variable's value");
