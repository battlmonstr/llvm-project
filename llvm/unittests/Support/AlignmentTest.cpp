//=== - llvm/unittest/Support/Alignment.cpp - Alignment utility tests -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Alignment.h"
#include "gtest/gtest.h"

#include <vector>

using namespace llvm;

namespace {

std::vector<uint64_t> getValidAlignments() {
  std::vector<uint64_t> Out;
  for (size_t Shift = 0; Shift < 64; ++Shift)
    Out.push_back(1ULL << Shift);
  return Out;
}

TEST(AlignmentTest, AlignDefaultCTor) { EXPECT_EQ(Align().value(), 1ULL); }

TEST(AlignmentTest, MaybeAlignDefaultCTor) {
  EXPECT_FALSE(MaybeAlign().hasValue());
}

TEST(AlignmentTest, ValidCTors) {
  for (uint64_t Value : getValidAlignments()) {
    EXPECT_EQ(Align(Value).value(), Value);
    EXPECT_EQ((*MaybeAlign(Value)).value(), Value);
  }
}

TEST(AlignmentTest, CheckMaybeAlignHasValue) {
  EXPECT_TRUE(MaybeAlign(1));
  EXPECT_TRUE(MaybeAlign(1).hasValue());
  EXPECT_FALSE(MaybeAlign(0));
  EXPECT_FALSE(MaybeAlign(0).hasValue());
  EXPECT_FALSE(MaybeAlign());
  EXPECT_FALSE(MaybeAlign().hasValue());
}

TEST(AlignmentTest, Division) {
  for (uint64_t Value : getValidAlignments()) {
    if (Value > 1) {
      EXPECT_EQ(Align(Value) / 2, Value / 2);
      EXPECT_EQ(MaybeAlign(Value) / 2, Value / 2);
    }
  }
  EXPECT_EQ(MaybeAlign(0) / 2, MaybeAlign(0));
}

TEST(AlignmentTest, AlignTo) {
  struct {
    uint64_t alignment;
    uint64_t offset;
    uint64_t rounded;
  } kTests[] = {
      // MaybeAlign
      {0, 0, 0},
      {0, 1, 1},
      {0, 5, 5},
      // MaybeAlign / Align
      {1, 0, 0},
      {1, 1, 1},
      {1, 5, 5},
      {2, 0, 0},
      {2, 1, 2},
      {2, 2, 2},
      {2, 7, 8},
      {2, 16, 16},
      {4, 0, 0},
      {4, 1, 4},
      {4, 4, 4},
      {4, 6, 8},
  };
  for (const auto &T : kTests) {
    MaybeAlign A(T.alignment);
    // Test MaybeAlign
    EXPECT_EQ(alignTo(T.offset, A), T.rounded);
    // Test Align
    if (A) {
      EXPECT_EQ(alignTo(T.offset, A.getValue()), T.rounded);
    }
  }
}

TEST(AlignmentTest, Log2) {
  for (uint64_t Value : getValidAlignments()) {
    EXPECT_EQ(Log2(Align(Value)), Log2_64(Value));
    EXPECT_EQ(Log2(MaybeAlign(Value)), Log2_64(Value));
  }
}

TEST(AlignmentTest, MinAlign) {
  struct {
    uint64_t A;
    uint64_t B;
    uint64_t MinAlign;
  } kTests[] = {
      // MaybeAlign
      {0, 0, 0},
      {0, 8, 8},
      {2, 0, 2},
      // MaybeAlign / Align
      {1, 2, 1},
      {8, 4, 4},
  };
  for (const auto &T : kTests) {
    EXPECT_EQ(commonAlignment(MaybeAlign(T.A), MaybeAlign(T.B)), T.MinAlign);
    EXPECT_EQ(MinAlign(T.A, T.B), T.MinAlign);
    if (T.A) {
      EXPECT_EQ(commonAlignment(Align(T.A), MaybeAlign(T.B)), T.MinAlign);
    }
    if (T.B) {
      EXPECT_EQ(commonAlignment(MaybeAlign(T.A), Align(T.B)), T.MinAlign);
    }
    if (T.A && T.B) {
      EXPECT_EQ(commonAlignment(Align(T.A), Align(T.B)), T.MinAlign);
    }
  }
}

TEST(AlignmentTest, Encode_Decode) {
  for (uint64_t Value : getValidAlignments()) {
    {
      Align Actual(Value);
      Align Expected = decodeMaybeAlign(encode(Actual)).getValue();
      EXPECT_EQ(Expected, Actual);
    }
    {
      MaybeAlign Actual(Value);
      MaybeAlign Expected = decodeMaybeAlign(encode(Actual));
      EXPECT_EQ(Expected, Actual);
    }
  }
  MaybeAlign Actual(0);
  MaybeAlign Expected = decodeMaybeAlign(encode(Actual));
  EXPECT_EQ(Expected, Actual);
}

TEST(AlignmentTest, isAligned) {
  struct {
    uint64_t alignment;
    uint64_t offset;
    bool isAligned;
  } kTests[] = {
      // MaybeAlign / Align
      {1, 0, true},  {1, 1, true},  {1, 5, true},  {2, 0, true},
      {2, 1, false}, {2, 2, true},  {2, 7, false}, {2, 16, true},
      {4, 0, true},  {4, 1, false}, {4, 4, true},  {4, 6, false},
  };
  for (const auto &T : kTests) {
    MaybeAlign A(T.alignment);
    // Test MaybeAlign
    EXPECT_EQ(isAligned(A, T.offset), T.isAligned);
    // Test Align
    if (A) {
      EXPECT_EQ(isAligned(A.getValue(), T.offset), T.isAligned);
    }
  }
}

TEST(AlignmentTest, AlignComparisons) {
  std::vector<uint64_t> ValidAlignments = getValidAlignments();
  std::sort(ValidAlignments.begin(), ValidAlignments.end());
  for (size_t I = 1; I < ValidAlignments.size(); ++I) {
    assert(I >= 1);
    const Align A(ValidAlignments[I - 1]);
    const Align B(ValidAlignments[I]);
    EXPECT_EQ(A, A);
    EXPECT_NE(A, B);
    EXPECT_LT(A, B);
    EXPECT_GT(B, A);
    EXPECT_LE(A, B);
    EXPECT_GE(B, A);
    EXPECT_LE(A, A);
    EXPECT_GE(A, A);

    EXPECT_EQ(A, A.value());
    EXPECT_NE(A, B.value());
    EXPECT_LT(A, B.value());
    EXPECT_GT(B, A.value());
    EXPECT_LE(A, B.value());
    EXPECT_GE(B, A.value());
    EXPECT_LE(A, A.value());
    EXPECT_GE(A, A.value());

    EXPECT_EQ(std::max(A, B), B);
    EXPECT_EQ(std::min(A, B), A);

    const MaybeAlign MA(ValidAlignments[I - 1]);
    const MaybeAlign MB(ValidAlignments[I]);
    EXPECT_EQ(MA, MA);
    EXPECT_NE(MA, MB);
    EXPECT_LT(MA, MB);
    EXPECT_GT(MB, MA);
    EXPECT_LE(MA, MB);
    EXPECT_GE(MB, MA);
    EXPECT_LE(MA, MA);
    EXPECT_GE(MA, MA);

    EXPECT_EQ(MA, MA ? (*MA).value() : 0);
    EXPECT_NE(MA, MB ? (*MB).value() : 0);
    EXPECT_LT(MA, MB ? (*MB).value() : 0);
    EXPECT_GT(MB, MA ? (*MA).value() : 0);
    EXPECT_LE(MA, MB ? (*MB).value() : 0);
    EXPECT_GE(MB, MA ? (*MA).value() : 0);
    EXPECT_LE(MA, MA ? (*MA).value() : 0);
    EXPECT_GE(MA, MA ? (*MA).value() : 0);

    EXPECT_EQ(std::max(A, B), B);
    EXPECT_EQ(std::min(A, B), A);
  }
}

TEST(AlignmentTest, AssumeAligned) {
  EXPECT_EQ(assumeAligned(0), Align(1));
  EXPECT_EQ(assumeAligned(0), Align());
  EXPECT_EQ(assumeAligned(1), Align(1));
  EXPECT_EQ(assumeAligned(1), Align());
}

// Death tests reply on assert which is disabled in release mode.
#ifndef NDEBUG

// We use a subset of valid alignments for DEATH_TESTs as they are particularly
// slow.
std::vector<uint64_t> getValidAlignmentsForDeathTest() {
  return {1, 1ULL << 31, 1ULL << 63};
}

std::vector<uint64_t> getNonPowerOfTwo() { return {3, 10, 15}; }

TEST(AlignmentDeathTest, Log2) {
  EXPECT_DEATH(Log2(MaybeAlign(0)), ".* should be defined");
}

TEST(AlignmentDeathTest, CantConvertUnsetMaybe) {
  EXPECT_DEATH((MaybeAlign(0).getValue()), ".*");
}

TEST(AlignmentDeathTest, Division) {
  EXPECT_DEATH(Align(1) / 2, "Can't halve byte alignment");
  EXPECT_DEATH(MaybeAlign(1) / 2, "Can't halve byte alignment");

  EXPECT_DEATH(Align(8) / 0, "Divisor must be positive and a power of 2");
  EXPECT_DEATH(Align(8) / 3, "Divisor must be positive and a power of 2");
}

TEST(AlignmentDeathTest, InvalidCTors) {
  EXPECT_DEATH((Align(0)), "Value must not be 0");
  for (uint64_t Value : getNonPowerOfTwo()) {
    EXPECT_DEATH((Align(Value)), "Alignment is not a power of 2");
    EXPECT_DEATH((MaybeAlign(Value)), "Alignment is not 0 or a power of 2");
  }
}

TEST(AlignmentDeathTest, ComparisonsWithZero) {
  for (uint64_t Value : getValidAlignmentsForDeathTest()) {
    EXPECT_DEATH((void)(Align(Value) == 0), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) != 0), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) >= 0), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) <= 0), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) > 0), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) < 0), ".* should be defined");
  }
}

TEST(AlignmentDeathTest, CompareMaybeAlignToZero) {
  for (uint64_t Value : getValidAlignmentsForDeathTest()) {
    // MaybeAlign is allowed to be == or != 0
    (void)(MaybeAlign(Value) == 0);
    (void)(MaybeAlign(Value) != 0);
    EXPECT_DEATH((void)(MaybeAlign(Value) >= 0), ".* should be defined");
    EXPECT_DEATH((void)(MaybeAlign(Value) <= 0), ".* should be defined");
    EXPECT_DEATH((void)(MaybeAlign(Value) > 0), ".* should be defined");
    EXPECT_DEATH((void)(MaybeAlign(Value) < 0), ".* should be defined");
  }
}

TEST(AlignmentDeathTest, CompareAlignToUndefMaybeAlign) {
  for (uint64_t Value : getValidAlignmentsForDeathTest()) {
    EXPECT_DEATH((void)(Align(Value) == MaybeAlign(0)), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) != MaybeAlign(0)), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) >= MaybeAlign(0)), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) <= MaybeAlign(0)), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) > MaybeAlign(0)), ".* should be defined");
    EXPECT_DEATH((void)(Align(Value) < MaybeAlign(0)), ".* should be defined");
  }
}

#endif // NDEBUG

} // end anonymous namespace
