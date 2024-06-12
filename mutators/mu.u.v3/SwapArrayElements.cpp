#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>

#include "MutatorManager.h"

using namespace clang;

class SwapArrayElements : public Mutator,
                          public clang::RecursiveASTVisitor<SwapArrayElements> {
public:
    using Mutator::Mutator;
  
    bool VisitInitListExpr(InitListExpr* ILE) {
        if (ILE->getNumInits() > 1) // Array should have more than one element
            TheArrays.push_back(ILE);
        return true;
    }

    bool mutate() override {
        TraverseAST(getASTContext());

        if (TheArrays.empty()) 
            return false;

        // Pick a random array
        InitListExpr* array = randElement(TheArrays);

        // Pick two random elements within the array
        unsigned idx1 = randIndex(array->getNumInits());
        unsigned idx2 = randIndex(array->getNumInits());
        while (idx2 == idx1) // Ensure we have two distinct indices
            idx2 = randIndex(array->getNumInits());

        // Extract the source text for the two elements
        std::string elem1 = getSourceText(array->getInit(idx1));
        std::string elem2 = getSourceText(array->getInit(idx2));

        // Perform the replacements
        getRewriter().ReplaceText(array->getInit(idx1)->getSourceRange(), elem2);
        getRewriter().ReplaceText(array->getInit(idx2)->getSourceRange(), elem1);

        return true;
    }

private:
    std::vector<InitListExpr*> TheArrays;
};

static RegisterMutator<SwapArrayElements> M("u3.SwapArrayElements", "This mutator selects an array initialization and swaps two random elements within it, effectively altering the order of array elements");
