// In ToggleBitfield.h
namespace ysmut {

class ToggleBitfield : public Mutator,
                       public clang::RecursiveASTVisitor<ToggleBitfield> {
public:
  using Mutator::Mutator;
  bool mutate() override;
  bool VisitFieldDecl(clang::FieldDecl *FD);
  bool VisitBinaryOperator(clang::BinaryOperator *BO);

private:
  std::vector<clang::FieldDecl *> TheFields;
  std::set<clang::FieldDecl *> FieldUsedAsLValue;
};

} // namespace ysmut
