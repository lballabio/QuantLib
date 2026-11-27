#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tidy;

namespace {

void emitAutoFix(ClangTidyCheck &Check, const VarDecl *VD, const CallExpr *CE,
                 const SourceManager &SM, const LangOptions &LangOpts,
                 StringRef Message) {
  TypeSourceInfo *TSI = VD->getTypeSourceInfo();
  if (!TSI)
    return;

  auto Diag = Check.diag(VD->getLocation(), Message);
  Diag << FixItHint::CreateReplacement(TSI->getTypeLoc().getSourceRange(),
                                       "auto");

  if (VD->getInitStyle() == VarDecl::CallInit) {
    SourceLocation AfterName =
        Lexer::getLocForEndOfToken(VD->getLocation(), 0, SM, LangOpts);
    SourceLocation RParen = VD->getEndLoc();

    if (AfterName.isValid() && RParen.isValid()) {
      Diag << FixItHint::CreateReplacement(
          CharSourceRange::getCharRange(AfterName, CE->getBeginLoc()), " = ");
      Diag << FixItHint::CreateRemoval(
          CharSourceRange::getTokenRange(RParen, RParen));
    }
  }
}

bool isValidSingleNonMacroVar(const VarDecl *VD, const CallExpr *CE,
                              ASTContext &Ctx) {
  if (!VD || !CE)
    return false;
  if (VD->getBeginLoc().isMacroID() || CE->getBeginLoc().isMacroID())
    return false;

  auto Parents = Ctx.getParents(*VD);
  if (!Parents.empty()) {
    if (const auto *DS = Parents[0].get<DeclStmt>()) {
      if (!DS->isSingleDecl())
        return false;
    }
  }
  return true;
}

// Check 1: ext::shared_ptr<Foo> x = ext::make_shared<Foo>(...)
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
    if (!isValidSingleNonMacroVar(VD, CE, *Result.Context))
      return;

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

    emitAutoFix(*this, VD, CE, *Result.SourceManager,
                Result.Context->getLangOpts(),
                "use 'auto' when initializing with 'make_shared'");
  }
};

// Check 2: ext::shared_ptr<Foo> x = flatRate(...) / flatVol(...)
class UseAutoFlatRateVolCheck : public ClangTidyCheck {
public:
  UseAutoFlatRateVolCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(MatchFinder *Finder) override {
    Finder->addMatcher(
        varDecl(
            unless(hasType(autoType())),
            hasType(hasCanonicalType(recordType(hasDeclaration(
                classTemplateSpecializationDecl(hasName("::std::shared_ptr")))))),
            hasInitializer(ignoringImplicit(
                callExpr(callee(functionDecl(hasAnyName("flatRate", "flatVol"))))
                    .bind("call"))))
            .bind("var"),
        this);
  }

  void check(const MatchFinder::MatchResult &Result) override {
    const auto *VD = Result.Nodes.getNodeAs<VarDecl>("var");
    const auto *CE = Result.Nodes.getNodeAs<CallExpr>("call");
    if (!isValidSingleNonMacroVar(VD, CE, *Result.Context))
      return;

    const auto *FuncDecl = CE->getDirectCallee();
    if (!FuncDecl)
      return;

    if (VD->getType().getCanonicalType().getUnqualifiedType() !=
        FuncDecl->getReturnType().getCanonicalType().getUnqualifiedType())
      return;

    emitAutoFix(*this, VD, CE, *Result.SourceManager,
                Result.Context->getLangOpts(),
                "use 'auto' when initializing with 'flatRate' or 'flatVol'");
  }
};

// Check 3: ext::shared_ptr<Foo> / Handle<Foo> / RelinkableHandle<Foo> default arg -> = {}
class DefaultSharedPtrBracesCheck : public ClangTidyCheck {
public:
  DefaultSharedPtrBracesCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(MatchFinder *Finder) override {
    auto TargetType = hasCanonicalType(recordType(hasDeclaration(
        classTemplateSpecializationDecl(hasAnyName(
            "::std::shared_ptr", "::QuantLib::Handle",
            "::QuantLib::RelinkableHandle")))));

    Finder->addMatcher(
        parmVarDecl(
            anyOf(hasType(TargetType),
                  hasType(referenceType(pointee(TargetType)))),
            hasDefaultArgument())
            .bind("param"),
        this);
  }

  void check(const MatchFinder::MatchResult &Result) override {
    const auto *PVD = Result.Nodes.getNodeAs<ParmVarDecl>("param");
    if (!PVD || !PVD->hasDefaultArg() || PVD->hasUninstantiatedDefaultArg())
      return;

    const Expr *DefaultArg = PVD->getDefaultArg();
    if (!DefaultArg)
      return;

    const auto *CtorExpr =
        dyn_cast<CXXConstructExpr>(DefaultArg->IgnoreImplicit());
    if (!CtorExpr || CtorExpr->getNumArgs() != 0 ||
        CtorExpr->isListInitialization())
      return;

    SourceRange ArgRange = DefaultArg->getSourceRange();
    if (ArgRange.isInvalid() || ArgRange.getBegin().isMacroID() ||
        ArgRange.getEnd().isMacroID())
      return;

    diag(ArgRange.getBegin(),
         "use '{}' instead of explicit default constructor")
        << FixItHint::CreateReplacement(ArgRange, "{}");
  }
};

class CustomTidyModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<UseAutoMakeSharedCheck>(
        "custom-use-auto-make-shared");
    CheckFactories.registerCheck<UseAutoFlatRateVolCheck>(
        "custom-use-auto-flat-rate-vol");
    CheckFactories.registerCheck<DefaultSharedPtrBracesCheck>(
        "custom-default-shared-ptr-braces");
  }
};

} // namespace

static ClangTidyModuleRegistry::Add<CustomTidyModule>
    X("custom-module", "Adds custom refactoring checks.");
