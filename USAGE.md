# PL2C Usage Guide

## Quick Start

### Basic Conversion

Convert a Prolog file to C and run it:

```bash
./pl2c.sh examples/simple.pl my_program
```

This will:
1. Convert `examples/simple.pl` to `my_program.c`
2. Compile it with gcc to create `my_program` executable
3. Run the program

### With Verification

Verify that the C version produces the same results as SWI-Prolog:

```bash
./pl2c.sh --verify examples/simple.pl my_program
```

## Writing Prolog for PL2C

### Supported Features

#### 1. Facts

```prolog
parent(tom, bob).
parent(bob, ann).
age(tom, 45).
```

Facts are compiled to C functions that always return true.

#### 2. Rules

```prolog
grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
```

Rules are compiled with explicit control flow for the body goals.

#### 3. Lists

```prolog
% List construction
list([1, 2, 3]).

% List pattern matching
head([H|_], H).
tail([_|T], T).

% List operations
append([], L, L).
append([H|T], L2, [H|R]) :- append(T, L2, R).
```

Lists use head/tail representation internally.

#### 4. Recursion

```prolog
factorial(0, 1).
factorial(N, F) :-
    N > 0,
    N1 is N - 1,
    factorial(N1, F1),
    F is N * F1.
```

Recursive predicates become recursive C functions.

#### 5. Cut (!)

```prolog
% Commit to first solution
max(X, Y, X) :- X >= Y, !.
max(_, Y, Y).
```

Cut removes choice points and prevents backtracking.

#### 6. Nondeterministic Predicates

```prolog
color(red).
color(green).
color(blue).

% Find first color
first_color(C) :- color(C), !.
```

Multiple clauses create choice points for backtracking.

#### 7. findall/3

```prolog
all_colors(Colors) :- findall(C, color(C), Colors).
```

Enumerates all solutions to a goal.

### ISO Standard Predicates (Supported)

The following ISO Prolog standard predicates are supported:

#### Type Checking
- `atom(X)`, `number(X)`, `integer(X)`, `var(X)`, `nonvar(X)`
- `compound(X)`, `atomic(X)`, `is_list(X)`, `ground(X)`, `callable(X)`

#### Arithmetic
- Operators: `+`, `-`, `*`, `/`, `//`, `mod`, `rem`, `^`, `**`
- Functions: `abs`, `sign`, `min`, `max`, `sqrt`, `floor`, `ceiling`, `round`, `truncate`
- Bitwise: `<<`, `>>`, `/\`, `\/`, `xor`
- Comparison: `>`, `<`, `>=`, `=<`, `=:=`, `=\=`

#### Lists
- `length(List, Length)`, `reverse(List, Reversed)`
- `nth0(N, List, Elem)`, `nth1(N, List, Elem)`, `last(List, Last)`
- `sort(List, Sorted)`, `msort(List, Sorted)`, `keysort(Pairs, Sorted)`

#### Atoms and Strings
- `atom_codes(Atom, Codes)`, `atom_chars(Atom, Chars)`
- `atom_length(Atom, Length)`, `atom_concat(Atom1, Atom2, Result)`
- `sub_atom(Atom, Before, Length, After, Sub)`

#### Term Manipulation
- `functor(Term, Functor, Arity)`, `arg(N, Term, Arg)`
- `Term =.. List` (univ), `copy_term(Term, Copy)`
- `compare(Order, Term1, Term2)`

#### Term Comparison
- `@<`, `@>`, `@=<`, `@>=` (standard term ordering)

#### I/O
- `write(Term)`, `format(Format, Args)`, `nl`, `tab(N)`
- `get_char(Char)`, `put_char(Char)`

#### Control
- `true`, `fail`, `!` (cut), `once(Goal)`, `ignore(Goal)`

#### Meta-Predicates (Simplified)
- `call(Goal)`, `call(Closure, Arg)`, `call(Closure, Arg1, Arg2)`
- `apply(Goal, Args)`

### Currently Unsupported or Limited

- Full meta-call interpretation (simplified implementation)
- `bagof/3`, `setof/3` (not fully implemented)
- `assert/retract` (dynamic predicates)
- `catch/3`, `throw/1` (exception handling)
- DCGs (Definite Clause Grammars)
- Constraints (CLP)
- Module system
- Floating point arithmetic (uses integer arithmetic)

## Examples

### Example 1: List Operations

```prolog
% member.pl
member(X, [X|_]).
member(X, [_|T]) :- member(X, T).

main :-
    member(2, [1, 2, 3]),
    write('Found 2 in list\n').
```

Compile and run:
```bash
./pl2c.sh member.pl member
```

### Example 2: Factorial

```prolog
% factorial.pl
factorial(0, 1).
factorial(N, F) :-
    N > 0,
    N1 is N - 1,
    factorial(N1, F1),
    F is N * F1.

main :-
    factorial(5, F),
    format('factorial(5) = ~w\n', [F]).
```

Compile and run:
```bash
./pl2c.sh factorial.pl factorial
```

### Example 3: Nondeterministic Search

```prolog
% colors.pl
color(red).
color(green).
color(blue).

main :-
    findall(C, color(C), Colors),
    format('All colors: ~w\n', [Colors]).
```

Compile and run:
```bash
./pl2c.sh colors.pl colors
```

### Example 4: Cut Behavior

```prolog
% max.pl
max(X, Y, X) :- X >= Y, !.
max(_, Y, Y).

main :-
    max(10, 5, M1),
    format('max(10, 5) = ~w\n', [M1]),
    max(3, 8, M2),
    format('max(3, 8) = ~w\n', [M2]).
```

Compile and run:
```bash
./pl2c.sh max.pl max
```

## Running Tests

### Full Test Suite

```bash
./run_tests.sh
```

This runs:
1. Unification algorithm tests (C unit tests)
2. Simple predicate tests
3. Nondeterministic predicate tests

### Individual Tests

Compile and run unification tests:
```bash
gcc -o tests/test_unification tests/test_unification.c -std=c99 -Wall
./tests/test_unification
```

Test a specific example:
```bash
./pl2c.sh examples/simple.pl test_simple
```

## Debugging

### Compilation Errors

If compilation fails, check:
1. Syntax errors in Prolog source
2. GCC version (requires C99 support)
3. Missing dependencies

### Runtime Errors

Common issues:
1. **Segmentation fault**: Usually from uninitialized variables or incorrect unification
2. **Infinite recursion**: Check base cases in recursive predicates
3. **Wrong results**: Verify clause order and cut placement

### Debug Output

Add debug printing in generated C code:

```c
printf("Debug: n=%d\n", n->data.int_val);
```

Recompile:
```bash
gcc -o my_program my_program.c -std=c99 -Wall -g
```

Run with debugger:
```bash
gdb ./my_program
```

## Performance Considerations

### Memory Usage

- Each term allocates memory
- Variables create bindings
- Choice points save state

For large problems, consider:
- Limiting recursion depth
- Using cut to remove unnecessary choice points
- Avoiding deep list structures

### Execution Speed

Factors affecting performance:
- Number of clauses per predicate
- Depth of recursion
- Amount of backtracking
- Complexity of unification

Optimization tips:
- Put most common cases first
- Use cut when deterministic
- Avoid unnecessary choice points
- Simplify recursive predicates

## Advanced Features

### Custom C Integration

Edit the generated C file to add custom functions:

```c
// Add custom function
bool my_custom_predicate(prolog_state_t* state, term_t* arg) {
    // Custom logic
    return true;
}

// Call from main
int main() {
    prolog_state_t state;
    init_state(&state);
    
    term_t* arg = create_atom("test");
    my_custom_predicate(&state, arg);
    
    free_state(&state);
    return 0;
}
```

### Extending the Runtime

Add new term types or operations in the generated C code:

```c
// Add string type
typedef enum {
    TERM_VAR, TERM_ATOM, TERM_INT,
    TERM_STRING,  // New type
    TERM_COMPOUND, TERM_LIST, TERM_NIL
} term_type_t;

// Add creation function
term_t* create_string(const char* str) {
    term_t* t = malloc(sizeof(term_t));
    t->type = TERM_STRING;
    t->data.string = strdup(str);
    return t;
}
```

## Troubleshooting

### Problem: "Conversion failed"

The Prolog-based translator requires SWI-Prolog. If not available, pl2c.sh falls back to a default C implementation with example predicates.

**Solution**: Either install SWI-Prolog or use the default generated code as a template.

### Problem: "Compilation warnings"

Common warnings:
- `strdup` implicit declaration: Add `#define _POSIX_C_SOURCE 200809L`
- Unused variables: Safe to ignore or remove them

**Solution**: Warnings are generally safe to ignore for working code.

### Problem: "Segmentation fault on large inputs"

Stack overflow from deep recursion.

**Solution**: 
- Increase stack size: `ulimit -s unlimited`
- Rewrite using iteration instead of recursion
- Use tail recursion when possible

### Problem: "Wrong results"

Possible causes:
- Incorrect clause order
- Missing cut
- Variable binding issues

**Solution**: 
- Trace execution manually
- Add debug output
- Compare with SWI-Prolog behavior

## Getting Help

1. Check documentation: README.md, ARCHITECTURE.md
2. Review examples in `examples/` directory
3. Run tests to see expected behavior
4. Check generated C code for issues

## Contributing

To extend PL2C:

1. Add new examples in `examples/`
2. Add tests in `tests/`
3. Update documentation
4. Submit pull request

See ARCHITECTURE.md for implementation details.
