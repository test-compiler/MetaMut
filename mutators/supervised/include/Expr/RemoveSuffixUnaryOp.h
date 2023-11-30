#ifndef YSMUT_EXPR_REMOVESUFFIXUNARYOP_H
#define YSMUT_EXPR_REMOVESUFFIXUNARYOP_H

#include <clang/AST/RecursiveASTVisitor.h>
#include "Mutator.h"

namespace ysmut {

class RemoveSuffixUnaryOp : public Mutator,
                            public clang::RecursiveASTVisitor<RemoveSuffixUnaryOp> {
public:
    using Mutator::Mutator;
    bool mutate() override;
    bool VisitUnaryOperator(clang::UnaryOperator *UO);

private:
    std::vector<clang::UnaryOperator *> TheUnaryOps;
};

} // namespace ysmut

#endif
