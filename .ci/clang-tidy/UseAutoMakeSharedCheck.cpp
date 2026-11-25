#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tidy;

namespace {

class UseAutoMakeSharedCheck : public ClangTidyCheck {
public:
  UseAutoMakeSharedCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(MatchFinder *Finder) override {
    Finder->addMatcher(
        varDecl(
            unless(hasType(autoType())),
            hasType(hasCanonicalType(recordType(hasDeclaration(
                classTemplateSpecializationDecl(hasName("::std::shared_ptr")))))),
            hasInitializer(ignoringImplicit(
                callExpr(callee(functionDecl(hasName("::std::make_shared"))))
                    .bind("call"))))
            .bind("var"),
        this);
  }

  void check(const MatchFinder::MatchResult &Result) override {
    const auto *VD = Result.Nodes.getNodeAs<VarDecl>("var");
    const auto *CE = Result.Nodes.getNodeAs<CallExpr>("call");
    if (!VD || !CE)
      return;

    if (VD->getBeginLoc().isMacroID() || CE->getBeginLoc().isMacroID())
      return;

    // Skip multi-variable declarations like `ext::shared_ptr<Foo> a = ..., b = ...;`
    auto Parents = Result.Context->getParents(*VD);
    if (!Parents.empty()) {
      if (const auto *DS = Parents[0].get<DeclStmt>()) {
        if (!DS->isSingleDecl())
          return;
      }
    }

    // Ensure shared_ptr<T> and make_shared<T> use the exact same type T
    const auto *PtrSpec = VD->getType()->getAs<TemplateSpecializationType>();
    const auto *FuncDecl = CE->getDirectCallee();
    if (!PtrSpec || !FuncDecl || !FuncDecl->getTemplateSpecializationArgs())
      return;

    if (PtrSpec->template_arguments().empty() ||
        FuncDecl->getTemplateSpecializationArgs()->size() == 0)
      return;

    QualType PtrArgType = PtrSpec->template_arguments()[0].getAsType();
    QualType FuncArgType =
        FuncDecl->getTemplateSpecializationArgs()->get(0).getAsType();

    if (PtrArgType.getCanonicalType() != FuncArgType.getCanonicalType())
      return;

    TypeSourceInfo *TSI = VD->getTypeSourceInfo();
    if (!TSI)
      return;

    auto Diag = diag(VD->getLocation(),
                     "use 'auto' when initializing with 'make_shared'");

    // 1. Replace `ext::shared_ptr<Foo>` with `auto`
    Diag << FixItHint::CreateReplacement(TSI->getTypeLoc().getSourceRange(),
                                         "auto");

    // 2. If direct-initialized `x(ext::make_shared<Foo>(...))`, convert `(...)` to ` = ...`
    if (VD->getInitStyle() == VarDecl::CallInit) {
      SourceLocation AfterName = Lexer::getLocForEndOfToken(
          VD->getLocation(), 0, *Result.SourceManager, getLangOpts());
      SourceLocation RParen = VD->getEndLoc();

      if (AfterName.isValid() && RParen.isValid()) {
        // Replace everything between `x` and `ext::make_shared` (i.e. `(`) with ` = `
        Diag << FixItHint::CreateReplacement(
            CharSourceRange::getCharRange(AfterName, CE->getBeginLoc()),
            " = ");
        // Remove the trailing `)`
        Diag << FixItHint::CreateRemoval(
            CharSourceRange::getTokenRange(RParen, RParen));
      }
    }
  }
};

class CustomTidyModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<UseAutoMakeSharedCheck>(
        "custom-use-auto-make-shared");
  }
};

} // namespace

static ClangTidyModuleRegistry::Add<CustomTidyModule>
    X("custom-module", "Adds custom refactoring checks.");
