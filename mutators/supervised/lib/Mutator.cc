#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Lex/Lexer.h>

#include "Mutator.h"
#include "MutatorManager.h"

unsigned Mutator::randIndex(unsigned n) {
  return getManager().randint(0, n - 1);
}

bool Mutator::randBool() { return getManager().randint(0, 1); }

clang::Rewriter &Mutator::getRewriter() const {
  return getManager().getRewriter();
}

clang::ASTContext &Mutator::getASTContext() const {
  return getManager().getASTContext();
}

clang::CompilerInstance &Mutator::getCompilerInstance() const {
  return getManager().getCompilerInstance();
}

MutationRange Mutator::getMutationRange() const {
  return getManager().getMutationRange();
}

class IdentifierCollector
    : public clang::RecursiveASTVisitor<IdentifierCollector> {
public:
  std::set<std::string> identifiers;
  IdentifierCollector() = default;

  explicit IdentifierCollector(std::set<std::string> &identifiers)
      : identifiers(identifiers) {}

  bool VisitNamedDecl(clang::NamedDecl *ND) {
    identifiers.insert(ND->getNameAsString());
    return true;
  }

  static std::set<std::string> collect(clang::ASTContext &Ctx) {
    IdentifierCollector collector;
    collector.TraverseAST(Ctx);
    return collector.identifiers;
  }
};

std::string Mutator::generateUniqueName(const std::string &baseName) {
  static std::set<std::string> allIdentifiers =
      IdentifierCollector::collect(getASTContext());
  clang::ASTContext &Context = getASTContext();

  int suffix = 1;
  std::string newName;
  while (true) {
    newName = baseName + "_" + std::to_string(suffix);
    if (allIdentifiers.find(newName) != allIdentifiers.end()) {
      ++suffix;
      continue;
    }
    allIdentifiers.insert(newName);
    break;
  }
  return newName;
}

clang::Decl *Mutator::getMostRecentTranslationUnitDecl(
    clang::FunctionDecl *FD) {
  for (clang::DeclContext *DC = FD; DC; DC = DC->getParent()) {
    if (DC->getParent() && DC->getParent()->isTranslationUnit()) {
      return clang::cast<clang::Decl>(DC);
    }
  }
  return nullptr;
}

clang::SourceLocation Mutator::findStringLocationFrom(
    clang::SourceLocation beginLoc, llvm::StringRef target) {
  // Get the SourceManager from the ASTContext
  clang::SourceManager &SM = getASTContext().getSourceManager();

  // Decompose the location to a file position
  std::pair<clang::FileID, unsigned> locInfo = SM.getDecomposedLoc(beginLoc);

  // Get the file buffer
  bool invalidTemp;
  llvm::StringRef fileBuffer = SM.getBufferData(locInfo.first, &invalidTemp);

  // Search for the string
  size_t foundPos = fileBuffer.find(target, locInfo.second);

  if (foundPos != llvm::StringRef::npos) {
    // String found. Create and return a new SourceLocation
    clang::SourceLocation foundLoc =
        SM.getLocForStartOfFile(locInfo.first).getLocWithOffset(foundPos);
    return foundLoc;
  } else {
    // String not found. Return an invalid SourceLocation
    return clang::SourceLocation();
  }
}

clang::SourceRange Mutator::findBracesRange(
    clang::SourceLocation scrhBeginLoc, char beginBrace, char endBrace) {
  // Get the SourceManager from the ASTContext
  clang::SourceManager &SM = getASTContext().getSourceManager();

  // Decompose the location to a file position
  std::pair<clang::FileID, unsigned> locInfo =
      SM.getDecomposedLoc(scrhBeginLoc);

  // Get the file buffer
  bool invalidTemp;
  llvm::StringRef fileBuffer = SM.getBufferData(locInfo.first, &invalidTemp);

  unsigned foundPos = locInfo.second, nBraces = 0;
  for (; foundPos < fileBuffer.size(); foundPos++)
    if (fileBuffer[foundPos] == beginBrace) break;

  auto beginLoc =
      SM.getLocForStartOfFile(locInfo.first).getLocWithOffset(foundPos);
  for (; foundPos < fileBuffer.size(); foundPos++) {
    if (fileBuffer[foundPos] == beginBrace)
      nBraces++;
    else if (fileBuffer[foundPos] == endBrace)
      nBraces--;
    if (nBraces == 0) break;
  }
  auto endLoc =
      SM.getLocForStartOfFile(locInfo.first).getLocWithOffset(foundPos);
  return clang::SourceRange(beginLoc, endLoc);
}

clang::SourceLocation Mutator::getLocForEndOfToken(clang::SourceLocation loc) {
  return clang::Lexer::getLocForEndOfToken(loc, 0,
      getASTContext().getSourceManager(), getASTContext().getLangOpts());
}

std::string Mutator::formatAsDecl(
    clang::QualType ty, const std::string &placeholder) {
  std::string declStr;
  llvm::raw_string_ostream ostream(declStr);
  ty.print(ostream, getASTContext().getPrintingPolicy(), placeholder);
  ostream.flush();
  return ostream.str();
}

clang::SourceRange Mutator::getExpansionRange(clang::SourceRange range) {
  auto &SM = getASTContext().getSourceManager();
  clang::SourceLocation begin = range.getBegin();
  while (begin.isMacroID()) begin = SM.getExpansionLoc(begin);
  clang::SourceLocation end = range.getEnd();
  while (end.isMacroID()) end = SM.getExpansionLoc(end);
  return clang::SourceRange(begin, end);
}

clang::SourceLocation Mutator::getDeclEndLoc(clang::Decl *decl) {
  auto loc = findStringLocationFrom(decl->getEndLoc(), ";");
  return loc.getLocWithOffset(1);
}
