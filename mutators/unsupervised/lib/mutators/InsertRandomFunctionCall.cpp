// InsertRandomFunctionCall.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>

#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class InsertRandomFunctionCall
    : public Mutator,
      public clang::RecursiveASTVisitor<InsertRandomFunctionCall> {
public:
  using Mutator::Mutator;
  
  bool mutate() override {
    TraverseAST(getASTContext());
    if (TheFunctions.empty() || CompoundStmts.empty()) return false;

    FunctionDecl *func = randElement(TheFunctions);
    CompoundStmt *stmt = randElement(CompoundStmts);

    // Generate function call
    std::string funcCall = func->getNameInfo().getAsString() + "(";
    for (auto it = func->param_begin(); it != func->param_end(); ++it) {
      funcCall += (*it)->getNameAsString();
      if(std::next(it) != func->param_end())
        funcCall += ", ";
    }
    funcCall += ");\n";

    // Insert the function call at a random position in the CompoundStmt
    int randomStmtIndex = randIndex(stmt->size());
    auto stmtToInsertAfter = stmt->body_begin();
    std::advance(stmtToInsertAfter, randomStmtIndex);
    
    SourceLocation insertLoc = getLocForEndOfToken((*stmtToInsertAfter)->getEndLoc());
    getRewriter().InsertTextAfter(insertLoc, funcCall);

    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    // Only consider functions with body (definition) and parameters
    if(FD->hasBody() && FD->getNumParams() > 0) {
      TheFunctions.push_back(FD);
    }
    return true;
  }

  bool VisitCompoundStmt(clang::CompoundStmt *CS) {
    CompoundStmts.push_back(CS);
    return true;
  }

private:
  std::vector<clang::FunctionDecl *> TheFunctions;
  std::vector<clang::CompoundStmt *> CompoundStmts;
};

static RegisterMutator<InsertRandomFunctionCall> X(
    "InsertRandomFunctionCall", "Insert a random function call in a CompoundStmt.");