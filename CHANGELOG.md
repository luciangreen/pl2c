# Changelog

All notable changes to the pl2c project will be documented in this file.

## [Unreleased] - ISO Prolog Features

### Added

#### Type Checking Predicates (ISO Standard)
- `atom/1` - Test if term is an atom
- `number/1` - Test if term is a number
- `integer/1` - Test if term is an integer
- `var/1` - Test if term is an unbound variable
- `nonvar/1` - Test if term is not a variable
- `compound/1` - Test if term is a compound term
- `atomic/1` - Test if term is atomic
- `is_list/1` - Test if term is a proper list
- `ground/1` - Test if term contains no variables
- `callable/1` - Test if term can be called as a goal

#### Arithmetic Functions (ISO Standard)
- Extended `is/2` evaluation with:
  - Binary operators: `+`, `-`, `*`, `/`, `//`, `mod`, `rem`, `^`, `**`, `<<`, `>>`, `/\`, `\/`, `xor`
  - Unary functions: `abs`, `sign`, `-`, `+`, `sqrt`, `floor`, `ceiling`, `round`, `truncate`
  - Binary functions: `min`, `max`

#### Term Comparison (ISO Standard)
- `@</2` - Standard term less than
- `@>/2` - Standard term greater than
- `@=</2` - Standard term less than or equal
- `@>=/2` - Standard term greater than or equal
- `compare/3` - Three-way term comparison

#### List Predicates (ISO/SWI-Prolog)
- `length/2` - List length
- `nth0/3` - Zero-based list indexing
- `nth1/3` - One-based list indexing
- `last/2` - Last element of list
- `reverse/2` - Reverse a list
- `sort/2` - Sort list removing duplicates
- `msort/2` - Sort list keeping duplicates
- `keysort/2` - Sort Key-Value pairs by key

#### Atom/String Predicates (ISO Standard)
- `atom_codes/2` - Convert between atom and character codes
- `atom_chars/2` - Convert between atom and character list
- `atom_length/2` - Length of an atom
- `atom_concat/3` - Concatenate atoms
- `sub_atom/5` - Extract substring from atom

#### Term Manipulation (ISO Standard)
- `functor/3` - Term functor and arity
- `arg/3` - Extract argument from compound term
- `=../2` - Univ operator (term to list conversion)
- `copy_term/2` - Copy term with fresh variables

#### I/O Predicates (ISO Standard)
- `nl/0` - Write newline
- `tab/1` - Write spaces
- `get_char/1` - Read single character
- `put_char/1` - Write single character
- Improved `write/1` and `format/2` implementations

#### Control Predicates (ISO Standard)
- `true/0` - Always succeeds
- `once/1` - Execute goal once (simplified implementation)
- `ignore/1` - Always succeed (simplified implementation)

#### Meta-Call Predicates (ISO/SWI-Prolog)
- `call/1` - Dynamic goal calling (simplified)
- `call/2` - Call with one extra argument (simplified)
- `call/3` - Call with two extra arguments (simplified)
- `apply/2` - Apply goal with argument list (simplified)

#### Documentation
- Added `ISO_FEATURES.md` - Comprehensive documentation of all ISO features
- Updated `README.md` with feature list and limitations
- Updated `ARCHITECTURE.md` with implementation details
- Updated `USAGE.md` with usage examples for new predicates
- Added `examples/iso_features.pl` - Test file demonstrating new features
- Added `CHANGELOG.md` - This file

### Changed
- Enhanced arithmetic evaluation in `is/2` to support many more operators
- Improved term comparison with full standard term ordering
- Updated documentation to reflect ISO standard compliance

### Fixed
- Fixed `copy_term/2` to properly handle recursive copying
- Fixed `keysort/2` to compare only keys, not entire Key-Value pairs
- Added proper comments to `once/1` and `ignore/1` explaining their limitations

### Notes
- Arithmetic is integer-based only (no floating point support yet)
- Meta-call predicates have simplified implementations
- `bagof/3` and `setof/3` are placeholders (not fully implemented)
- Some predicates like `once/1` and `ignore/1` do not execute goals (documented limitation)

## [Previous] - Initial Implementation

### Added
- Basic Prolog to C compilation
- Unification algorithm
- Backtracking with choice points
- Cut operation
- findall/3 implementation (not working)
- Basic arithmetic operators
- List and compound term support
- Example programs
- Test suite
