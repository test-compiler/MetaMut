#include "Mutator.h"
#include "MutatorManager.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

using namespace clang;

class WrapFunctionWithTimer : public Mutator,
                              public clang::RecursiveASTVisitor<WrapFunctionWithTimer> {
public:
  using Mutator::Mutator;

  bool VisitFunctionDecl(FunctionDecl *FD) {
    TheFunctions.push_back(FD);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheFunctions.empty()) return false;

    FunctionDecl *FD = randElement(TheFunctions);
    Stmt *Body = FD->getBody();

    if (Body) {
      std::string TimerStartCode = "{ clock_t start, end; double cpu_time_used; start = clock();";
      std::string TimerEndCode = "end = clock(); cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; printf(\"Execution time: %f\\n\", cpu_time_used); }";

      getRewriter().InsertTextBefore(Body->getBeginLoc(), TimerStartCode);
      getRewriter().InsertTextAfter(Body->getEndLoc(), TimerEndCode);
      
      // Inject time.h and stdlib.h unconditionally
      FileID MainFileID = getASTContext().getSourceManager().getMainFileID();
      SourceLocation IncludeLoc = getASTContext().getSourceManager().getLocForStartOfFile(MainFileID);
      getRewriter().InsertTextAfter(IncludeLoc, "#include <time.h>\n#include <stdlib.h>\n");

      return true;
    }

    return false;
  }

private:
  std::vector<FunctionDecl *> TheFunctions;
};

static RegisterMutator<WrapFunctionWithTimer> M("u3.WrapFunctionWithTimer", "Wrap function execution with timer to measure its execution time.");
