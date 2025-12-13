# PL2C Architecture

## Overview

The PL2C compiler translates Prolog programs into equivalent C code that preserves the semantic behavior of Prolog, including:
- Unification
- Backtracking
- Nondeterminism
- Cut operations
- Solution enumeration (findall)

## System Architecture

```
┌─────────────────┐
│  Prolog Source  │
│    (.pl file)   │
└────────┬────────┘
         │
         ↓
┌─────────────────┐
│  pl2c.pl        │  ← Prolog-based translator
│  (Parser &      │     (optional, falls back to default)
│   Translator)   │
└────────┬────────┘
         │
         ↓
┌─────────────────┐
│  C Source Code  │  ← Generated C with runtime
│    (.c file)    │
└────────┬────────┘
         │
         ↓
┌─────────────────┐
│   GCC Compiler  │
└────────┬────────┘
         │
         ↓
┌─────────────────┐
│  Executable     │
│  Binary         │
└─────────────────┘
```

## Core Components

### 1. Term Representation

All Prolog terms are represented using a tagged union structure in C:

```c
typedef struct term {
    term_type_t type;      // TERM_VAR, TERM_ATOM, TERM_INT, etc.
    union {
        int var_id;                    // For variables
        char* atom;                    // For atoms
        int int_val;                   // For integers
        struct { ... } compound;       // For compound terms
        struct { head, tail } list;    // For lists
    } data;
} term_t;
```

This structure allows representing all Prolog data types uniformly.

### 2. Execution State

The execution state tracks all information needed for Prolog's execution model:

```c
typedef struct {
    choice_point_t* choice_stack;  // Stack for backtracking
    bindings_t bindings;            // Variable bindings
    int cut_level;                  // Cut barrier management
    bool failed;                    // Failure flag
    int next_var_id;                // Variable ID generator
} prolog_state_t;
```

Key aspects:
- **choice_stack**: Maintains choice points for backtracking
- **bindings**: Maps variable IDs to their bound values
- **cut_level**: Tracks cut barriers for proper cut semantics
- **next_var_id**: Ensures unique variable IDs across recursion

### 3. Unification Algorithm

The unification engine is the heart of the system:

```c
bool unify(prolog_state_t* state, term_t* t1, term_t* t2);
```

Process:
1. Dereference both terms through bindings
2. If either is a variable, bind it to the other
3. For ground terms, check structural equality
4. For compound terms, recursively unify arguments
5. For lists, unify heads and tails separately

The occurs check (preventing infinite structures) is implicit through the dereferencing mechanism.

### 4. Choice Points and Backtracking

Choice points save execution state at decision points:

```c
typedef struct choice_point {
    int predicate_id;                // Which predicate
    int clause_index;                // Which clause to try next
    bindings_t saved_bindings;       // Bindings to restore
    struct choice_point* prev;       // Stack pointer
} choice_point_t;
```

Operations:
- `push_choice_point()`: Save state before trying alternatives
- `pop_choice_point()`: Restore state and try next alternative
- Used automatically in disjunctions and nondeterministic predicates

### 5. Cut Implementation

Cut (!/0) is implemented using cut barriers:

```c
void perform_cut(prolog_state_t* state) {
    // Remove all choice points at or above current cut level
    while (state->choice_stack && 
           state->choice_stack->predicate_id >= state->cut_level) {
        // Pop and free choice point
    }
}
```

Cut semantics:
- Removes choice points from the current clause
- Prevents backtracking to alternative clauses
- Does not affect choice points from parent goals

### 6. Clause Selection

Each Prolog predicate becomes a C function:

```prolog
% Prolog
member(X, [X|_]).
member(X, [_|T]) :- member(X, T).
```

Becomes:

```c
// C
bool member_2(prolog_state_t* state, term_t* elem, term_t* list) {
    term_t* list_deref = deref(state, list);
    
    // Try first clause
    if (list_deref->type == TERM_LIST) {
        if (unify(state, elem, list_deref->data.list.head)) {
            return true;
        }
        // Backtrack implicitly
        state->failed = false;
    }
    
    // Try second clause
    if (list_deref->type == TERM_LIST) {
        return member_2(state, elem, list_deref->data.list.tail);
    }
    
    state->failed = true;
    return false;
}
```

### 7. Nondeterministic Execution

Nondeterministic predicates use resumable loops via choice points:

```prolog
% Multiple solutions
color(red).
color(green).
color(blue).
```

Implementation strategy:
- Each clause becomes a case in a loop
- Choice points track which clause to try next
- On backtracking, resume from saved choice point
- Continue until all alternatives exhausted

### 8. findall/3 Implementation

Solution enumeration creates an isolated execution context:

```c
// Pseudocode for findall(Template, Goal, Result)
{
    term_t** solutions = NULL;
    int count = 0;
    prolog_state_t isolated_state;
    init_state(&isolated_state);
    
    // Force backtracking to enumerate all solutions
    while (execute_goal(&isolated_state, Goal)) {
        solutions[count++] = instantiate(Template, &isolated_state);
        force_backtrack(&isolated_state);
    }
    
    // Build result list from collected solutions
    Result = build_list(solutions, count);
}
```

### 9. Conjunction and Disjunction

Conjunctions (A, B):
- Execute A
- If A succeeds, execute B
- If B fails, backtrack to A
- Managed implicitly through control flow

Disjunctions (A ; B):
- Create choice point before A
- Try A first
- If A fails, restore state and try B
- Requires explicit choice point management

## Memory Management

### Allocation Strategy

1. **Terms**: Heap-allocated when created
2. **Bindings**: Dynamically grown array
3. **Choice points**: Stack-allocated structures with heap data

### Cleanup

- Choice points freed when popped
- Bindings cleared with state
- Terms: Currently not freed (potential improvement)

## Code Generation

### Translation Phases

1. **Parse Prolog**: Read clauses from source file
2. **Analyze Structure**: Extract predicates, arities, clauses
3. **Generate Header**: Type definitions and prototypes
4. **Generate Functions**: One per predicate/arity pair
5. **Generate Runtime**: Unification, choice points, etc.
6. **Generate Main**: Test harness and entry point

### Optimizations

Current:
- Direct variable binding (no trail)
- Inline unification for simple cases

Potential:
- Last call optimization
- Determinism analysis
- Index first argument
- Compile-time partial evaluation

## Limitations

### Implemented ISO Standard Features

- **Type checking predicates**: `atom/1`, `number/1`, `integer/1`, `var/1`, `nonvar/1`, `compound/1`, `is_list/1`, `atomic/1`, `ground/1`, `callable/1`
- **Term comparison**: `@</2`, `@>/2`, `@=</2`, `@>=/2`, `compare/3` with standard term ordering
- **List predicates**: `length/2`, `nth0/3`, `nth1/3`, `last/2`, `reverse/2`, `sort/2`, `msort/2`, `keysort/2`
- **Atom manipulation**: `atom_codes/2`, `atom_chars/2`, `atom_length/2`, `atom_concat/3`, `sub_atom/5`
- **Term manipulation**: `functor/3`, `arg/3`, `=../2` (univ), `copy_term/2`
- **Arithmetic**: Full `is/2` evaluation with operators: `+`, `-`, `*`, `/`, `//`, `mod`, `rem`, `^`, `**`, `abs`, `sign`, `min`, `max`, `sqrt`, `floor`, `ceiling`, `round`, `truncate`, bitwise ops (`<<`, `>>`, `/\`, `\/`, `xor`)
- **I/O predicates**: `write/1`, `format/2`, `nl/0`, `tab/1`, `get_char/1`, `put_char/1`
- **Comparison operators**: `>`, `<`, `>=`, `=<`, `=`, `==`, `\=`, `\==`
- **Meta-predicates**: Simplified `call/1`, `call/2`, `call/3`, `apply/2`
- **Control predicates**: `true/0`, `once/1`, `ignore/1`, `!/0` (cut)
- **Sorting**: `sort/2` (removes duplicates), `msort/2` (keeps duplicates), `keysort/2` (sorts Key-Value pairs)

### Not Yet Implemented

- Full meta-call interpretation (current `call/1` etc. are simplified)
- `bagof/3`, `setof/3` (solution collection with free variables)
- `assert/retract` (dynamic predicates)
- `catch/3`, `throw/1` (exception handling)
- `read/1`, `get/1`, `put/1` (character I/O beyond basic support)
- Definite clause grammars (DCGs)
- Module system
- Constraints (CLP)
- Floating point arithmetic (currently uses integer arithmetic)

### Semantic Differences

- Variable IDs are integers, not atoms
- No occurs check optimization
- Memory not automatically reclaimed
- Limited error handling

## Performance Characteristics

### Time Complexity

- Unification: O(n) where n = term size
- Backtracking: O(k) where k = choice point depth
- findall: O(n × m) where n = solutions, m = goal complexity

### Space Complexity

- Bindings: O(v) where v = number of variables
- Choice points: O(d) where d = recursion depth
- Terms: O(t) where t = total term size

## Testing Strategy

### Unit Tests

- Unification correctness
- Variable binding and dereferencing
- Choice point management
- Cut behavior

### Integration Tests

- Simple predicates (facts, rules)
- Recursive predicates (list operations)
- Nondeterministic predicates (multiple solutions)
- Complex control flow (cut, disjunction)

### Equivalence Testing

Compare output with SWI-Prolog for:
- Deterministic queries
- Solution enumeration
- Edge cases (empty lists, etc.)

## Future Enhancements

1. **Compiler Optimizations**
   - Determinism analysis
   - Indexing on first argument
   - Partial evaluation

2. **Extended Features**
   - Arithmetic evaluation
   - More built-in predicates
   - DCG support
   - Module system

3. **Better Code Generation**
   - More idiomatic C code
   - Better variable naming
   - Inline small predicates

4. **Performance Improvements**
   - Memory pooling
   - Trail-based backtracking
   - Instruction-level optimizations

## References

- Warren's Abstract Machine (WAM)
- "The Implementation of Prolog" by Patrice Boizumault
- "The Art of Prolog" by Sterling and Shapiro
- SWI-Prolog documentation
