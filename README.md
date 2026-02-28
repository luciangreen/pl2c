# pl2c - Prolog to C Compiler

A comprehensive Prolog-to-C compiler that converts Prolog code into equivalent C code with explicit control flow structures.

## Limitations and Notes

- `findall/3` and `assert/1` do not necessarily work.
- There have been no benchmarks performed on pl2c because of lack of specific predicates. Given the the good algorithm and light design, the optimisation to minimise the logical if-then C structures to conditions unchecked elsewhere has been implemented, approaching the speed of C.
- The author designed pl2c to complement [Prolog to Starlog](https://github.com/luciangreen/prolog_to_starlog), so one can specify and run Starlog code quickly, but it's probably better to run the converted Prolog code in SWI-Prolog.
- [SSI](https://github.com/luciangreen/SSI), [List Prolog Interpreter](https://github.com/luciangreen/listprologinterpreter), and this should have a findall and prime (argument number indicator) converter to predicates for efficiency and ability to be converted to C.
- `assert/1` should pass around non-code as variables, but code needs to be recompiled, slowing down the interpreter.

## Features

- **Complete Prolog-to-C Translation**: Converts Prolog predicates into C functions with explicit loops and if-then statements
- **Unification Engine**: Full implementation of the unification algorithm with variable binding and dereferencing
- **Clause Selection**: Intelligent clause selection logic with pattern matching
- **Backtracking Support**: Choice-point stack management for proper failure propagation
- **Nondeterministic Predicates**: Compiles nondeterministic predicates to resumable loops with saved execution state
- **Conjunction Handling**: Proper combination of choice points for complex goal sequences
- **findall/3 Implementation**: Solution enumeration in isolated generator contexts (not working)
- **Cut (!/0) Support**: Global cut barriers that prune local choice points and prevent clause fallback
- **Verification Tool**: Single command to convert, compile, and verify equivalence with SWI-Prolog
- **ISO Prolog Standard Predicates**: Comprehensive support for ISO standard built-in predicates including:
  - Type checking: `atom/1`, `number/1`, `integer/1`, `var/1`, `nonvar/1`, `compound/1`, `is_list/1`, `atomic/1`, `ground/1`, `callable/1`
  - Term comparison: `@</2`, `@>/2`, `@=</2`, `@>=/2`, `compare/3`
  - List operations: `length/2`, `nth0/3`, `nth1/3`, `last/2`, `reverse/2`, `sort/2`, `msort/2`, `keysort/2`
  - Atom/string manipulation: `atom_codes/2`, `atom_chars/2`, `atom_length/2`, `atom_concat/3`, `sub_atom/5`
  - Term manipulation: `functor/3`, `arg/3`, `=../2`, `copy_term/2`
  - Arithmetic: Extended support for `abs`, `sign`, `min`, `max`, `sqrt`, `floor`, `ceiling`, `round`, `truncate`, power (`^`, `**`), bitwise operators
  - I/O: `nl/0`, `tab/1`, `get_char/1`, `put_char/1`, `write/1`, `format/2`
  - Meta-predicates: `call/1`, `call/2`, `call/3`, `apply/2`
  - Control: `true/0`, `once/1`, `ignore/1`

For a complete list of supported ISO Prolog standard features, see [ISO_FEATURES.md](ISO_FEATURES.md).

## Architecture

The compiler consists of several key components:

### 1. Term Representation
Terms are represented in C using a tagged union structure supporting:
- Variables (with unique IDs)
- Atoms (strings)
- Integers
- Compound terms (functor + arguments)
- Lists (head/tail pairs)
- Empty list (nil)

### 2. Unification Algorithm
The unification engine handles:
- Variable binding and dereferencing
- Occurs check prevention
- Structural matching of compound terms
- List unification with proper head/tail decomposition

### 3. Choice Point Stack
For backtracking and nondeterministic execution:
- Saves variable bindings at decision points
- Enables failure-driven loops
- Supports cut operations via level management

### 4. Cut Implementation
Global cut barriers:
- Track cut levels during execution
- Prune choice points at or above current level
- Prevent fallback to alternative clauses after cut

### 5. findall/3 Generator (not working)
Solution enumeration:
- Isolated execution state for goal
- Collects all solutions via backtracking
- Returns solutions as a list term

## Installation

No installation required. Just clone the repository:

```bash
git clone https://github.com/luciangreen/pl2c.git
cd pl2c
```

## Usage

### Basic Conversion

Convert a Prolog file to C and compile it:

```bash
./pl2c.sh examples/simple.pl output_name
```

This will:
1. Convert the Prolog code to C
2. Compile the C code with gcc
3. Run the resulting executable

### With Verification

Verify that the compiled C code produces the same results as SWI-Prolog:

```bash
./pl2c.sh --verify examples/simple.pl output_name
```

### Running Tests

Run the complete test suite:

```bash
./run_tests.sh
```

This will:
1. Run C-level unit tests for unification
2. Compile and test example programs
3. Verify output correctness

## Examples

### Simple Facts and Rules

```prolog
% facts
parent(tom, bob).
parent(bob, ann).

% rule
grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
```

### Nondeterministic Predicates

```prolog
% Multiple solutions via backtracking
color(red).
color(green).
color(blue).

% Enumerate all solutions
all_colors(Colors) :- findall(C, color(C), Colors). % not working
```

### Cut Example

```prolog
% Cut prevents backtracking
max(X, Y, X) :- X >= Y, !.
max(_, Y, Y).
```

### List Operations

```prolog
% Member predicate
member(X, [X|_]).
member(X, [_|T]) :- member(X, T).

% Append predicate
append([], L, L).
append([H|T1], L2, [H|T3]) :- append(T1, L2, T3).
```

## Generated C Code Structure

The compiler generates C code with:

1. **Header Section**: Type definitions and function prototypes
2. **Predicate Functions**: Each Prolog predicate becomes a C function
   - Takes `prolog_state_t*` for execution context
   - Takes term pointers for arguments
   - Returns `bool` for success/failure
3. **Runtime Library**: Unification, choice points, term creation
4. **Main Function**: Test harness and example calls

## Testing

The test suite includes:

1. **Unit Tests** (`tests/test_unification.c`)
   - Unification of atoms, integers, variables
   - List and compound term unification
   - Variable dereferencing

2. **Integration Tests**
   - Simple predicates (facts, rules)
   - Nondeterministic execution
   - Cut behavior
   - findall/3 operation (not working)

3. **Equivalence Verification**
   - Compare output with SWI-Prolog
   - Verify semantic equivalence

## Technical Details

### Execution Model

The compiled C code follows Prolog's execution model:

1. **Clause Selection**: Try clauses in order until one succeeds
2. **Unification**: Match arguments with clause heads
3. **Goal Execution**: Process body goals left-to-right
4. **Backtracking**: On failure, restore state and try next clause
5. **Cut**: Remove choice points and commit to current branch

### Memory Management

- Terms are heap-allocated with explicit lifecycle
- Choice points save and restore variable bindings
- State cleanup on completion or error

### Limitations

Current implementation limitations:
- No dynamic predicates (assert/retract) - predicates must be defined at compile time
- No DCG (Definite Clause Grammar) support
- No module system
- Meta-call predicates (`call/1`, etc.) have simplified implementations
- No constraint logic programming (CLP)
- Limited exception handling (no `catch/3`, `throw/1`)
- Solution collection (`bagof/3`, `setof/3`) not fully implemented

## Contributing

Contributions welcome! Areas for improvement:
- More built-in predicates
- Optimization passes
- Better error messages
- DCG support
- Module system

## License

See LICENSE file for details.

## References

- Warren's Abstract Machine (WAM)
- SWI-Prolog documentation
- "The Art of Prolog" by Sterling and Shapiro
