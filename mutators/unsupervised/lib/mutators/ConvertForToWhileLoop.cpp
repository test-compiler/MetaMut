#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include "Mutator.h"
#include "MutatorManager.h"

using namespace clang;
using namespace ysmut;

class ConvertForToWhileLoop : public Mutator,
                      public clang::RecursiveASTVisitor<ConvertForToWhileLoop> {
public:
  using Mutator::Mutator;

  bool VisitForStmt(clang::ForStmt *FS) {
    TheLoops.push_back(FS);
    return true;
  }

  bool mutate() override {
    TraverseAST(getASTContext());

    if (TheLoops.empty())
      return false;

    ForStmt *forLoop = randElement(TheLoops);

    std::string init = forLoop->getInit() ? getSourceText(forLoop->getInit()) : "";
    std::string cond = forLoop->getCond() ? getSourceText(forLoop->getCond()) : "";
    std::string inc = forLoop->getInc() ? getSourceText(forLoop->getInc()) : "";
    std::string body = getSourceText(forLoop->getBody());

    if (!body.empty() && body.back() == ';')
        body.pop_back();

    std::string whileLoop = "{\n" + init + ";\n" + "while (" + cond + ") {\n";
    whileLoop += body + ";\n";
    whileLoop += inc.empty() ? "" : (inc + ";\n");
    whileLoop += "}\n}\n";

    getRewriter().ReplaceText(forLoop->getSourceRange(), whileLoop);

    return true;
  }

private:
  std::vector<clang::ForStmt *> TheLoops;

  std::string getSourceText(const Stmt *S) {
    SourceManager &SM = getASTContext().getSourceManager();
    LangOptions LO;
    SourceRange SR = S->getSourceRange();
    SR.setEnd(Lexer::getLocForEndOfToken(SR.getEnd(), 0, SM, LO));
    return Lexer::getSourceText(CharSourceRange::getTokenRange(SR), SM, LO).str();
  }
};

static RegisterMutator<ConvertForToWhileLoop> X(
    "ConvertForToWhileLoop", "Convert a 'for' loop into an equivalent 'while' loop.");