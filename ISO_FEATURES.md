# ISO Prolog and SWI-Prolog Features in PL2C

This document lists the ISO Prolog standard and SWI-Prolog features that have been implemented in the PL2C compiler.

## Type Checking Predicates (ISO Standard)

These predicates check the type of terms:

- `atom(Term)` - Succeeds if Term is an atom
- `number(Term)` - Succeeds if Term is a number (currently only integers)
- `integer(Term)` - Succeeds if Term is an integer
- `var(Term)` - Succeeds if Term is an unbound variable
- `nonvar(Term)` - Succeeds if Term is not a variable
- `compound(Term)` - Succeeds if Term is a compound term or list
- `atomic(Term)` - Succeeds if Term is atomic (atom, number, or nil)
- `is_list(Term)` - Succeeds if Term is a proper list (ending with nil)
- `ground(Term)` - Succeeds if Term contains no unbound variables
- `callable(Term)` - Succeeds if Term can be called as a goal (atom or compound)

## Arithmetic Evaluation (ISO Standard)

The `is/2` predicate evaluates arithmetic expressions with the following operators and functions:

### Binary Operators
- `+` - Addition
- `-` - Subtraction
- `*` - Multiplication
- `/` - Division
- `//` - Integer division
- `mod` - Modulo
- `rem` - Remainder
- `^` - Exponentiation
- `**` - Exponentiation (alternative)
- `<<` - Left bit shift
- `>>` - Right bit shift
- `/\` - Bitwise AND
- `\/` - Bitwise OR
- `xor` - Bitwise XOR

### Unary Functions
- `abs(X)` - Absolute value
- `sign(X)` - Sign of X (-1, 0, or 1)
- `-(X)` - Negation
- `+(X)` - Identity
- `sqrt(X)` - Square root
- `floor(X)` - Floor function
- `ceiling(X)` - Ceiling function
- `round(X)` - Round to nearest integer
- `truncate(X)` - Truncate towards zero

### Binary Functions
- `min(X, Y)` - Minimum of X and Y
- `max(X, Y)` - Maximum of X and Y

### Comparison Operators
- `>` - Greater than
- `<` - Less than
- `>=` - Greater than or equal
- `=<` - Less than or equal
- `=:=` - Arithmetic equality
- `=\=` - Arithmetic inequality

## Term Comparison (ISO Standard)

Standard term ordering comparison:

- `@<` - Term less than
- `@>` - Term greater than
- `@=<` - Term less than or equal
- `@>=` - Term greater than or equal
- `compare(Order, Term1, Term2)` - Unifies Order with `<`, `>`, or `=`

Standard term ordering: `var < number < atom < compound`

## List Predicates (ISO/SWI)

- `length(List, Length)` - Relates list to its length
- `nth0(N, List, Elem)` - Zero-based list indexing
- `nth1(N, List, Elem)` - One-based list indexing
- `last(List, Last)` - Gets the last element of a list
- `reverse(List, Reversed)` - Reverses a list
- `sort(List, Sorted)` - Sorts list, removing duplicates
- `msort(List, Sorted)` - Sorts list, keeping duplicates
- `keysort(Pairs, Sorted)` - Sorts list of Key-Value pairs by Key

## Atom and String Predicates (ISO Standard)

- `atom_codes(Atom, Codes)` - Converts between atom and list of character codes
- `atom_chars(Atom, Chars)` - Converts between atom and list of character atoms
- `atom_length(Atom, Length)` - Gets the length of an atom
- `atom_concat(Atom1, Atom2, Result)` - Concatenates two atoms
- `sub_atom(Atom, Before, Length, After, Sub)` - Extracts substring from atom

## Term Manipulation (ISO Standard)

- `functor(Term, Functor, Arity)` - Relates term to its functor and arity
- `arg(N, Term, Arg)` - Gets the Nth argument of a compound term (1-indexed)
- `=..(Term, List)` - Univ operator - converts between term and list [functor|args]
- `copy_term(Term, Copy)` - Creates a copy of a term with fresh variables

## I/O Predicates (ISO Standard)

- `write(Term)` - Writes a term to output
- `format(Format, Args)` - Formatted output with ~w placeholders
- `nl` - Writes a newline
- `tab(N)` - Writes N spaces
- `get_char(Char)` - Reads a single character
- `put_char(Char)` - Writes a single character

## Unification and Comparison (ISO Standard)

- `=(Term1, Term2)` - Unification
- `\=(Term1, Term2)` - Not unifiable
- `==(Term1, Term2)` - Structural equality (no unification)
- `\==(Term1, Term2)` - Structural inequality

## Control Predicates (ISO Standard)

- `true` - Always succeeds
- `fail` - Always fails
- `!` - Cut - removes choice points
- `once(Goal)` - Executes goal once, removing choice points on success (simplified)
- `ignore(Goal)` - Always succeeds, ignoring goal failure

## Meta-Call Predicates (ISO/SWI)

Simplified implementations for meta-calling:

- `call(Goal)` - Calls a goal dynamically
- `call(Closure, Arg)` - Calls closure with additional argument
- `call(Closure, Arg1, Arg2)` - Calls closure with two additional arguments
- `apply(Goal, Args)` - Applies goal with arguments from list

Note: These are simplified implementations that handle basic cases. Full meta-call interpretation would require a complete goal interpreter.

## Solution Collection (not working)

- `findall(Template, Goal, Result)` - Collects all solutions (already implemented)
- `bagof(Template, Goal, Bag)` - Placeholder (not fully implemented)
- `setof(Template, Goal, Set)` - Placeholder (not fully implemented)

## Implementation Status

### Fully Implemented
All predicates listed above are implemented with the following notes:
- Arithmetic is integer-based (no floating point support yet)
- Meta-call predicates have simplified implementations
- Type system supports: variables, atoms, integers, compound terms, lists, nil

### Partially Implemented
- `once/1`, `ignore/1` - Simplified implementations
- `call/1`, `call/2`, `call/3`, `apply/2` - Basic functionality without full goal interpretation
- `sub_atom/5` - Only works when positions are fully specified

### Not Implemented
- `bagof/3`, `setof/3` - Complex solution collection with free variables
- `assert/1`, `retract/1` - Dynamic predicates
- `catch/3`, `throw/1` - Exception handling
- `read/1` - Reading terms from input
- Floating point arithmetic
- DCGs (Definite Clause Grammars)
- Module system
- Constraints (CLP)

## Compliance with ISO Prolog Standard

This implementation follows the ISO Prolog standard (ISO/IEC 13211-1:1995) for the predicates listed above, with the following deviations:

1. **Integer-only arithmetic**: The implementation uses integer arithmetic throughout. Floating point operations are not yet supported.

2. **Simplified meta-call**: Meta-call predicates (`call/1`, etc.) have simplified implementations that handle basic cases but do not provide full goal interpretation.

3. **Limited dynamic predicates**: No support for `assert/retract` or other database modification predicates.

4. **No exception handling**: `catch/3` and `throw/1` are not implemented.

5. **Simplified I/O**: Only basic character and term I/O is supported.

## Testing

Test cases for these features can be found in:
- `examples/iso_features.pl` - Comprehensive tests for all ISO predicates
- `examples/simple.pl` - Basic usage examples
- `examples/comprehensive.pl` - Complex examples demonstrating multiple features
- `tests/test_unification.c` - Low-level unification tests

## References

- ISO/IEC 13211-1:1995 - Information technology — Programming languages — Prolog — Part 1: General core
- SWI-Prolog Manual - https://www.swi-prolog.org/pldoc/doc_for?object=manual
- "The Art of Prolog" by Sterling and Shapiro
- "Programming in Prolog" by Clocksin and Mellish

## Future Work

Planned additions to achieve fuller ISO compliance:
- Floating point arithmetic support
- Full meta-call interpretation
- Exception handling (`catch/3`, `throw/1`)
- Complete solution collection (`bagof/3`, `setof/3`)
- More comprehensive I/O predicates
- Dynamic predicate support (if feasible in compiled context)
