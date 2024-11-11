//===--- UsePrefixIncrementCheck.cpp - clang-tidy -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UsePrefixIncrementCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/FixIt.h"

using namespace clang::ast_matchers;

namespace clang::tidy::misc {

template <typename TMatcher>
auto sourceOnly(TMatcher&& Matcher) {
  return traverse(TK_IgnoreUnlessSpelledInSource, std::move(Matcher));
}

void UsePrefixIncrementCheck::registerMatchers(MatchFinder *Finder) {
  auto BasicOpMatcher = unaryOperator(hasAnyOperatorName("++", "--")).bind("Basic");
  auto OverloadedOpMatcher = cxxOperatorCallExpr(hasAnyOverloadedOperatorName("++", "--")).bind("Overloaded");
  auto OpMatcher = anyOf(BasicOpMatcher, OverloadedOpMatcher);

  Finder->addMatcher(sourceOnly(compoundStmt(hasAnySubstatement(OpMatcher))), this);
  Finder->addMatcher(sourceOnly(ifStmt(hasThen(OpMatcher))), this);
  Finder->addMatcher(sourceOnly(forStmt(hasIncrement(OpMatcher))), this);
  Finder->addMatcher(sourceOnly(forStmt(hasBody(OpMatcher))), this);
  Finder->addMatcher(sourceOnly(whileStmt(hasBody(OpMatcher))), this);
  Finder->addMatcher(sourceOnly(doStmt(hasBody(OpMatcher))), this);
}

void UsePrefixIncrementCheck::check(const MatchFinder::MatchResult &Result) {
  const clang::Expr *Expr = nullptr;
  const clang::Expr *SubExpr = nullptr;
  bool IsInc = false;

  auto *Basic = Result.Nodes.getNodeAs<UnaryOperator>("Basic");
  if (Basic) {
    if (Basic->isPrefix()) return;
    Expr = Basic;
    SubExpr = Basic->getSubExpr();
    IsInc = Basic->isIncrementOp();
  }

  auto *Overloaded = Result.Nodes.getNodeAs<CXXOperatorCallExpr>("Overloaded");
  if (Overloaded) {
    if (Overloaded->getNumArgs() != 2) return; // isPrefix
    Expr = Overloaded;
    SubExpr = Overloaded->getArg(0);
    IsInc = Overloaded->getOperator() == clang::OO_PlusPlus;
  }

  auto Location = Expr->getBeginLoc();
  auto Replacement = llvm::Twine{IsInc ? "++" : "--"} + tooling::fixit::getText(*SubExpr, *Result.Context);

  diag(
    Location,
    "Use the prefix form (++i/--i) unless you need postfix semantics. "
    "See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement")
    << FixItHint::CreateReplacement(Expr->getSourceRange(), Replacement.str());
  // diag(Location, "convert to the prefix form", DiagnosticIDs::Note);
}

} // namespace clang::tidy::misc
