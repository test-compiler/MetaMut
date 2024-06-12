#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;

class InsertRandomException : public Mutator,
                      public clang::RecursiveASTVisitor<InsertRandomException> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->getReturnType()->isVoidType() || FD->getBody()->children().empty()) {
      return true; // Skip non-void functions and empty bodies
    }
    for (Stmt* S : FD->getBody()->children()) {
      TheStmts.push_back({FD, S});
    }
    return true;
  }

  bool mutate() override {
    // Step 1, Traverse the AST
    TraverseAST(getASTContext());

    // Early exit if there is no mutation target
    if (TheStmts.empty()) {
      return false;
    }

    // Step 2, Randomly select a mutation instance
    auto selected_instance = randElement(TheStmts);
    FunctionDecl *parent_function = selected_instance.first;
    Stmt *selected_stmt = selected_instance.second;

    // Step 3, Generate unique identifier for the exception
    std::string exception_identifier = generateUniqueName("exception");

    // Step 4, Insert a setjmp statement at the beginning of the function
    // and a corresponding longjmp statement at the randomly selected statement
    std::string insert_text = "jmp_buf " + exception_identifier + ";\n" +
                              "if (setjmp(" + exception_identifier + ") != 0) {\n" +
                              "    printf(\"Exception thrown at function %s!\\n\", __func__);\n" +
                              "    return;\n" +
                              "}\n";
                              
    getRewriter().InsertTextBefore(parent_function->getBody()->getBeginLoc().getLocWithOffset(1), insert_text);

    insert_text = "longjmp(" + exception_identifier + ", 1);\n";
    getRewriter().InsertTextBefore(selected_stmt->getBeginLoc(), insert_text);

    // Insert necessary include at the top of the file
    insert_text = "#include <setjmp.h>\n";
    getRewriter().InsertTextBefore(getMostRecentTranslationUnitDecl(parent_function)->getBeginLoc(), insert_text);

    // Step 5, Return true if changed
    return true;
  }

private:
  std::vector<std::pair<FunctionDecl *, Stmt *>> TheStmts;
};

static RegisterMutator<InsertRandomException> M("u3.InsertRandomException", "Introduce a new potential exception throw point.");
