// InsertRandomPrintf.cpp
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Sema/Sema.h>
#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRandomPrintf : public Mutator,
                           public clang::RecursiveASTVisitor<InsertRandomPrintf> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(clang::FunctionDecl *FD) {
    // Collect all function declarations
    TheFunctions.push_back(FD);
    return true;
  }

  bool mutate() override {
    // Traverse the AST
    TraverseAST(getASTContext());

    // If no functions are found, return false
    if (TheFunctions.empty()) return false;

    // Randomly select a function
    FunctionDecl *selectedFunc = randElement(TheFunctions);

    // Insert printf at the beginning of the function body
    std::string printfStmt = "printf(\"Debug: Inside function " + selectedFunc->getNameAsString() + "\\n\");\n";
    SourceLocation insertionPoint = selectedFunc->getBody()->getBeginLoc().getLocWithOffset(1);
    getRewriter().InsertText(insertionPoint, printfStmt);

    return true;
  }

private:
  // Store all function declarations
  std::vector<clang::FunctionDecl *> TheFunctions;
};

// Register the mutator to the mutator manager
static RegisterMutator<InsertRandomPrintf> M("u3.InsertRandomPrintf", 
  "This mutator identifies a function in the code and inserts a random 'printf' debugging statement at the beginning of the function body, effectively adding debug logging to the function."
);
