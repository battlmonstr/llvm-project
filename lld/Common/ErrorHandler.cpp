//===- ErrorHandler.cpp ---------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lld/Common/ErrorHandler.h"

#include "lld/Common/Threads.h"

#include "llvm/ADT/Twine.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include <mutex>
#include <regex>

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#endif

using namespace llvm;
using namespace lld;

// The functions defined in this file can be called from multiple threads,
// but outs() or errs() are not thread-safe. We protect them using a mutex.
static std::mutex mu;

// Prints "\n" or does nothing, depending on Msg contents of
// the previous call of this function.
static void newline(raw_ostream *errorOS, const Twine &msg) {
  // True if the previous error message contained "\n".
  // We want to separate multi-line error messages with a newline.
  static bool flag;

  if (flag)
    *errorOS << "\n";
  flag = StringRef(msg.str()).contains('\n');
}

ErrorHandler &lld::errorHandler() {
  static ErrorHandler handler;
  return handler;
}

void lld::exitLld(int val) {
  // Delete any temporary file, while keeping the memory mapping open.
  if (errorHandler().outputBuffer)
    errorHandler().outputBuffer->discard();

  // Dealloc/destroy ManagedStatic variables before calling
  // _exit(). In a non-LTO build, this is a nop. In an LTO
  // build allows us to get the output of -time-passes.
  llvm_shutdown();

  outs().flush();
  errs().flush();
  _exit(val);
}

void lld::diagnosticHandler(const DiagnosticInfo &di) {
  SmallString<128> s;
  raw_svector_ostream os(s);
  DiagnosticPrinterRawOStream dp(os);
  di.print(dp);
  switch (di.getSeverity()) {
  case DS_Error:
    error(s);
    break;
  case DS_Warning:
    warn(s);
    break;
  case DS_Remark:
  case DS_Note:
    message(s);
    break;
  }
}

void lld::checkError(Error e) {
  handleAllErrors(std::move(e),
                  [&](ErrorInfoBase &eib) { error(eib.message()); });
}

// This is for --vs-diagnostics.
//
// Normally, lld's error message starts with argv[0]. Therefore, it usually
// looks like this:
//
//   ld.lld: error: ...
//
// This error message style is unfortunately unfriendly to Visual Studio
// IDE. VS interprets the first word of the first line as an error location
// and make it clickable, thus "ld.lld" in the above message would become a
// clickable text. When you click it, VS opens "ld.lld" executable file with
// a binary editor.
//
// As a workaround, we print out an error location instead of "ld.lld" if
// lld is running in VS diagnostics mode. As a result, error message will
// look like this:
//
//   src/foo.c(35): error: ...
//
// This function returns an error location string. An error location is
// extracted from an error message using regexps.
static std::string getLocation(std::string msg, std::string defaultMsg) {
  static std::vector<std::regex> Regexes{
      std::regex(R"(^undefined (?:\S+ )?symbol:.*\n>>> referenced by (\S+):(\d+)\n.*)"),
      std::regex(R"(^undefined symbol:.*\n>>> referenced by (.*):)"),
      std::regex(
          R"(^duplicate symbol: .*\n>>> defined in (\S+)\n>>> defined in.*)"),
      std::regex(
          R"(^duplicate symbol: .*\n>>> defined at (\S+):(\d+).*)"),
      std::regex(
          R"(.*\n>>> defined in .*\n>>> referenced by (\S+):(\d+))"),
      std::regex(R"((\S+):(\d+): unclosed quote)"),
  };

  std::smatch Match;
  for (std::regex &Re : Regexes) {
    if (std::regex_search(msg, Match, Re)) {
      return Match.size() > 2 ? Match.str(1) + "(" + Match.str(2) + ")"
                              : Match.str(1);
    }
  }
  return defaultMsg;
}

void ErrorHandler::printHeader(StringRef s, raw_ostream::Colors c,
                               const Twine &msg) {

  if (vsDiagnostics) {
    // A Visual Studio-style error message starts with an error location.
    // If a location cannot be extracted then we default to LogName.
    *errorOS << getLocation(msg.str(), logName) << ": ";
  } else {
    *errorOS << logName << ": ";
  }

  if (colorDiagnostics) {
    errorOS->changeColor(c, true);
    *errorOS << s;
    errorOS->resetColor();
  } else {
    *errorOS << s;
  }
}

void ErrorHandler::log(const Twine &msg) {
  if (verbose) {
    std::lock_guard<std::mutex> lock(mu);
    *errorOS << logName << ": " << msg << "\n";
  }
}

void ErrorHandler::message(const Twine &msg) {
  std::lock_guard<std::mutex> lock(mu);
  outs() << msg << "\n";
  outs().flush();
}

void ErrorHandler::warn(const Twine &msg) {
  if (fatalWarnings) {
    error(msg);
    return;
  }

  std::lock_guard<std::mutex> lock(mu);
  newline(errorOS, msg);
  printHeader("warning: ", raw_ostream::MAGENTA, msg);
  *errorOS << msg << "\n";
}

void ErrorHandler::printErrorMsg(const Twine &msg) {
  newline(errorOS, msg);
  printHeader("error: ", raw_ostream::RED, msg);
  *errorOS << msg << "\n";
}

void ErrorHandler::printError(const Twine &msg) {
  if (vsDiagnostics) {
    static std::regex reDuplicateSymbol(
        R"(^(duplicate symbol: .*))"
        R"((\n>>> defined at \S+:\d+\n>>>.*))"
        R"((\n>>> defined at \S+:\d+\n>>>.*))");
    std::string msgStr = msg.str();
    std::smatch match;
    if (std::regex_match(msgStr, match, reDuplicateSymbol)) {
      printErrorMsg(match.str(1) + match.str(2));
      printErrorMsg(match.str(1) + match.str(3));
      return;
    }
  }
  printErrorMsg(msg);
}

void ErrorHandler::error(const Twine &msg) {
  std::lock_guard<std::mutex> lock(mu);

  if (errorLimit == 0 || errorCount < errorLimit) {
    printError(msg);
  } else if (errorCount == errorLimit) {
    newline(errorOS, msg);
    printHeader("error: ", raw_ostream::RED, msg);
    *errorOS << errorLimitExceededMsg << "\n";
    if (exitEarly)
      exitLld(1);
  }

  ++errorCount;
}

void ErrorHandler::fatal(const Twine &msg) {
  error(msg);
  exitLld(1);
}
