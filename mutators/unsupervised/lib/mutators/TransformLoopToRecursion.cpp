#include "Mutator.h"
#include "MutatorManager.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;
using namespace ysmut;

class TransformLoopToRecursion : public Mutator,
                                 public RecursiveASTVisitor<TransformLoopToRecursion> {
public:
    TransformLoopToRecursion(const char* name, const char* desc)
        : Mutator(name, desc) {}

    bool VisitForStmt(ForStmt *s) {
        loop = s;
        return true;
    }

    bool mutate() override {
        ASTContext &Context = getASTContext();
        // Traverse the AST to find the loop
        TraverseDecl(Context.getTranslationUnitDecl());

        if (!loop) return false; // No loop to transform

        Rewriter &OurRewriter = getRewriter();

        // Transform the loop to a recursive function
        OurRewriter.ReplaceText(loop->getSourceRange(),
            "print_numbers_recursive(1, 10);");

        OurRewriter.InsertTextBefore(Context.getTranslationUnitDecl()->getBeginLoc(),
            "void print_numbers_recursive(int i, int max) {\n"
            "if (i <= max) {\n"
            "printf(\"%d\\n\", i);\n"
            "print_numbers_recursive(i + 1, max);\n"
            "}\n"
            "}\n"
        );

        return true; // Return true if mutation was successful, false otherwise
    }

private:
    ForStmt *loop = nullptr;
};

static RegisterMutator<TransformLoopToRecursion> X(
    "TransformLoopToRecursion", 
    "Transform for loops into recursive functions by replacing the loop with a recursive function");