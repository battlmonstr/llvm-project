.. title:: clang-tidy - misc-use-prefix-increment

misc-use-prefix-increment
=========================

Replaces ``i++`` with ``++i`` (``i--`` with ``--i``) if postfix semantics is not needed.

See <https://google.github.io/styleguide/cppguide.html#Preincrement_and_Predecrement>
