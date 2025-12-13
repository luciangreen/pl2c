#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

/* Prolog term representation */
typedef enum {
    TERM_VAR,
    TERM_ATOM,
    TERM_INT,
    TERM_COMPOUND,
    TERM_LIST,
    TERM_NIL
} term_type_t;

typedef struct term {
    term_type_t type;
    union {
        int var_id;
        char* atom;
        int int_val;
        struct {
            char* functor;
            int arity;
            struct term** args;
        } compound;
        struct {
            struct term* head;
            struct term* tail;
        } list;
    } data;
} term_t;

/* Unification and variable binding */
typedef struct {
    int var_id;
    term_t* value;
} binding_t;

typedef struct {
    binding_t* bindings;
    int size;
    int capacity;
} bindings_t;

/* Choice point for backtracking */
typedef struct choice_point {
    int predicate_id;
    int clause_index;
    bindings_t saved_bindings;
    struct choice_point* prev;
} choice_point_t;

/* Global state */
typedef struct {
    choice_point_t* choice_stack;
    bindings_t bindings;
    int cut_level;
    bool failed;
    int next_var_id;
} prolog_state_t;

/* Function prototypes */
term_t* create_var(int id);
term_t* create_atom(const char* name);
term_t* create_int(int val);
term_t* create_compound(const char* functor, int arity, term_t** args);
term_t* create_list(term_t* head, term_t* tail);
term_t* create_nil();
bool unify(prolog_state_t* state, term_t* t1, term_t* t2);
term_t* deref(prolog_state_t* state, term_t* term);
void push_choice_point(prolog_state_t* state, int pred_id, int clause_idx);
bool pop_choice_point(prolog_state_t* state);
void init_state(prolog_state_t* state);
void free_state(prolog_state_t* state);
void perform_cut(prolog_state_t* state);
int eval_arithmetic(prolog_state_t* state, term_t* expr);

/* Built-in operators */
bool gt_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool lt_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool gte_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool lte_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool eq_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool neq_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool eqeq_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool neqeq_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool is_2(prolog_state_t* state, term_t* arg1, term_t* arg2);

/* Built-in I/O predicates */
void print_term(term_t* term);
bool write_1(prolog_state_t* state, term_t* arg1);
bool format_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool nl_0(prolog_state_t* state);
bool tab_1(prolog_state_t* state, term_t* arg1);
bool get_char_1(prolog_state_t* state, term_t* arg1);
bool put_char_1(prolog_state_t* state, term_t* arg1);

/* Type checking predicates (ISO) */
bool atom_1(prolog_state_t* state, term_t* arg1);
bool number_1(prolog_state_t* state, term_t* arg1);
bool integer_1(prolog_state_t* state, term_t* arg1);
bool var_1(prolog_state_t* state, term_t* arg1);
bool nonvar_1(prolog_state_t* state, term_t* arg1);
bool compound_1(prolog_state_t* state, term_t* arg1);
bool atomic_1(prolog_state_t* state, term_t* arg1);
bool is_list_1(prolog_state_t* state, term_t* arg1);
bool ground_1(prolog_state_t* state, term_t* arg1);
bool callable_1(prolog_state_t* state, term_t* arg1);

/* Term comparison predicates (ISO) */
bool term_lt_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool term_gt_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool term_lte_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool term_gte_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool compare_3(prolog_state_t* state, term_t* order, term_t* arg1, term_t* arg2);

/* List predicates (ISO/SWI) */
bool length_2(prolog_state_t* state, term_t* list, term_t* length);
bool nth0_3(prolog_state_t* state, term_t* n, term_t* list, term_t* elem);
bool nth1_3(prolog_state_t* state, term_t* n, term_t* list, term_t* elem);
bool last_2(prolog_state_t* state, term_t* list, term_t* last);
bool reverse_2(prolog_state_t* state, term_t* list, term_t* reversed);

/* Atom/string predicates (ISO) */
bool atom_codes_2(prolog_state_t* state, term_t* atom, term_t* codes);
bool atom_chars_2(prolog_state_t* state, term_t* atom, term_t* chars);
bool atom_length_2(prolog_state_t* state, term_t* atom, term_t* length);
bool atom_concat_3(prolog_state_t* state, term_t* atom1, term_t* atom2, term_t* result);
bool sub_atom_5(prolog_state_t* state, term_t* atom, term_t* before, term_t* length, term_t* after, term_t* sub);

/* Term manipulation predicates (ISO) */
bool functor_3(prolog_state_t* state, term_t* term, term_t* functor, term_t* arity);
bool arg_3(prolog_state_t* state, term_t* n, term_t* term, term_t* arg);
bool univ_2(prolog_state_t* state, term_t* term, term_t* list);
bool copy_term_2(prolog_state_t* state, term_t* term, term_t* copy);

/* Control predicates (ISO) */
bool true_0(prolog_state_t* state);
bool once_1(prolog_state_t* state, term_t* goal);
bool ignore_1(prolog_state_t* state, term_t* goal);

/* Solution collection predicates (ISO) */
bool bagof_3(prolog_state_t* state, term_t* template, term_t* goal, term_t* bag);
bool setof_3(prolog_state_t* state, term_t* template, term_t* goal, term_t* set);

/* Sorting predicates (ISO/SWI) */
bool sort_2(prolog_state_t* state, term_t* list, term_t* sorted);
bool msort_2(prolog_state_t* state, term_t* list, term_t* sorted);
bool keysort_2(prolog_state_t* state, term_t* list, term_t* sorted);

/* Meta-call predicates (ISO/SWI) */
bool call_1(prolog_state_t* state, term_t* goal);
bool call_2(prolog_state_t* state, term_t* closure, term_t* arg1);
bool call_3(prolog_state_t* state, term_t* closure, term_t* arg1, term_t* arg2);
bool apply_2(prolog_state_t* state, term_t* goal, term_t* args);

bool test_type_checking_0(prolog_state_t* state);
bool test_list_predicates_0(prolog_state_t* state);
bool test_atom_predicates_0(prolog_state_t* state);
bool test_term_manipulation_0(prolog_state_t* state);
bool test_arithmetic_0(prolog_state_t* state);
bool test_term_comparison_0(prolog_state_t* state);
bool test_io_0(prolog_state_t* state);
bool test_sorting_0(prolog_state_t* state);
bool main_0(prolog_state_t* state);
bool test_type_checking_0(prolog_state_t* state) {
    /* Clause 1: test_type_checking :- write(Testing type checking predicates:
),(atom(hello)->write(  atom(hello) - passed
);write(  atom(hello) - failed
)),(integer(42)->write(  integer(42) - passed
);write(  integer(42) - failed
)),(number(42)->write(  number(42) - passed
);write(  number(42) - failed
)),_2744=5,(nonvar(_2744)->write(  nonvar(5) - passed
);write(  nonvar(5) - failed
)),(atomic(hello)->write(  atomic(hello) - passed
);write(  atomic(hello) - failed
)),(is_list([1,2,3])->write(  is_list([1,2,3]) - passed
);write(  is_list([1,2,3]) - failed
)),(ground(foo(1,2))->write(  ground(foo(1,2)) - passed
);write(  ground(foo(1,2)) - failed
)) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__2744 = create_var(state->next_var_id++);
        if (true) {

            do {
    if (!write_1(state, create_atom("Testing type checking predicates:\n"))) { state->failed = true; break; }
    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
    if (!if_then_2(state, create_compound("atom", 1, (term_t*[]){create_atom("hello")}), create_compound("write", 1, (term_t*[]){create_atom("  atom(hello) - passed\n")}))) { state->failed = true; break; }

    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
    if (!write_1(state, create_atom("  atom(hello) - failed\n"))) { state->failed = true; break; }

    }
    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
    if (!if_then_2(state, create_compound("integer", 1, (term_t*[]){create_int(42)}), create_compound("write", 1, (term_t*[]){create_atom("  integer(42) - passed\n")}))) { state->failed = true; break; }

    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
    if (!write_1(state, create_atom("  integer(42) - failed\n"))) { state->failed = true; break; }

    }
    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
    if (!if_then_2(state, create_compound("number", 1, (term_t*[]){create_int(42)}), create_compound("write", 1, (term_t*[]){create_atom("  number(42) - passed\n")}))) { state->failed = true; break; }

    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
    if (!write_1(state, create_atom("  number(42) - failed\n"))) { state->failed = true; break; }

    }
    if (!eq_2(state, var__2744, create_int(5))) { state->failed = true; break; }
    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
    if (!if_then_2(state, create_compound("nonvar", 1, (term_t*[]){var__2744}), create_compound("write", 1, (term_t*[]){create_atom("  nonvar(5) - passed\n")}))) { state->failed = true; break; }

    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
    if (!write_1(state, create_atom("  nonvar(5) - failed\n"))) { state->failed = true; break; }

    }
    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
    if (!if_then_2(state, create_compound("atomic", 1, (term_t*[]){create_atom("hello")}), create_compound("write", 1, (term_t*[]){create_atom("  atomic(hello) - passed\n")}))) { state->failed = true; break; }

    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
    if (!write_1(state, create_atom("  atomic(hello) - failed\n"))) { state->failed = true; break; }

    }
    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
    if (!if_then_2(state, create_compound("is_list", 1, (term_t*[]){create_list(create_int(1), create_list(create_int(2), create_list(create_int(3), create_nil())))}), create_compound("write", 1, (term_t*[]){create_atom("  is_list([1,2,3]) - passed\n")}))) { state->failed = true; break; }

    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
    if (!write_1(state, create_atom("  is_list([1,2,3]) - failed\n"))) { state->failed = true; break; }

    }
    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
    if (!if_then_2(state, create_compound("ground", 1, (term_t*[]){create_compound("foo", 2, (term_t*[]){create_int(1), create_int(2)})}), create_compound("write", 1, (term_t*[]){create_atom("  ground(foo(1,2)) - passed\n")}))) { state->failed = true; break; }

    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
    if (!write_1(state, create_atom("  ground(foo(1,2)) - failed\n"))) { state->failed = true; break; }

    }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool test_list_predicates_0(prolog_state_t* state) {
    /* Clause 1: test_list_predicates :- write(
Testing list predicates:
),length([1,2,3],_2960),format(  length([1,2,3], ~w)
,[_2960]),nth0(1,[a,b,c],_3002),format(  nth0(1, [a,b,c], ~w)
,[_3002]),last([1,2,3],_3042),format(  last([1,2,3], ~w)
,[_3042]),reverse([1,2,3],_3082),format(  reverse([1,2,3], ~w)
,[_3082]) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__2960 = create_var(state->next_var_id++);
        term_t* var__3002 = create_var(state->next_var_id++);
        term_t* var__3042 = create_var(state->next_var_id++);
        term_t* var__3082 = create_var(state->next_var_id++);
        if (true) {

            do {
    if (!write_1(state, create_atom("\nTesting list predicates:\n"))) { state->failed = true; break; }
    if (!length_2(state, create_list(create_int(1), create_list(create_int(2), create_list(create_int(3), create_nil()))), var__2960)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  length([1,2,3], ~w)\n"), create_list(var__2960, create_nil()))) { state->failed = true; break; }
    if (!nth0_3(state, create_int(1), create_list(create_atom("a"), create_list(create_atom("b"), create_list(create_atom("c"), create_nil()))), var__3002)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  nth0(1, [a,b,c], ~w)\n"), create_list(var__3002, create_nil()))) { state->failed = true; break; }
    if (!last_2(state, create_list(create_int(1), create_list(create_int(2), create_list(create_int(3), create_nil()))), var__3042)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  last([1,2,3], ~w)\n"), create_list(var__3042, create_nil()))) { state->failed = true; break; }
    if (!reverse_2(state, create_list(create_int(1), create_list(create_int(2), create_list(create_int(3), create_nil()))), var__3082)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  reverse([1,2,3], ~w)\n"), create_list(var__3082, create_nil()))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool test_atom_predicates_0(prolog_state_t* state) {
    /* Clause 1: test_atom_predicates :- write(
Testing atom predicates:
),atom_codes(hello,_3168),format(  atom_codes(hello, ~w)
,[_3168]),atom_chars(world,_3188),format(  atom_chars(world, ~w)
,[_3188]),atom_length(hello,_3208),format(  atom_length(hello, ~w)
,[_3208]),atom_concat(hello,world,_3230),format(  atom_concat(hello, world, ~w)
,[_3230]) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3168 = create_var(state->next_var_id++);
        term_t* var__3188 = create_var(state->next_var_id++);
        term_t* var__3208 = create_var(state->next_var_id++);
        term_t* var__3230 = create_var(state->next_var_id++);
        if (true) {

            do {
    if (!write_1(state, create_atom("\nTesting atom predicates:\n"))) { state->failed = true; break; }
    if (!atom_codes_2(state, create_atom("hello"), var__3168)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  atom_codes(hello, ~w)\n"), create_list(var__3168, create_nil()))) { state->failed = true; break; }
    if (!atom_chars_2(state, create_atom("world"), var__3188)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  atom_chars(world, ~w)\n"), create_list(var__3188, create_nil()))) { state->failed = true; break; }
    if (!atom_length_2(state, create_atom("hello"), var__3208)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  atom_length(hello, ~w)\n"), create_list(var__3208, create_nil()))) { state->failed = true; break; }
    if (!atom_concat_3(state, create_atom("hello"), create_atom("world"), var__3230)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  atom_concat(hello, world, ~w)\n"), create_list(var__3230, create_nil()))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool test_term_manipulation_0(prolog_state_t* state) {
    /* Clause 1: test_term_manipulation :- write(
Testing term manipulation:
),functor(foo(a,b,c),_3324,_3326),format(  functor(foo(a,b,c), ~w, ~w)
,[_3324,_3326]),arg(2,foo(a,b,c),_3362),format(  arg(2, foo(a,b,c), ~w)
,[_3362]),foo(a,b)=.._3388,format(  foo(a,b) =.. ~w
,[_3388]) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3324 = create_var(state->next_var_id++);
        term_t* var__3326 = create_var(state->next_var_id++);
        term_t* var__3362 = create_var(state->next_var_id++);
        term_t* var__3388 = create_var(state->next_var_id++);
        if (true) {

            do {
    if (!write_1(state, create_atom("\nTesting term manipulation:\n"))) { state->failed = true; break; }
    if (!functor_3(state, create_compound("foo", 3, (term_t*[]){create_atom("a"), create_atom("b"), create_atom("c")}), var__3324, var__3326)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  functor(foo(a,b,c), ~w, ~w)\n"), create_list(var__3324, create_list(var__3326, create_nil())))) { state->failed = true; break; }
    if (!arg_3(state, create_int(2), create_compound("foo", 3, (term_t*[]){create_atom("a"), create_atom("b"), create_atom("c")}), var__3362)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  arg(2, foo(a,b,c), ~w)\n"), create_list(var__3362, create_nil()))) { state->failed = true; break; }
    if (!univ_2(state, create_compound("foo", 2, (term_t*[]){create_atom("a"), create_atom("b")}), var__3388)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  foo(a,b) =.. ~w\n"), create_list(var__3388, create_nil()))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool test_arithmetic_0(prolog_state_t* state) {
    /* Clause 1: test_arithmetic :- write(
Testing arithmetic:
),_832 is abs(-5),format(  abs(-5) = ~w
,[_832]),_858 is max(10,5),format(  max(10, 5) = ~w
,[_858]),_884 is min(10,5),format(  min(10, 5) = ~w
,[_884]),_910 is 2^3,format(  2 ^ 3 = ~w
,[_910]),_936 is 10 mod 3,format(  10 mod 3 = ~w
,[_936]),_960 is sign(-10),format(  sign(-10) = ~w
,[_960]),_984 is round(3),format(  round(3) = ~w
,[_984]) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3464 = create_var(state->next_var_id++);
        term_t* var__3490 = create_var(state->next_var_id++);
        term_t* var__3516 = create_var(state->next_var_id++);
        term_t* var__3542 = create_var(state->next_var_id++);
        term_t* var__3568 = create_var(state->next_var_id++);
        term_t* var__3592 = create_var(state->next_var_id++);
        term_t* var__3616 = create_var(state->next_var_id++);
        if (true) {

            do {
    if (!write_1(state, create_atom("\nTesting arithmetic:\n"))) { state->failed = true; break; }
    if (!is_2(state, var__3464, create_compound("abs", 1, (term_t*[]){create_int(-5)}))) { state->failed = true; break; }
    if (!format_2(state, create_atom("  abs(-5) = ~w\n"), create_list(var__3464, create_nil()))) { state->failed = true; break; }
    if (!is_2(state, var__3490, create_compound("max", 2, (term_t*[]){create_int(10), create_int(5)}))) { state->failed = true; break; }
    if (!format_2(state, create_atom("  max(10, 5) = ~w\n"), create_list(var__3490, create_nil()))) { state->failed = true; break; }
    if (!is_2(state, var__3516, create_compound("min", 2, (term_t*[]){create_int(10), create_int(5)}))) { state->failed = true; break; }
    if (!format_2(state, create_atom("  min(10, 5) = ~w\n"), create_list(var__884, create_nil()))) { state->failed = true; break; }
    if (!is_2(state, var__910, create_compound("^", 2, (term_t*[]){create_int(2), create_int(3)}))) { state->failed = true; break; }
    if (!format_2(state, create_atom("  2 ^ 3 = ~w\n"), create_list(var__910, create_nil()))) { state->failed = true; break; }
    if (!is_2(state, var__936, create_compound("mod", 2, (term_t*[]){create_int(10), create_int(3)}))) { state->failed = true; break; }
    if (!format_2(state, create_atom("  10 mod 3 = ~w\n"), create_list(var__936, create_nil()))) { state->failed = true; break; }
    if (!is_2(state, var__960, create_compound("sign", 1, (term_t*[]){create_int(-10)}))) { state->failed = true; break; }
    if (!format_2(state, create_atom("  sign(-10) = ~w\n"), create_list(var__960, create_nil()))) { state->failed = true; break; }
    if (!is_2(state, var__984, create_compound("round", 1, (term_t*[]){create_int(3)}))) { state->failed = true; break; }
    if (!format_2(state, create_atom("  round(3) = ~w\n"), create_list(var__984, create_nil()))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool test_term_comparison_0(prolog_state_t* state) {
    /* Clause 1: test_term_comparison :- write(
Testing term comparison:
),(1@<2->write(  1 @< 2 - passed
);write(  1 @< 2 - failed
)),(atom@<foo(1)->write(  atom @< foo(1) - passed
);write(  atom @< foo(1) - failed
)),compare(_1154,1,2),format(  compare(~w, 1, 2)
,[_1154]) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1154 = create_var(state->next_var_id++);
        if (true) {

            do {
    if (!write_1(state, create_atom("\nTesting term comparison:\n"))) { state->failed = true; break; }
    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
    if (!if_then_2(state, create_compound("@<", 2, (term_t*[]){create_int(1), create_int(2)}), create_compound("write", 1, (term_t*[]){create_atom("  1 @< 2 - passed\n")}))) { state->failed = true; break; }

    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
    if (!write_1(state, create_atom("  1 @< 2 - failed\n"))) { state->failed = true; break; }

    }
    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
    if (!if_then_2(state, create_compound("@<", 2, (term_t*[]){create_atom("atom"), create_compound("foo", 1, (term_t*[]){create_int(1)})}), create_compound("write", 1, (term_t*[]){create_atom("  atom @< foo(1) - passed\n")}))) { state->failed = true; break; }

    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
    if (!write_1(state, create_atom("  atom @< foo(1) - failed\n"))) { state->failed = true; break; }

    }
    if (!compare_3(state, var__1154, create_int(1), create_int(2))) { state->failed = true; break; }
    if (!format_2(state, create_atom("  compare(~w, 1, 2)\n"), create_list(var__1154, create_nil()))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool test_io_0(prolog_state_t* state) {
    /* Clause 1: test_io :- write(
Testing I/O predicates:
),write(  write works),nl,tab(5),write(  tab(5) works
) */
    {
        int saved_bindings_size = state->bindings.size;
        if (true) {

            do {
    if (!write_1(state, create_atom("\nTesting I/O predicates:\n"))) { state->failed = true; break; }
    if (!write_1(state, create_atom("  write works"))) { state->failed = true; break; }
    if (!nl_0(state)) { state->failed = true; break; }
    if (!tab_1(state, create_int(5))) { state->failed = true; break; }
    if (!write_1(state, create_atom("  tab(5) works\n"))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool test_sorting_0(prolog_state_t* state) {
    /* Clause 1: test_sorting :- write(
Testing sorting predicates:
),sort([3,1,2,1,3],_1290),format(  sort([3,1,2,1,3], ~w) - removes duplicates
,[_1290]),msort([3,1,2,1,3],_1342),format(  msort([3,1,2,1,3], ~w) - keeps duplicates
,[_1342]) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1290 = create_var(state->next_var_id++);
        term_t* var__1342 = create_var(state->next_var_id++);
        if (true) {

            do {
    if (!write_1(state, create_atom("\nTesting sorting predicates:\n"))) { state->failed = true; break; }
    if (!sort_2(state, create_list(create_int(3), create_list(create_int(1), create_list(create_int(2), create_list(create_int(1), create_list(create_int(3), create_nil()))))), var__1290)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  sort([3,1,2,1,3], ~w) - removes duplicates\n"), create_list(var__1290, create_nil()))) { state->failed = true; break; }
    if (!msort_2(state, create_list(create_int(3), create_list(create_int(1), create_list(create_int(2), create_list(create_int(1), create_list(create_int(3), create_nil()))))), var__1342)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  msort([3,1,2,1,3], ~w) - keeps duplicates\n"), create_list(var__1342, create_nil()))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool main_0(prolog_state_t* state) {
    /* Clause 1: main :- write(=== ISO Prolog Features Test ===

),test_type_checking,test_list_predicates,test_atom_predicates,test_term_manipulation,test_arithmetic,test_term_comparison,test_io,test_sorting,write(
=== All Tests Completed ===
) */
    {
        int saved_bindings_size = state->bindings.size;
        if (true) {

            do {
    if (!write_1(state, create_atom("=== ISO Prolog Features Test ===\n\n"))) { state->failed = true; break; }
    if (!test_type_checking_0(state)) { state->failed = true; break; }
    if (!test_list_predicates_0(state)) { state->failed = true; break; }
    if (!test_atom_predicates_0(state)) { state->failed = true; break; }
    if (!test_term_manipulation_0(state)) { state->failed = true; break; }
    if (!test_arithmetic_0(state)) { state->failed = true; break; }
    if (!test_term_comparison_0(state)) { state->failed = true; break; }
    if (!test_io_0(state)) { state->failed = true; break; }
    if (!test_sorting_0(state)) { state->failed = true; break; }
    if (!write_1(state, create_atom("\n=== All Tests Completed ===\n"))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}



/* Term creation functions */
term_t* create_var(int id) {
    term_t* t = malloc(sizeof(term_t));
    t->type = TERM_VAR;
    t->data.var_id = id;
    return t;
}

term_t* create_atom(const char* name) {
    term_t* t = malloc(sizeof(term_t));
    t->type = TERM_ATOM;
    t->data.atom = strdup(name);
    return t;
}

term_t* create_int(int val) {
    term_t* t = malloc(sizeof(term_t));
    t->type = TERM_INT;
    t->data.int_val = val;
    return t;
}

term_t* create_compound(const char* functor, int arity, term_t** args) {
    term_t* t = malloc(sizeof(term_t));
    t->type = TERM_COMPOUND;
    t->data.compound.functor = strdup(functor);
    t->data.compound.arity = arity;
    t->data.compound.args = malloc(sizeof(term_t*) * arity);
    for (int i = 0; i < arity; i++) {
        t->data.compound.args[i] = args[i];
    }
    return t;
}

term_t* create_list(term_t* head, term_t* tail) {
    term_t* t = malloc(sizeof(term_t));
    t->type = TERM_LIST;
    t->data.list.head = head;
    t->data.list.tail = tail;
    return t;
}

term_t* create_nil() {
    term_t* t = malloc(sizeof(term_t));
    t->type = TERM_NIL;
    return t;
}

/* Dereference a term through variable bindings */
term_t* deref(prolog_state_t* state, term_t* term) {
    if (term->type != TERM_VAR) return term;
    
    for (int i = 0; i < state->bindings.size; i++) {
        if (state->bindings.bindings[i].var_id == term->data.var_id) {
            return deref(state, state->bindings.bindings[i].value);
        }
    }
    return term;
}

/* Unification algorithm */
bool unify(prolog_state_t* state, term_t* t1, term_t* t2) {
    t1 = deref(state, t1);
    t2 = deref(state, t2);
    
    if (t1->type == TERM_VAR) {
        /* Bind variable t1 to t2 */
        if (state->bindings.size >= state->bindings.capacity) {
            state->bindings.capacity = state->bindings.capacity * 2 + 1;
            state->bindings.bindings = realloc(state->bindings.bindings, 
                sizeof(binding_t) * state->bindings.capacity);
        }
        state->bindings.bindings[state->bindings.size].var_id = t1->data.var_id;
        state->bindings.bindings[state->bindings.size].value = t2;
        state->bindings.size++;
        return true;
    }
    
    if (t2->type == TERM_VAR) {
        return unify(state, t2, t1);
    }
    
    if (t1->type != t2->type) return false;
    
    switch (t1->type) {
        case TERM_ATOM:
            return strcmp(t1->data.atom, t2->data.atom) == 0;
        case TERM_INT:
            return t1->data.int_val == t2->data.int_val;
        case TERM_NIL:
            return true;
        case TERM_COMPOUND:
            if (strcmp(t1->data.compound.functor, t2->data.compound.functor) != 0)
                return false;
            if (t1->data.compound.arity != t2->data.compound.arity)
                return false;
            for (int i = 0; i < t1->data.compound.arity; i++) {
                if (!unify(state, t1->data.compound.args[i], t2->data.compound.args[i]))
                    return false;
            }
            return true;
        case TERM_LIST:
            return unify(state, t1->data.list.head, t2->data.list.head) &&
                   unify(state, t1->data.list.tail, t2->data.list.tail);
        default:
            return false;
    }
}

/* Choice point management */
void push_choice_point(prolog_state_t* state, int pred_id, int clause_idx) {
    choice_point_t* cp = malloc(sizeof(choice_point_t));
    cp->predicate_id = pred_id;
    cp->clause_index = clause_idx;
    
    /* Save current bindings */
    cp->saved_bindings.size = state->bindings.size;
    cp->saved_bindings.capacity = state->bindings.capacity;
    cp->saved_bindings.bindings = malloc(sizeof(binding_t) * state->bindings.capacity);
    memcpy(cp->saved_bindings.bindings, state->bindings.bindings,
           sizeof(binding_t) * state->bindings.size);
    
    cp->prev = state->choice_stack;
    state->choice_stack = cp;
}

bool pop_choice_point(prolog_state_t* state) {
    if (!state->choice_stack) return false;
    
    choice_point_t* cp = state->choice_stack;
    
    /* Restore bindings */
    free(state->bindings.bindings);
    state->bindings = cp->saved_bindings;
    
    state->choice_stack = cp->prev;
    free(cp);
    return true;
}

/* Cut implementation */
void perform_cut(prolog_state_t* state) {
    int current_level = state->cut_level;
    
    /* Remove all choice points at or above current level */
    while (state->choice_stack && state->choice_stack->predicate_id >= current_level) {
        choice_point_t* cp = state->choice_stack;
        state->choice_stack = cp->prev;
        free(cp->saved_bindings.bindings);
        free(cp);
    }
}

/* Built-in arithmetic and comparison operators */
bool gt_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    if (t1->type == TERM_INT && t2->type == TERM_INT) {
        return t1->data.int_val > t2->data.int_val;
    }
    state->failed = true;
    return false;
}

bool lt_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    if (t1->type == TERM_INT && t2->type == TERM_INT) {
        return t1->data.int_val < t2->data.int_val;
    }
    state->failed = true;
    return false;
}

bool gte_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    if (t1->type == TERM_INT && t2->type == TERM_INT) {
        return t1->data.int_val >= t2->data.int_val;
    }
    state->failed = true;
    return false;
}

bool lte_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    if (t1->type == TERM_INT && t2->type == TERM_INT) {
        return t1->data.int_val <= t2->data.int_val;
    }
    state->failed = true;
    return false;
}

bool eq_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    return unify(state, arg1, arg2);
}

bool neq_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    return !unify(state, arg1, arg2);
}

bool eqeq_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Structural equality (==) - no unification */
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    
    if (t1->type != t2->type) return false;
    
    if (t1->type == TERM_INT) {
        return t1->data.int_val == t2->data.int_val;
    } else if (t1->type == TERM_ATOM) {
        return strcmp(t1->data.atom, t2->data.atom) == 0;
    } else if (t1->type == TERM_VAR) {
        return t1->data.var_id == t2->data.var_id;
    }
    /* For compound terms and lists, would need recursive comparison */
    return false;
}

bool neqeq_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    return !eqeq_2(state, arg1, arg2);
}

/* Arithmetic evaluation for is/2 */
int eval_arithmetic(prolog_state_t* state, term_t* expr) {
    term_t* t = deref(state, expr);
    
    if (t->type == TERM_INT) {
        return t->data.int_val;
    }
    
    if (t->type == TERM_COMPOUND) {
        if (strcmp(t->data.compound.functor, "+") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left + right;
        }
        if (strcmp(t->data.compound.functor, "-") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left - right;
        }
        if (strcmp(t->data.compound.functor, "*") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left * right;
        }
        if (strcmp(t->data.compound.functor, "/") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left / right;
        }
        if (strcmp(t->data.compound.functor, "//") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left / right;
        }
        if (strcmp(t->data.compound.functor, "mod") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left % right;
        }
        if (strcmp(t->data.compound.functor, "rem") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left % right;
        }
        if (strcmp(t->data.compound.functor, "^") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return (int)pow((double)left, (double)right);
        }
        if (strcmp(t->data.compound.functor, "**") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return (int)pow((double)left, (double)right);
        }
        if (strcmp(t->data.compound.functor, ">>") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left >> right;
        }
        if (strcmp(t->data.compound.functor, "<<") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left << right;
        }
        if (strcmp(t->data.compound.functor, "/\\") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left & right;
        }
        if (strcmp(t->data.compound.functor, "\\/") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left | right;
        }
        if (strcmp(t->data.compound.functor, "xor") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left ^ right;
        }
        /* Unary functions */
        if (strcmp(t->data.compound.functor, "abs") == 0 && t->data.compound.arity == 1) {
            int val = eval_arithmetic(state, t->data.compound.args[0]);
            return abs(val);
        }
        if (strcmp(t->data.compound.functor, "sign") == 0 && t->data.compound.arity == 1) {
            int val = eval_arithmetic(state, t->data.compound.args[0]);
            return (val > 0) - (val < 0);
        }
        if (strcmp(t->data.compound.functor, "-") == 0 && t->data.compound.arity == 1) {
            int val = eval_arithmetic(state, t->data.compound.args[0]);
            return -val;
        }
        if (strcmp(t->data.compound.functor, "+") == 0 && t->data.compound.arity == 1) {
            return eval_arithmetic(state, t->data.compound.args[0]);
        }
        if (strcmp(t->data.compound.functor, "sqrt") == 0 && t->data.compound.arity == 1) {
            int val = eval_arithmetic(state, t->data.compound.args[0]);
            return (int)sqrt((double)val);
        }
        if (strcmp(t->data.compound.functor, "floor") == 0 && t->data.compound.arity == 1) {
            int val = eval_arithmetic(state, t->data.compound.args[0]);
            return (int)floor((double)val);
        }
        if (strcmp(t->data.compound.functor, "ceiling") == 0 && t->data.compound.arity == 1) {
            int val = eval_arithmetic(state, t->data.compound.args[0]);
            return (int)ceil((double)val);
        }
        if (strcmp(t->data.compound.functor, "round") == 0 && t->data.compound.arity == 1) {
            int val = eval_arithmetic(state, t->data.compound.args[0]);
            return (int)round((double)val);
        }
        if (strcmp(t->data.compound.functor, "truncate") == 0 && t->data.compound.arity == 1) {
            int val = eval_arithmetic(state, t->data.compound.args[0]);
            return (int)trunc((double)val);
        }
        if (strcmp(t->data.compound.functor, "min") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left < right ? left : right;
        }
        if (strcmp(t->data.compound.functor, "max") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left > right ? left : right;
        }
    }
    
    return 0;
}

bool is_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    int result = eval_arithmetic(state, arg2);
    term_t* result_term = create_int(result);
    return unify(state, arg1, result_term);
}

/* Built-in I/O predicates */
void print_term(term_t* term) {
    if (term->type == TERM_ATOM) {
        printf("%s", term->data.atom);
    } else if (term->type == TERM_INT) {
        printf("%d", term->data.int_val);
    } else if (term->type == TERM_NIL) {
        printf("[]");
    } else if (term->type == TERM_LIST) {
        printf("[");
        term_t* current = term;
        while (current->type == TERM_LIST) {
            print_term(current->data.list.head);
            current = current->data.list.tail;
            if (current->type == TERM_LIST) {
                printf(", ");
            }
        }
        if (current->type != TERM_NIL) {
            printf("|");
            print_term(current);
        }
        printf("]");
    } else if (term->type == TERM_VAR) {
        printf("_%d", term->data.var_id);
    } else if (term->type == TERM_COMPOUND) {
        printf("%s(", term->data.compound.functor);
        for (int i = 0; i < term->data.compound.arity; i++) {
            print_term(term->data.compound.args[i]);
            if (i < term->data.compound.arity - 1) {
                printf(", ");
            }
        }
        printf(")");
    }
}

bool write_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    print_term(t);
    return true;
}

bool format_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    term_t* format_str = deref(state, arg1);
    term_t* args_list = deref(state, arg2);
    
    if (format_str->type != TERM_ATOM) {
        return false;
    }
    
    /* Simple format implementation - just replaces ~w with argument values */
    const char* fmt = format_str->data.atom;
    term_t* current_arg = args_list;
    
    for (int i = 0; fmt[i] != 0; i++) {
        if (fmt[i] == 126 && fmt[i+1] == 119) {  /* ~w */
            if (current_arg->type == TERM_LIST) {
                print_term(current_arg->data.list.head);
                current_arg = current_arg->data.list.tail;
            }
            i++;  /* Skip the w */
        } else {
            putchar(fmt[i]);
        }
    }
    
    return true;
}

/* Additional I/O predicates */
bool nl_0(prolog_state_t* state) {
    printf("\n");
    return true;
}

bool tab_1(prolog_state_t* state, term_t* arg1) {
    term_t* n = deref(state, arg1);
    if (n->type != TERM_INT) {
        state->failed = true;
        return false;
    }
    for (int i = 0; i < n->data.int_val; i++) {
        printf(" ");
    }
    return true;
}

bool get_char_1(prolog_state_t* state, term_t* arg1) {
    int c = getchar();
    if (c == EOF) {
        return unify(state, arg1, create_atom("end_of_file"));
    }
    char str[2] = {(char)c, 0};
    return unify(state, arg1, create_atom(str));
}

bool put_char_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    if (t->type != TERM_ATOM) {
        state->failed = true;
        return false;
    }
    if (strlen(t->data.atom) > 0) {
        putchar(t->data.atom[0]);
    }
    return true;
}

/* Type checking predicates (ISO) */
bool atom_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    return t->type == TERM_ATOM;
}

bool number_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    return t->type == TERM_INT;
}

bool integer_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    return t->type == TERM_INT;
}

bool var_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    return t->type == TERM_VAR;
}

bool nonvar_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    return t->type != TERM_VAR;
}

bool compound_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    return t->type == TERM_COMPOUND || t->type == TERM_LIST;
}

bool atomic_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    return t->type == TERM_ATOM || t->type == TERM_INT || t->type == TERM_NIL;
}

bool is_list_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    if (t->type == TERM_NIL) return true;
    if (t->type != TERM_LIST) return false;
    
    /* Check if it's a proper list (ends with NIL) */
    while (t->type == TERM_LIST) {
        t = deref(state, t->data.list.tail);
    }
    return t->type == TERM_NIL;
}

bool ground_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    if (t->type == TERM_VAR) return false;
    if (t->type == TERM_COMPOUND) {
        for (int i = 0; i < t->data.compound.arity; i++) {
            if (!ground_1(state, t->data.compound.args[i])) return false;
        }
    }
    if (t->type == TERM_LIST) {
        return ground_1(state, t->data.list.head) && ground_1(state, t->data.list.tail);
    }
    return true;
}

bool callable_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    return t->type == TERM_ATOM || t->type == TERM_COMPOUND;
}

/* Term comparison predicates (ISO) */
int term_compare(term_t* t1, term_t* t2) {
    /* Standard term ordering: var < number < atom < compound */
    if (t1->type != t2->type) {
        return t1->type - t2->type;
    }
    
    switch (t1->type) {
        case TERM_VAR:
            return t1->data.var_id - t2->data.var_id;
        case TERM_INT:
            return t1->data.int_val - t2->data.int_val;
        case TERM_ATOM:
            return strcmp(t1->data.atom, t2->data.atom);
        case TERM_NIL:
            return 0;
        case TERM_COMPOUND: {
            int cmp = strcmp(t1->data.compound.functor, t2->data.compound.functor);
            if (cmp != 0) return cmp;
            cmp = t1->data.compound.arity - t2->data.compound.arity;
            if (cmp != 0) return cmp;
            for (int i = 0; i < t1->data.compound.arity; i++) {
                cmp = term_compare(t1->data.compound.args[i], t2->data.compound.args[i]);
                if (cmp != 0) return cmp;
            }
            return 0;
        }
        case TERM_LIST: {
            int cmp = term_compare(t1->data.list.head, t2->data.list.head);
            if (cmp != 0) return cmp;
            return term_compare(t1->data.list.tail, t2->data.list.tail);
        }
        default:
            return 0;
    }
}

bool term_lt_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    return term_compare(t1, t2) < 0;
}

bool term_gt_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    return term_compare(t1, t2) > 0;
}

bool term_lte_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    return term_compare(t1, t2) <= 0;
}

bool term_gte_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    return term_compare(t1, t2) >= 0;
}

bool compare_3(prolog_state_t* state, term_t* order, term_t* arg1, term_t* arg2) {
    term_t* t1 = deref(state, arg1);
    term_t* t2 = deref(state, arg2);
    int cmp = term_compare(t1, t2);
    
    term_t* result;
    if (cmp < 0) {
        result = create_atom("<");
    } else if (cmp > 0) {
        result = create_atom(">");
    } else {
        result = create_atom("=");
    }
    
    return unify(state, order, result);
}

/* List predicates (ISO/SWI) */
bool length_2(prolog_state_t* state, term_t* list, term_t* length) {
    term_t* l = deref(state, list);
    int count = 0;
    
    while (l->type == TERM_LIST) {
        count++;
        l = deref(state, l->data.list.tail);
    }
    
    if (l->type != TERM_NIL) {
        state->failed = true;
        return false;
    }
    
    return unify(state, length, create_int(count));
}

bool nth0_3(prolog_state_t* state, term_t* n, term_t* list, term_t* elem) {
    term_t* n_deref = deref(state, n);
    if (n_deref->type != TERM_INT) {
        state->failed = true;
        return false;
    }
    
    int index = n_deref->data.int_val;
    term_t* l = deref(state, list);
    
    for (int i = 0; i < index; i++) {
        if (l->type != TERM_LIST) {
            state->failed = true;
            return false;
        }
        l = deref(state, l->data.list.tail);
    }
    
    if (l->type != TERM_LIST) {
        state->failed = true;
        return false;
    }
    
    return unify(state, elem, l->data.list.head);
}

bool nth1_3(prolog_state_t* state, term_t* n, term_t* list, term_t* elem) {
    term_t* n_deref = deref(state, n);
    if (n_deref->type != TERM_INT) {
        state->failed = true;
        return false;
    }
    
    term_t* zero_based = create_int(n_deref->data.int_val - 1);
    return nth0_3(state, zero_based, list, elem);
}

bool last_2(prolog_state_t* state, term_t* list, term_t* last) {
    term_t* l = deref(state, list);
    
    if (l->type != TERM_LIST) {
        state->failed = true;
        return false;
    }
    
    term_t* current_head = l->data.list.head;
    l = deref(state, l->data.list.tail);
    
    while (l->type == TERM_LIST) {
        current_head = l->data.list.head;
        l = deref(state, l->data.list.tail);
    }
    
    if (l->type != TERM_NIL) {
        state->failed = true;
        return false;
    }
    
    return unify(state, last, current_head);
}

bool reverse_2(prolog_state_t* state, term_t* list, term_t* reversed) {
    term_t* l = deref(state, list);
    term_t* result = create_nil();
    
    while (l->type == TERM_LIST) {
        result = create_list(l->data.list.head, result);
        l = deref(state, l->data.list.tail);
    }
    
    if (l->type != TERM_NIL) {
        state->failed = true;
        return false;
    }
    
    return unify(state, reversed, result);
}

/* Atom/string predicates (ISO) */
bool atom_codes_2(prolog_state_t* state, term_t* atom, term_t* codes) {
    term_t* a = deref(state, atom);
    term_t* c = deref(state, codes);
    
    if (a->type == TERM_ATOM) {
        /* Convert atom to codes */
        const char* str = a->data.atom;
        term_t* result = create_nil();
        
        /* Build list in reverse, then reverse it */
        for (int i = strlen(str) - 1; i >= 0; i--) {
            result = create_list(create_int((unsigned char)str[i]), result);
        }
        
        return unify(state, codes, result);
    } else if (c->type == TERM_LIST || c->type == TERM_NIL) {
        /* Convert codes to atom */
        char buffer[1024];
        int idx = 0;
        term_t* l = c;
        
        while (l->type == TERM_LIST && idx < 1023) {
            term_t* head = deref(state, l->data.list.head);
            if (head->type != TERM_INT) {
                state->failed = true;
                return false;
            }
            buffer[idx++] = (char)head->data.int_val;
            l = deref(state, l->data.list.tail);
        }
        
        if (l->type != TERM_NIL) {
            state->failed = true;
            return false;
        }
        
        buffer[idx] = 0;
        return unify(state, atom, create_atom(buffer));
    }
    
    state->failed = true;
    return false;
}

bool atom_chars_2(prolog_state_t* state, term_t* atom, term_t* chars) {
    term_t* a = deref(state, atom);
    term_t* c = deref(state, chars);
    
    if (a->type == TERM_ATOM) {
        /* Convert atom to chars */
        const char* str = a->data.atom;
        term_t* result = create_nil();
        
        for (int i = strlen(str) - 1; i >= 0; i--) {
            char ch[2] = {str[i], 0};
            result = create_list(create_atom(ch), result);
        }
        
        return unify(state, chars, result);
    } else if (c->type == TERM_LIST || c->type == TERM_NIL) {
        /* Convert chars to atom */
        char buffer[1024];
        int idx = 0;
        term_t* l = c;
        
        while (l->type == TERM_LIST && idx < 1023) {
            term_t* head = deref(state, l->data.list.head);
            if (head->type != TERM_ATOM) {
                state->failed = true;
                return false;
            }
            if (strlen(head->data.atom) > 0) {
                buffer[idx++] = head->data.atom[0];
            }
            l = deref(state, l->data.list.tail);
        }
        
        if (l->type != TERM_NIL) {
            state->failed = true;
            return false;
        }
        
        buffer[idx] = 0;
        return unify(state, atom, create_atom(buffer));
    }
    
    state->failed = true;
    return false;
}

bool atom_length_2(prolog_state_t* state, term_t* atom, term_t* length) {
    term_t* a = deref(state, atom);
    if (a->type != TERM_ATOM) {
        state->failed = true;
        return false;
    }
    return unify(state, length, create_int(strlen(a->data.atom)));
}

bool atom_concat_3(prolog_state_t* state, term_t* atom1, term_t* atom2, term_t* result) {
    term_t* a1 = deref(state, atom1);
    term_t* a2 = deref(state, atom2);
    
    if (a1->type != TERM_ATOM || a2->type != TERM_ATOM) {
        state->failed = true;
        return false;
    }
    
    /* Check for potential overflow - snprintf is safe but truncates */
    size_t len1 = strlen(a1->data.atom);
    size_t len2 = strlen(a2->data.atom);
    if (len1 + len2 >= 2047) {
        /* Result would be truncated */
        state->failed = true;
        return false;
    }
    
    char buffer[2048];
    snprintf(buffer, sizeof(buffer), "%s%s", a1->data.atom, a2->data.atom);
    return unify(state, result, create_atom(buffer));
}

bool sub_atom_5(prolog_state_t* state, term_t* atom, term_t* before, term_t* length, term_t* after, term_t* sub) {
    term_t* a = deref(state, atom);
    if (a->type != TERM_ATOM) {
        state->failed = true;
        return false;
    }
    
    term_t* b = deref(state, before);
    term_t* l = deref(state, length);
    term_t* aft = deref(state, after);
    
    /* Simple case: all positions specified */
    if (b->type == TERM_INT && l->type == TERM_INT) {
        int before_val = b->data.int_val;
        int length_val = l->data.int_val;
        int atom_len = strlen(a->data.atom);
        
        if (before_val < 0 || length_val < 0 || before_val + length_val > atom_len) {
            state->failed = true;
            return false;
        }
        
        char buffer[1024];
        strncpy(buffer, a->data.atom + before_val, length_val);
        buffer[length_val] = 0;
        
        int after_val = atom_len - before_val - length_val;
        
        return unify(state, sub, create_atom(buffer)) &&
               unify(state, after, create_int(after_val));
    }
    
    /* Non-deterministic case would require backtracking */
    state->failed = true;
    return false;
}

/* Term manipulation predicates (ISO) */
bool functor_3(prolog_state_t* state, term_t* term, term_t* functor, term_t* arity) {
    term_t* t = deref(state, term);
    
    if (t->type == TERM_COMPOUND) {
        return unify(state, functor, create_atom(t->data.compound.functor)) &&
               unify(state, arity, create_int(t->data.compound.arity));
    } else if (t->type == TERM_ATOM) {
        return unify(state, functor, t) &&
               unify(state, arity, create_int(0));
    } else if (t->type == TERM_INT) {
        return unify(state, functor, t) &&
               unify(state, arity, create_int(0));
    }
    
    /* Construction mode: functor and arity are given */
    term_t* f = deref(state, functor);
    term_t* a = deref(state, arity);
    
    if (f->type == TERM_ATOM && a->type == TERM_INT) {
        if (a->data.int_val == 0) {
            return unify(state, term, f);
        } else {
            /* Create compound with unbound variables as args */
            term_t** args = malloc(sizeof(term_t*) * a->data.int_val);
            if (!args) {
                state->failed = true;
                return false;
            }
            for (int i = 0; i < a->data.int_val; i++) {
                args[i] = create_var(i);
            }
            term_t* compound = create_compound(f->data.atom, a->data.int_val, args);
            return unify(state, term, compound);
        }
    }
    
    state->failed = true;
    return false;
}

bool arg_3(prolog_state_t* state, term_t* n, term_t* term, term_t* arg) {
    term_t* n_deref = deref(state, n);
    term_t* t = deref(state, term);
    
    if (n_deref->type != TERM_INT || t->type != TERM_COMPOUND) {
        state->failed = true;
        return false;
    }
    
    int index = n_deref->data.int_val;
    if (index < 1 || index > t->data.compound.arity) {
        state->failed = true;
        return false;
    }
    
    return unify(state, arg, t->data.compound.args[index - 1]);
}

bool univ_2(prolog_state_t* state, term_t* term, term_t* list) {
    term_t* t = deref(state, term);
    term_t* l = deref(state, list);
    
    if (t->type != TERM_VAR) {
        /* Term to list mode */
        if (t->type == TERM_COMPOUND) {
            /* Build list: [functor|args] */
            term_t* result = create_nil();
            for (int i = t->data.compound.arity - 1; i >= 0; i--) {
                result = create_list(t->data.compound.args[i], result);
            }
            result = create_list(create_atom(t->data.compound.functor), result);
            return unify(state, list, result);
        } else if (t->type == TERM_ATOM || t->type == TERM_INT) {
            /* Atomic term: [term] */
            return unify(state, list, create_list(t, create_nil()));
        }
    } else if (l->type == TERM_LIST) {
        /* List to term mode */
        term_t* head = deref(state, l->data.list.head);
        term_t* tail = deref(state, l->data.list.tail);
        
        if (tail->type == TERM_NIL) {
            /* Single element list */
            return unify(state, term, head);
        } else if (tail->type == TERM_LIST && head->type == TERM_ATOM) {
            /* Compound term */
            int arity = 0;
            term_t* arg_list = tail;
            
            /* Count arguments */
            while (arg_list->type == TERM_LIST) {
                arity++;
                arg_list = deref(state, arg_list->data.list.tail);
            }
            
            if (arg_list->type != TERM_NIL) {
                state->failed = true;
                return false;
            }
            
            /* Build compound */
            term_t** args = malloc(sizeof(term_t*) * arity);
            arg_list = tail;
            for (int i = 0; i < arity; i++) {
                args[i] = arg_list->data.list.head;
                arg_list = deref(state, arg_list->data.list.tail);
            }
            
            term_t* compound = create_compound(head->data.atom, arity, args);
            return unify(state, term, compound);
        }
    }
    
    state->failed = true;
    return false;
}

/* Helper function to recursively copy a term */
term_t* copy_term_helper(prolog_state_t* state, term_t* term, int* var_offset) {
    term_t* t = deref(state, term);
    
    if (t->type == TERM_VAR) {
        /* Create new variable with offset ID */
        return create_var(t->data.var_id + *var_offset);
    } else if (t->type == TERM_ATOM) {
        return create_atom(t->data.atom);
    } else if (t->type == TERM_INT) {
        return create_int(t->data.int_val);
    } else if (t->type == TERM_NIL) {
        return create_nil();
    } else if (t->type == TERM_LIST) {
        term_t* new_head = copy_term_helper(state, t->data.list.head, var_offset);
        term_t* new_tail = copy_term_helper(state, t->data.list.tail, var_offset);
        return create_list(new_head, new_tail);
    } else if (t->type == TERM_COMPOUND) {
        term_t** new_args = malloc(sizeof(term_t*) * t->data.compound.arity);
        if (!new_args) {
            return create_var(0); /* fallback on malloc failure */
        }
        for (int i = 0; i < t->data.compound.arity; i++) {
            new_args[i] = copy_term_helper(state, t->data.compound.args[i], var_offset);
        }
        return create_compound(t->data.compound.functor, t->data.compound.arity, new_args);
    }
    
    return create_var(0); /* fallback */
}

bool copy_term_2(prolog_state_t* state, term_t* term, term_t* copy) {
    /* Use a large offset to avoid variable ID collisions */
    int var_offset = 100000;
    term_t* copied = copy_term_helper(state, term, &var_offset);
    return unify(state, copy, copied);
}

/* Control predicates (ISO) */
bool true_0(prolog_state_t* state) {
    return true;
}

bool once_1(prolog_state_t* state, term_t* goal) {
    /* Execute goal once, removing choice points on success */
    /* SIMPLIFIED IMPLEMENTATION: Does not actually execute the goal or manage choice points. */
    /* Full implementation would require:
     * 1. Execute the goal using call_1
     * 2. On success, remove all choice points created during goal execution
     * 3. Prevent backtracking into the goal
     * For now, this is a placeholder that succeeds without executing the goal. */
    (void)goal; /* Suppress unused parameter warning */
    return true;
}

bool ignore_1(prolog_state_t* state, term_t* goal) {
    /* Always succeeds, ignoring goal failure */
    /* SIMPLIFIED IMPLEMENTATION: Does not actually execute the goal. */
    /* Full implementation would:
     * 1. Execute the goal using call_1
     * 2. Always return true regardless of goal success/failure
     * For now, this is a placeholder that succeeds without executing the goal. */
    (void)goal; /* Suppress unused parameter warning */
    return true;
}

/* Solution collection predicates (ISO) */
bool bagof_3(prolog_state_t* state, term_t* template, term_t* goal, term_t* bag) {
    /* Simplified bagof implementation - similar to findall but should handle free variables */
    /* For now, this is a placeholder that behaves like findall */
    state->failed = true;
    return false;
}

bool setof_3(prolog_state_t* state, term_t* template, term_t* goal, term_t* set) {
    /* Simplified setof implementation - like bagof but removes duplicates and sorts */
    /* For now, this is a placeholder */
    state->failed = true;
    return false;
}

/* Sorting predicates (ISO/SWI) */
int compare_terms_for_sort(const void* a, const void* b) {
    term_t* t1 = *(term_t**)a;
    term_t* t2 = *(term_t**)b;
    return term_compare(t1, t2);
}

int compare_keys_for_keysort(const void* a, const void* b) {
    term_t* t1 = *(term_t**)a;
    term_t* t2 = *(term_t**)b;
    /* Both should be compound terms with functor "-" and arity 2 */
    /* Compare only the first argument (the key) */
    if (t1->type == TERM_COMPOUND && t2->type == TERM_COMPOUND &&
        t1->data.compound.arity >= 1 && t2->data.compound.arity >= 1) {
        return term_compare(t1->data.compound.args[0], t2->data.compound.args[0]);
    }
    return 0;
}

bool sort_2(prolog_state_t* state, term_t* list, term_t* sorted) {
    term_t* l = deref(state, list);
    
    /* Count elements */
    int count = 0;
    term_t* current = l;
    while (current->type == TERM_LIST) {
        count++;
        current = deref(state, current->data.list.tail);
    }
    
    if (current->type != TERM_NIL) {
        state->failed = true;
        return false;
    }
    
    if (count == 0) {
        return unify(state, sorted, create_nil());
    }
    
    /* Copy elements to array */
    term_t** elements = malloc(sizeof(term_t*) * count);
    if (!elements) {
        state->failed = true;
        return false;
    }
    current = l;
    for (int i = 0; i < count; i++) {
        elements[i] = current->data.list.head;
        current = deref(state, current->data.list.tail);
    }
    
    /* Sort array */
    qsort(elements, count, sizeof(term_t*), compare_terms_for_sort);
    
    /* Remove duplicates and build result list */
    term_t* result = create_nil();
    for (int i = count - 1; i >= 0; i--) {
        if (i == 0 || term_compare(elements[i], elements[i-1]) != 0) {
            result = create_list(elements[i], result);
        }
    }
    
    free(elements);
    return unify(state, sorted, result);
}

bool msort_2(prolog_state_t* state, term_t* list, term_t* sorted) {
    term_t* l = deref(state, list);
    
    /* Count elements */
    int count = 0;
    term_t* current = l;
    while (current->type == TERM_LIST) {
        count++;
        current = deref(state, current->data.list.tail);
    }
    
    if (current->type != TERM_NIL) {
        state->failed = true;
        return false;
    }
    
    if (count == 0) {
        return unify(state, sorted, create_nil());
    }
    
    /* Copy elements to array */
    term_t** elements = malloc(sizeof(term_t*) * count);
    if (!elements) {
        state->failed = true;
        return false;
    }
    current = l;
    for (int i = 0; i < count; i++) {
        elements[i] = current->data.list.head;
        current = deref(state, current->data.list.tail);
    }
    
    /* Sort array (keeping duplicates) */
    qsort(elements, count, sizeof(term_t*), compare_terms_for_sort);
    
    /* Build result list */
    term_t* result = create_nil();
    for (int i = count - 1; i >= 0; i--) {
        result = create_list(elements[i], result);
    }
    
    free(elements);
    return unify(state, sorted, result);
}

bool keysort_2(prolog_state_t* state, term_t* list, term_t* sorted) {
    /* Sort list of Key-Value pairs by Key */
    term_t* l = deref(state, list);
    
    /* Count elements */
    int count = 0;
    term_t* current = l;
    while (current->type == TERM_LIST) {
        count++;
        current = deref(state, current->data.list.tail);
    }
    
    if (current->type != TERM_NIL) {
        state->failed = true;
        return false;
    }
    
    if (count == 0) {
        return unify(state, sorted, create_nil());
    }
    
    /* Copy elements to array */
    term_t** elements = malloc(sizeof(term_t*) * count);
    if (!elements) {
        state->failed = true;
        return false;
    }
    current = l;
    for (int i = 0; i < count; i++) {
        term_t* elem = deref(state, current->data.list.head);
        /* Check if element is Key-Value compound */
        if (elem->type != TERM_COMPOUND || 
            strcmp(elem->data.compound.functor, "-") != 0 ||
            elem->data.compound.arity != 2) {
            free(elements);
            state->failed = true;
            return false;
        }
        elements[i] = current->data.list.head;
        current = deref(state, current->data.list.tail);
    }
    
    /* Sort array by keys only */
    qsort(elements, count, sizeof(term_t*), compare_keys_for_keysort);
    
    /* Build result list */
    term_t* result = create_nil();
    for (int i = count - 1; i >= 0; i--) {
        result = create_list(elements[i], result);
    }
    
    free(elements);
    return unify(state, sorted, result);
}

/* Meta-call predicates (ISO/SWI) */
bool call_1(prolog_state_t* state, term_t* goal) {
    /* Call a goal dynamically */
    /* This requires runtime interpretation of the goal term */
    /* Simplified implementation - just succeeds for now */
    term_t* g = deref(state, goal);
    
    if (g->type == TERM_ATOM) {
        /* Atom with 0 arguments - would need to look up predicate */
        if (strcmp(g->data.atom, "true") == 0) {
            return true;
        } else if (strcmp(g->data.atom, "fail") == 0) {
            state->failed = true;
            return false;
        }
    } else if (g->type == TERM_COMPOUND) {
        /* Compound term - would need to look up predicate and call it */
        /* This is a placeholder */
    }
    
    /* For now, just succeed */
    return true;
}

bool call_2(prolog_state_t* state, term_t* closure, term_t* arg1) {
    /* Call closure with an additional argument */
    term_t* c = deref(state, closure);
    
    if (c->type == TERM_ATOM) {
        /* Create compound term functor(arg1) */
        term_t** args = malloc(sizeof(term_t*) * 1);
        args[0] = arg1;
        term_t* goal = create_compound(c->data.atom, 1, args);
        return call_1(state, goal);
    } else if (c->type == TERM_COMPOUND) {
        /* Add argument to existing compound */
        int new_arity = c->data.compound.arity + 1;
        term_t** args = malloc(sizeof(term_t*) * new_arity);
        for (int i = 0; i < c->data.compound.arity; i++) {
            args[i] = c->data.compound.args[i];
        }
        args[new_arity - 1] = arg1;
        term_t* goal = create_compound(c->data.compound.functor, new_arity, args);
        return call_1(state, goal);
    }
    
    state->failed = true;
    return false;
}

bool call_3(prolog_state_t* state, term_t* closure, term_t* arg1, term_t* arg2) {
    /* Call closure with two additional arguments */
    term_t* c = deref(state, closure);
    
    if (c->type == TERM_ATOM) {
        /* Create compound term functor(arg1, arg2) */
        term_t** args = malloc(sizeof(term_t*) * 2);
        args[0] = arg1;
        args[1] = arg2;
        term_t* goal = create_compound(c->data.atom, 2, args);
        return call_1(state, goal);
    } else if (c->type == TERM_COMPOUND) {
        /* Add arguments to existing compound */
        int new_arity = c->data.compound.arity + 2;
        term_t** args = malloc(sizeof(term_t*) * new_arity);
        for (int i = 0; i < c->data.compound.arity; i++) {
            args[i] = c->data.compound.args[i];
        }
        args[new_arity - 2] = arg1;
        args[new_arity - 1] = arg2;
        term_t* goal = create_compound(c->data.compound.functor, new_arity, args);
        return call_1(state, goal);
    }
    
    state->failed = true;
    return false;
}

bool apply_2(prolog_state_t* state, term_t* goal, term_t* args) {
    /* Apply goal with arguments from list */
    term_t* g = deref(state, goal);
    term_t* a = deref(state, args);
    
    /* Count arguments */
    int argc = 0;
    term_t* current = a;
    while (current->type == TERM_LIST) {
        argc++;
        current = deref(state, current->data.list.tail);
    }
    
    if (current->type != TERM_NIL) {
        state->failed = true;
        return false;
    }
    
    /* Build compound term with arguments */
    if (g->type == TERM_ATOM) {
        if (argc == 0) {
            return call_1(state, g);
        }
        
        term_t** arg_array = malloc(sizeof(term_t*) * argc);
        current = a;
        for (int i = 0; i < argc; i++) {
            arg_array[i] = current->data.list.head;
            current = deref(state, current->data.list.tail);
        }
        
        term_t* compound = create_compound(g->data.atom, argc, arg_array);
        return call_1(state, compound);
    } else if (g->type == TERM_COMPOUND) {
        /* Add arguments to existing compound */
        int new_arity = g->data.compound.arity + argc;
        term_t** arg_array = malloc(sizeof(term_t*) * new_arity);
        
        for (int i = 0; i < g->data.compound.arity; i++) {
            arg_array[i] = g->data.compound.args[i];
        }
        
        current = a;
        for (int i = 0; i < argc; i++) {
            arg_array[g->data.compound.arity + i] = current->data.list.head;
            current = deref(state, current->data.list.tail);
        }
        
        term_t* compound = create_compound(g->data.compound.functor, new_arity, arg_array);
        return call_1(state, compound);
    }
    
    state->failed = true;
    return false;
}

/* State initialization and cleanup */
void init_state(prolog_state_t* state) {
    state->choice_stack = NULL;
    state->bindings.bindings = NULL;
    state->bindings.size = 0;
    state->bindings.capacity = 0;
    state->cut_level = 0;
    state->failed = false;
    state->next_var_id = 1000;
}

void free_state(prolog_state_t* state) {
    /* Free choice points */
    while (state->choice_stack) {
        pop_choice_point(state);
    }
    
    /* Free bindings */
    free(state->bindings.bindings);
}

int main(int argc, char** argv) {
    prolog_state_t state;
    init_state(&state);
    
    printf("Prolog-to-C compiled program\n");
    
    /* Call main/0 predicate if it exists */
    main_0(&state);
    
    free_state(&state);
    return 0;
}
