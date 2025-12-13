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
void print_term(prolog_state_t* state, term_t* term);
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

bool city_1(prolog_state_t* state, term_t* arg1);
bool country_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool capital_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool list_length_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool reverse_list_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool sum_list_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool between_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3);
bool max_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3);
bool min_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3);
bool if_then_else_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3);
bool append_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3);
bool member_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool last_elem_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool nth_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3);
bool positive_numbers_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool map_double_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool all_cities_1(prolog_state_t* state, term_t* arg1);
bool all_countries_1(prolog_state_t* state, term_t* arg1);
bool find_capitals_1(prolog_state_t* state, term_t* arg1);
bool same_country_pair_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool factorial_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool fibonacci_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool gcd_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3);
bool insert_sorted_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3);
bool insertion_sort_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool main_0(prolog_state_t* state);
bool city_1(prolog_state_t* state, term_t* arg1) {
    /* Clause 1: city(london) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_atom("london"), arg1)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: city(paris) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_atom("paris"), arg1)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 3: city(tokyo) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_atom("tokyo"), arg1)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 4: city(newYork) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_atom("newYork"), arg1)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool country_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: country(uk,london) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_atom("uk"), arg1) &&
            unify(state, create_atom("london"), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: country(france,paris) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_atom("france"), arg1) &&
            unify(state, create_atom("paris"), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 3: country(japan,tokyo) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_atom("japan"), arg1) &&
            unify(state, create_atom("tokyo"), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 4: country(usa,newYork) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_atom("usa"), arg1) &&
            unify(state, create_atom("newYork"), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool capital_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: capital(_2772,_2774) :- country(_2772,_2774) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__2772 = create_var(state->next_var_id++);
        term_t* var__2774 = create_var(state->next_var_id++);
        if (unify(state, var__2772, arg1) &&
            unify(state, var__2774, arg2)) {

            do {
    if (!country_2(state, var__2772, var__2774)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool list_length_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: list_length([],0) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_nil(), arg1) &&
            unify(state, create_int(0), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: list_length([_2814|_2816],_2822) :- list_length(_2816,_2828),_2822 is _2828+1 */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__2814 = create_var(state->next_var_id++);
        term_t* var__2816 = create_var(state->next_var_id++);
        term_t* var__2822 = create_var(state->next_var_id++);
        term_t* var__2828 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__2814, var__2816), arg1) &&
            unify(state, var__2822, arg2)) {

            do {
    if (!list_length_2(state, var__2816, var__2828)) { state->failed = true; break; }
    if (!is_2(state, var__2822, create_compound("+", 2, (term_t*[]){var__2828, create_int(1)}))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool reverse_list_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: reverse_list([],[]) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_nil(), arg1) &&
            unify(state, create_nil(), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: reverse_list([_2880|_2882],_2888) :- reverse_list(_2882,_2894),append(_2894,[_2880],_2888) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__2880 = create_var(state->next_var_id++);
        term_t* var__2882 = create_var(state->next_var_id++);
        term_t* var__2888 = create_var(state->next_var_id++);
        term_t* var__2894 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__2880, var__2882), arg1) &&
            unify(state, var__2888, arg2)) {

            do {
    if (!reverse_list_2(state, var__2882, var__2894)) { state->failed = true; break; }
    if (!append_3(state, var__2894, create_list(var__2880, create_nil()), var__2888)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool sum_list_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: sum_list([],0) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_nil(), arg1) &&
            unify(state, create_int(0), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: sum_list([_2950|_2952],_2958) :- sum_list(_2952,_2964),_2958 is _2950+_2964 */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__2950 = create_var(state->next_var_id++);
        term_t* var__2952 = create_var(state->next_var_id++);
        term_t* var__2958 = create_var(state->next_var_id++);
        term_t* var__2964 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__2950, var__2952), arg1) &&
            unify(state, var__2958, arg2)) {

            do {
    if (!sum_list_2(state, var__2952, var__2964)) { state->failed = true; break; }
    if (!is_2(state, var__2958, create_compound("+", 2, (term_t*[]){var__2950, var__2964}))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool between_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: between(_3000,_3002,_3000) :- _3000=<_3002 */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3000 = create_var(state->next_var_id++);
        term_t* var__3002 = create_var(state->next_var_id++);
        if (unify(state, var__3000, arg1) &&
            unify(state, var__3002, arg2) &&
            unify(state, var__3000, arg3)) {

            do {
    if (!lte_2(state, var__3000, var__3002)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: between(_3028,_3030,_3032) :- _3028<_3030,_3048 is _3028+1,between(_3048,_3030,_3032) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3028 = create_var(state->next_var_id++);
        term_t* var__3030 = create_var(state->next_var_id++);
        term_t* var__3032 = create_var(state->next_var_id++);
        term_t* var__3048 = create_var(state->next_var_id++);
        if (unify(state, var__3028, arg1) &&
            unify(state, var__3030, arg2) &&
            unify(state, var__3032, arg3)) {

            do {
    if (!lt_2(state, var__3028, var__3030)) { state->failed = true; break; }
    if (!is_2(state, var__3048, create_compound("+", 2, (term_t*[]){var__3028, create_int(1)}))) { state->failed = true; break; }
    if (!between_3(state, var__3048, var__3030, var__3032)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool max_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: max(_3088,_3090,_3088) :- _3088>=_3090,! */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3088 = create_var(state->next_var_id++);
        term_t* var__3090 = create_var(state->next_var_id++);
        if (unify(state, var__3088, arg1) &&
            unify(state, var__3090, arg2) &&
            unify(state, var__3088, arg3)) {

            do {
    if (!gte_2(state, var__3088, var__3090)) { state->failed = true; break; }
    perform_cut(state);
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: max(_3122,_3124,_3124) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3122 = create_var(state->next_var_id++);
        term_t* var__3124 = create_var(state->next_var_id++);
        if (unify(state, var__3122, arg1) &&
            unify(state, var__3124, arg2) &&
            unify(state, var__3124, arg3)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool min_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: min(_3138,_3140,_3138) :- _3138=<_3140,! */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3138 = create_var(state->next_var_id++);
        term_t* var__3140 = create_var(state->next_var_id++);
        if (unify(state, var__3138, arg1) &&
            unify(state, var__3140, arg2) &&
            unify(state, var__3138, arg3)) {

            do {
    if (!lte_2(state, var__3138, var__3140)) { state->failed = true; break; }
    perform_cut(state);
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: min(_3172,_3174,_3174) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3172 = create_var(state->next_var_id++);
        term_t* var__3174 = create_var(state->next_var_id++);
        if (unify(state, var__3172, arg1) &&
            unify(state, var__3174, arg2) &&
            unify(state, var__3174, arg3)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool if_then_else_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: if_then_else(true,true,_3192) :- true,!,true */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3188 = create_var(state->next_var_id++);
        term_t* var__3190 = create_var(state->next_var_id++);
        term_t* var__3192 = create_var(state->next_var_id++);
        if (unify(state, var__3188, arg1) &&
            unify(state, var__3190, arg2) &&
            unify(state, var__3192, arg3)) {

            do {
    /* true */
    perform_cut(state);
    /* true */
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: if_then_else(_3222,_3224,true) :- true */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3222 = create_var(state->next_var_id++);
        term_t* var__3224 = create_var(state->next_var_id++);
        term_t* var__3226 = create_var(state->next_var_id++);
        if (unify(state, var__3222, arg1) &&
            unify(state, var__3224, arg2) &&
            unify(state, var__3226, arg3)) {

            do {
    /* true */
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool append_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: append([],_3246,_3246) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3246 = create_var(state->next_var_id++);
        if (unify(state, create_nil(), arg1) &&
            unify(state, var__3246, arg2) &&
            unify(state, var__3246, arg3)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: append([_3262|_3264],_3278,[_3262|_3272]) :- append(_3264,_3278,_3272) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3262 = create_var(state->next_var_id++);
        term_t* var__3264 = create_var(state->next_var_id++);
        term_t* var__3278 = create_var(state->next_var_id++);
        term_t* var__3272 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__3262, var__3264), arg1) &&
            unify(state, var__3278, arg2) &&
            unify(state, create_list(var__3262, var__3272), arg3)) {

            do {
    if (!append_3(state, var__3264, var__3278, var__3272)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool member_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: member(_3308,[_3308|_3310]) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3308 = create_var(state->next_var_id++);
        term_t* var__3310 = create_var(state->next_var_id++);
        if (unify(state, var__3308, arg1) &&
            unify(state, create_list(var__3308, var__3310), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: member(_3336,[_3330|_3332]) :- member(_3336,_3332) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3336 = create_var(state->next_var_id++);
        term_t* var__3330 = create_var(state->next_var_id++);
        term_t* var__3332 = create_var(state->next_var_id++);
        if (unify(state, var__3336, arg1) &&
            unify(state, create_list(var__3330, var__3332), arg2)) {

            do {
    if (!member_2(state, var__3336, var__3332)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool last_elem_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: last_elem([_3364],_3364) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3364 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__3364, create_nil()), arg1) &&
            unify(state, var__3364, arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: last_elem([_3386|_3388],_3394) :- last_elem(_3388,_3394) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3386 = create_var(state->next_var_id++);
        term_t* var__3388 = create_var(state->next_var_id++);
        term_t* var__3394 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__3386, var__3388), arg1) &&
            unify(state, var__3394, arg2)) {

            do {
    if (!last_elem_2(state, var__3388, var__3394)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool nth_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: nth(0,[_3420|_3422],_3420) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3420 = create_var(state->next_var_id++);
        term_t* var__3422 = create_var(state->next_var_id++);
        if (unify(state, create_int(0), arg1) &&
            unify(state, create_list(var__3420, var__3422), arg2) &&
            unify(state, var__3420, arg3)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: nth(_3450,[_3444|_3446],_3454) :- _3450>0,_3470 is _3450-1,nth(_3470,_3446,_3454) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3450 = create_var(state->next_var_id++);
        term_t* var__3444 = create_var(state->next_var_id++);
        term_t* var__3446 = create_var(state->next_var_id++);
        term_t* var__3454 = create_var(state->next_var_id++);
        term_t* var__3470 = create_var(state->next_var_id++);
        if (unify(state, var__3450, arg1) &&
            unify(state, create_list(var__3444, var__3446), arg2) &&
            unify(state, var__3454, arg3)) {

            do {
    if (!gt_2(state, var__3450, create_int(0))) { state->failed = true; break; }
    if (!is_2(state, var__3470, create_compound("-", 2, (term_t*[]){var__3450, create_int(1)}))) { state->failed = true; break; }
    if (!nth_3(state, var__3470, var__3446, var__3454)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool positive_numbers_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: positive_numbers([],[]) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_nil(), arg1) &&
            unify(state, create_nil(), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: positive_numbers([_3526|_3528],[_3526|_3536]) :- _3526>0,!,positive_numbers(_3528,_3536) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3526 = create_var(state->next_var_id++);
        term_t* var__3528 = create_var(state->next_var_id++);
        term_t* var__3536 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__3526, var__3528), arg1) &&
            unify(state, create_list(var__3526, var__3536), arg2)) {

            do {
    if (!gt_2(state, var__3526, create_int(0))) { state->failed = true; break; }
    perform_cut(state);
    if (!positive_numbers_2(state, var__3528, var__3536)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 3: positive_numbers([_3586|_3588],_3594) :- positive_numbers(_3588,_3594) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__3586 = create_var(state->next_var_id++);
        term_t* var__3588 = create_var(state->next_var_id++);
        term_t* var__3594 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__3586, var__3588), arg1) &&
            unify(state, var__3594, arg2)) {

            do {
    if (!positive_numbers_2(state, var__3588, var__3594)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool map_double_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: map_double([],[]) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_nil(), arg1) &&
            unify(state, create_nil(), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: map_double([_754|_756],[_762|_764]) :- _762 is _754*2,map_double(_756,_764) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__754 = create_var(state->next_var_id++);
        term_t* var__756 = create_var(state->next_var_id++);
        term_t* var__762 = create_var(state->next_var_id++);
        term_t* var__764 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__754, var__756), arg1) &&
            unify(state, create_list(var__762, var__764), arg2)) {

            do {
    if (!is_2(state, var__762, create_compound("*", 2, (term_t*[]){var__754, create_int(2)}))) { state->failed = true; break; }
    if (!map_double_2(state, var__756, var__764)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool all_cities_1(prolog_state_t* state, term_t* arg1) {
    /* Clause 1: all_cities(_804) :- findall(_808,city(_808),_804) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__804 = create_var(state->next_var_id++);
        term_t* var__808 = create_var(state->next_var_id++);
        if (unify(state, var__804, arg1)) {

            do {
    /* findall(_808, city(_808), _804) */
    {
        term_t** solutions = NULL;
        int solution_count = 0;
        prolog_state_t findall_state;
        init_state(&findall_state);
        
        /* Enumerate solutions */
        while (true) {
    if (!city_1(state, var__808)) { state->failed = true; break; }

            if (state->failed) break;
            
            /* Collect solution */
            solution_count++;
            solutions = realloc(solutions, sizeof(term_t*) * solution_count);
            /* Store template instance */
            
            /* Force backtracking */
            if (!pop_choice_point(&findall_state)) break;
        }
        
        /* Build result list */
        free_state(&findall_state);
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
bool all_countries_1(prolog_state_t* state, term_t* arg1) {
    /* Clause 1: all_countries(_826) :- findall(_830,country(_830,_832),_826) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__826 = create_var(state->next_var_id++);
        term_t* var__830 = create_var(state->next_var_id++);
        term_t* var__832 = create_var(state->next_var_id++);
        if (unify(state, var__826, arg1)) {

            do {
    /* findall(_830, country(_830,_832), _826) */
    {
        term_t** solutions = NULL;
        int solution_count = 0;
        prolog_state_t findall_state;
        init_state(&findall_state);
        
        /* Enumerate solutions */
        while (true) {
    if (!country_2(state, var__830, var__832)) { state->failed = true; break; }

            if (state->failed) break;
            
            /* Collect solution */
            solution_count++;
            solutions = realloc(solutions, sizeof(term_t*) * solution_count);
            /* Store template instance */
            
            /* Force backtracking */
            if (!pop_choice_point(&findall_state)) break;
        }
        
        /* Build result list */
        free_state(&findall_state);
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
bool find_capitals_1(prolog_state_t* state, term_t* arg1) {
    /* Clause 1: find_capitals(_850) :- findall(capital(_854,_856),capital(_854,_856),_850) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__850 = create_var(state->next_var_id++);
        term_t* var__854 = create_var(state->next_var_id++);
        term_t* var__856 = create_var(state->next_var_id++);
        if (unify(state, var__850, arg1)) {

            do {
    /* findall(capital(_854,_856), capital(_854,_856), _850) */
    {
        term_t** solutions = NULL;
        int solution_count = 0;
        prolog_state_t findall_state;
        init_state(&findall_state);
        
        /* Enumerate solutions */
        while (true) {
    if (!capital_2(state, var__854, var__856)) { state->failed = true; break; }

            if (state->failed) break;
            
            /* Collect solution */
            solution_count++;
            solutions = realloc(solutions, sizeof(term_t*) * solution_count);
            /* Store template instance */
            
            /* Force backtracking */
            if (!pop_choice_point(&findall_state)) break;
        }
        
        /* Build result list */
        free_state(&findall_state);
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
bool same_country_pair_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: same_country_pair(_880,_882) :- country(_886,_880),country(_886,_882),_880\=_882 */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__880 = create_var(state->next_var_id++);
        term_t* var__882 = create_var(state->next_var_id++);
        term_t* var__886 = create_var(state->next_var_id++);
        if (unify(state, var__880, arg1) &&
            unify(state, var__882, arg2)) {

            do {
    if (!country_2(state, var__886, var__880)) { state->failed = true; break; }
    if (!country_2(state, var__886, var__882)) { state->failed = true; break; }
    if (!neq_2(state, var__880, var__882)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool factorial_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: factorial(0,1) :- ! */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_int(0), arg1) &&
            unify(state, create_int(1), arg2)) {

            do {
    perform_cut(state);
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: factorial(_934,_936) :- _934>0,_952 is _934-1,factorial(_952,_960),_936 is _934*_960 */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__934 = create_var(state->next_var_id++);
        term_t* var__936 = create_var(state->next_var_id++);
        term_t* var__952 = create_var(state->next_var_id++);
        term_t* var__960 = create_var(state->next_var_id++);
        if (unify(state, var__934, arg1) &&
            unify(state, var__936, arg2)) {

            do {
    if (!gt_2(state, var__934, create_int(0))) { state->failed = true; break; }
    if (!is_2(state, var__952, create_compound("-", 2, (term_t*[]){var__934, create_int(1)}))) { state->failed = true; break; }
    if (!factorial_2(state, var__952, var__960)) { state->failed = true; break; }
    if (!is_2(state, var__936, create_compound("*", 2, (term_t*[]){var__934, var__960}))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool fibonacci_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: fibonacci(0,0) :- ! */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_int(0), arg1) &&
            unify(state, create_int(0), arg2)) {

            do {
    perform_cut(state);
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: fibonacci(1,1) :- ! */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_int(1), arg1) &&
            unify(state, create_int(1), arg2)) {

            do {
    perform_cut(state);
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 3: fibonacci(_1024,_1026) :- _1024>1,_1042 is _1024-1,_1054 is _1024-2,fibonacci(_1042,_1062),fibonacci(_1054,_1068),_1026 is _1062+_1068 */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1024 = create_var(state->next_var_id++);
        term_t* var__1026 = create_var(state->next_var_id++);
        term_t* var__1042 = create_var(state->next_var_id++);
        term_t* var__1054 = create_var(state->next_var_id++);
        term_t* var__1062 = create_var(state->next_var_id++);
        term_t* var__1068 = create_var(state->next_var_id++);
        if (unify(state, var__1024, arg1) &&
            unify(state, var__1026, arg2)) {

            do {
    if (!gt_2(state, var__1024, create_int(1))) { state->failed = true; break; }
    if (!is_2(state, var__1042, create_compound("-", 2, (term_t*[]){var__1024, create_int(1)}))) { state->failed = true; break; }
    if (!is_2(state, var__1054, create_compound("-", 2, (term_t*[]){var__1024, create_int(2)}))) { state->failed = true; break; }
    if (!fibonacci_2(state, var__1042, var__1062)) { state->failed = true; break; }
    if (!fibonacci_2(state, var__1054, var__1068)) { state->failed = true; break; }
    if (!is_2(state, var__1026, create_compound("+", 2, (term_t*[]){var__1062, var__1068}))) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool gcd_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: gcd(_1120,0,_1120) :- ! */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1120 = create_var(state->next_var_id++);
        if (unify(state, var__1120, arg1) &&
            unify(state, create_int(0), arg2) &&
            unify(state, var__1120, arg3)) {

            do {
    perform_cut(state);
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: gcd(_1134,_1136,_1138) :- _1136>0,_1154 is _1134 mod _1136,gcd(_1136,_1154,_1138) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1134 = create_var(state->next_var_id++);
        term_t* var__1136 = create_var(state->next_var_id++);
        term_t* var__1138 = create_var(state->next_var_id++);
        term_t* var__1154 = create_var(state->next_var_id++);
        if (unify(state, var__1134, arg1) &&
            unify(state, var__1136, arg2) &&
            unify(state, var__1138, arg3)) {

            do {
    if (!gt_2(state, var__1136, create_int(0))) { state->failed = true; break; }
    if (!is_2(state, var__1154, create_compound("mod", 2, (term_t*[]){var__1134, var__1136}))) { state->failed = true; break; }
    if (!gcd_3(state, var__1136, var__1154, var__1138)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool insert_sorted_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: insert_sorted(_1188,[],[_1188]) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1188 = create_var(state->next_var_id++);
        if (unify(state, var__1188, arg1) &&
            unify(state, create_nil(), arg2) &&
            unify(state, create_list(var__1188, create_nil()), arg3)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: insert_sorted(_1212,[_1204|_1206],[_1212,_1204|_1206]) :- _1212=<_1204,! */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1212 = create_var(state->next_var_id++);
        term_t* var__1204 = create_var(state->next_var_id++);
        term_t* var__1206 = create_var(state->next_var_id++);
        if (unify(state, var__1212, arg1) &&
            unify(state, create_list(var__1204, var__1206), arg2) &&
            unify(state, create_list(var__1212, create_list(var__1204, var__1206)), arg3)) {

            do {
    if (!lte_2(state, var__1212, var__1204)) { state->failed = true; break; }
    perform_cut(state);
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 3: insert_sorted(_1266,[_1252|_1254],[_1252|_1262]) :- _1266>_1252,insert_sorted(_1266,_1254,_1262) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1266 = create_var(state->next_var_id++);
        term_t* var__1252 = create_var(state->next_var_id++);
        term_t* var__1254 = create_var(state->next_var_id++);
        term_t* var__1262 = create_var(state->next_var_id++);
        if (unify(state, var__1266, arg1) &&
            unify(state, create_list(var__1252, var__1254), arg2) &&
            unify(state, create_list(var__1252, var__1262), arg3)) {

            do {
    if (!gt_2(state, var__1266, var__1252)) { state->failed = true; break; }
    if (!insert_sorted_3(state, var__1266, var__1254, var__1262)) { state->failed = true; break; }
            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }

    return false; /* No clause matched */
}
bool insertion_sort_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: insertion_sort([],[]) */
    {
        int saved_bindings_size = state->bindings.size;
        if (unify(state, create_nil(), arg1) &&
            unify(state, create_nil(), arg2)) {

            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
    /* Clause 2: insertion_sort([_1308|_1310],_1316) :- insertion_sort(_1310,_1322),insert_sorted(_1308,_1322,_1316) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1308 = create_var(state->next_var_id++);
        term_t* var__1310 = create_var(state->next_var_id++);
        term_t* var__1316 = create_var(state->next_var_id++);
        term_t* var__1322 = create_var(state->next_var_id++);
        if (unify(state, create_list(var__1308, var__1310), arg1) &&
            unify(state, var__1316, arg2)) {

            do {
    if (!insertion_sort_2(state, var__1310, var__1322)) { state->failed = true; break; }
    if (!insert_sorted_3(state, var__1308, var__1322, var__1316)) { state->failed = true; break; }
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
    /* Clause 1: main :- write(=== PL2C Comprehensive Test ===

),write(Test 1: Facts and Rules
),capital(uk,_1356),format(  Capital of UK: ~w
,[_1356]),write(
Test 2: List Operations
),append([1,2],[3,4],_1410),format(  append([1,2], [3,4]) = ~w
,[_1410]),member(2,[1,2,3]),write(  member(2, [1,2,3]) succeeded
),write(
Test 3: Arithmetic
),factorial(5,_1464),format(  factorial(5) = ~w
,[_1464]),write(
Test 4: Cut Behavior
),max(10,5,_1490),format(  max(10, 5) = ~w
,[_1490]),min(10,5,_1512),format(  min(10, 5) = ~w
,[_1512]),write(
Test 5: Recursive Predicates
),reverse_list([1,2,3],_1556),format(  reverse([1,2,3]) = ~w
,[_1556]),write(
=== All Tests Completed ===
) */
    {
        int saved_bindings_size = state->bindings.size;
        term_t* var__1356 = create_var(state->next_var_id++);
        term_t* var__1410 = create_var(state->next_var_id++);
        term_t* var__1464 = create_var(state->next_var_id++);
        term_t* var__1490 = create_var(state->next_var_id++);
        term_t* var__1512 = create_var(state->next_var_id++);
        term_t* var__1556 = create_var(state->next_var_id++);
        if (true) {

            do {
    if (!write_1(state, create_atom("=== PL2C Comprehensive Test ===\n\n"))) { state->failed = true; break; }
    if (!write_1(state, create_atom("Test 1: Facts and Rules\n"))) { state->failed = true; break; }
    if (!capital_2(state, create_atom("uk"), var__1356)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  Capital of UK: ~w\n"), create_list(var__1356, create_nil()))) { state->failed = true; break; }
    if (!write_1(state, create_atom("\nTest 2: List Operations\n"))) { state->failed = true; break; }
    if (!append_3(state, create_list(create_int(1), create_list(create_int(2), create_nil())), create_list(create_int(3), create_list(create_int(4), create_nil())), var__1410)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  append([1,2], [3,4]) = ~w\n"), create_list(var__1410, create_nil()))) { state->failed = true; break; }
    if (!member_2(state, create_int(2), create_list(create_int(1), create_list(create_int(2), create_list(create_int(3), create_nil()))))) { state->failed = true; break; }
    if (!write_1(state, create_atom("  member(2, [1,2,3]) succeeded\n"))) { state->failed = true; break; }
    if (!write_1(state, create_atom("\nTest 3: Arithmetic\n"))) { state->failed = true; break; }
    if (!factorial_2(state, create_int(5), var__1464)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  factorial(5) = ~w\n"), create_list(var__1464, create_nil()))) { state->failed = true; break; }
    if (!write_1(state, create_atom("\nTest 4: Cut Behavior\n"))) { state->failed = true; break; }
    if (!max_3(state, create_int(10), create_int(5), var__1490)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  max(10, 5) = ~w\n"), create_list(var__1490, create_nil()))) { state->failed = true; break; }
    if (!min_3(state, create_int(10), create_int(5), var__1512)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  min(10, 5) = ~w\n"), create_list(var__1512, create_nil()))) { state->failed = true; break; }
    if (!write_1(state, create_atom("\nTest 5: Recursive Predicates\n"))) { state->failed = true; break; }
    if (!reverse_list_2(state, create_list(create_int(1), create_list(create_int(2), create_list(create_int(3), create_nil()))), var__1556)) { state->failed = true; break; }
    if (!format_2(state, create_atom("  reverse([1,2,3]) = ~w\n"), create_list(var__1556, create_nil()))) { state->failed = true; break; }
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
void print_term(prolog_state_t* state, term_t* term) {
    term = deref(state, term);
    
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
            print_term(state, current->data.list.head);
            current = deref(state, current->data.list.tail);
            if (current->type == TERM_LIST) {
                printf(", ");
            }
        }
        if (current->type != TERM_NIL) {
            printf("|");
            print_term(state, current);
        }
        printf("]");
    } else if (term->type == TERM_VAR) {
        printf("_%d", term->data.var_id);
    } else if (term->type == TERM_COMPOUND) {
        printf("%s(", term->data.compound.functor);
        for (int i = 0; i < term->data.compound.arity; i++) {
            print_term(state, term->data.compound.args[i]);
            if (i < term->data.compound.arity - 1) {
                printf(", ");
            }
        }
        printf(")");
    }
}

bool write_1(prolog_state_t* state, term_t* arg1) {
    term_t* t = deref(state, arg1);
    print_term(state, t);
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
                print_term(state, current_arg->data.list.head);
                current_arg = deref(state, current_arg->data.list.tail);
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
