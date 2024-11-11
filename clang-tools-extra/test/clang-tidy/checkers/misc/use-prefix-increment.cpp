// RUN: %check_clang_tidy %s misc-use-prefix-increment %t

struct Value {
  int v{0};
  int operator++(int) const { return v; }
  int operator++() const { return v; }
};

static bool cond = true;

void basic_yes(int x) {
  x++;
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  ++x;{{$}}
  { x++; }
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  { ++x; }{{$}}
  for (;; x++) {}
  // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  for (;; ++x) {}{{$}}
  for (;;) { x++; }
  // CHECK-MESSAGES: :[[@LINE-1]]:14: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  for (;;) { ++x; }{{$}}
  for (;;) x++;
  // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  for (;;) ++x;{{$}}
  if (cond) { x++; }
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  if (cond) { ++x; }{{$}}
  if (cond) x++;
  // CHECK-MESSAGES: :[[@LINE-1]]:13: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  if (cond) ++x;{{$}}
  while (cond) { x++; }
  // CHECK-MESSAGES: :[[@LINE-1]]:18: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  while (cond) { ++x; }{{$}}
  while (cond) x++;
  // CHECK-MESSAGES: :[[@LINE-1]]:16: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  while (cond) ++x;{{$}}
  do { x++; } while (cond);
  // CHECK-MESSAGES: :[[@LINE-1]]:8: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  do { ++x; } while (cond);{{$}}
  do x++; while (cond);
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  do ++x; while (cond);{{$}}
}

void overloaded_yes(Value x) {
  x++;
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  ++x;{{$}}
  { x++; }
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  { ++x; }{{$}}
  for (;; x++) {}
  // CHECK-MESSAGES: :[[@LINE-1]]:11: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  for (;; ++x) {}{{$}}
  for (;;) { x++; }
  // CHECK-MESSAGES: :[[@LINE-1]]:14: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  for (;;) { ++x; }{{$}}
  for (;;) x++;
  // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  for (;;) ++x;{{$}}
  if (cond) { x++; }
  // CHECK-MESSAGES: :[[@LINE-1]]:15: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  if (cond) { ++x; }{{$}}
  if (cond) x++;
  // CHECK-MESSAGES: :[[@LINE-1]]:13: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  if (cond) ++x;{{$}}
  while (cond) { x++; }
  // CHECK-MESSAGES: :[[@LINE-1]]:18: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  while (cond) { ++x; }{{$}}
  while (cond) x++;
  // CHECK-MESSAGES: :[[@LINE-1]]:16: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  while (cond) ++x;{{$}}
  do { x++; } while (cond);
  // CHECK-MESSAGES: :[[@LINE-1]]:8: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  do { ++x; } while (cond);{{$}}
  do x++; while (cond);
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: Use the prefix form (++i/--i) unless you need postfix semantics. See https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement [misc-use-prefix-increment]
  // CHECK-FIXES: {{^}}  do ++x; while (cond);{{$}}
}

void basic_no(int x) {
  x = x++;
  { x = x++; }
  for (; x++; ) {}
  if (x++) {}
  while (x++) {}
  do {} while (x++);
  int y{x++};
}

void overloaded_no(Value x) {
  x.v = x++;
  { x.v = x++; }
  for (; x++; ) {}
  if (x++) {}
  while (x++) {}
  do {} while (x++);
  int y{x++};
}
