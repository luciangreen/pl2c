#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
        if (!unify(state, create_atom("london"), arg1)) return false;

        return true;
    }
    /* Clause 2: city(paris) */
    {
        if (!unify(state, create_atom("paris"), arg1)) return false;

        return true;
    }
    /* Clause 3: city(tokyo) */
    {
        if (!unify(state, create_atom("tokyo"), arg1)) return false;

        return true;
    }
    /* Clause 4: city(newYork) */
    {
        if (!unify(state, create_atom("newYork"), arg1)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool country_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: country(uk,london) */
    {
        if (!unify(state, create_atom("uk"), arg1)) return false;
        if (!unify(state, create_atom("london"), arg2)) return false;

        return true;
    }
    /* Clause 2: country(france,paris) */
    {
        if (!unify(state, create_atom("france"), arg1)) return false;
        if (!unify(state, create_atom("paris"), arg2)) return false;

        return true;
    }
    /* Clause 3: country(japan,tokyo) */
    {
        if (!unify(state, create_atom("japan"), arg1)) return false;
        if (!unify(state, create_atom("tokyo"), arg2)) return false;

        return true;
    }
    /* Clause 4: country(usa,newYork) */
    {
        if (!unify(state, create_atom("usa"), arg1)) return false;
        if (!unify(state, create_atom("newYork"), arg2)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool capital_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: capital(_2772,_2774) :- country(_2772,_2774) */
    {
        term_t* var__2772 = create_var(0);
        term_t* var__2774 = create_var(1);
        if (!unify(state, var__2772, arg1)) return false;
        if (!unify(state, var__2774, arg2)) return false;
    if (!country_2(state, var__2772, var__2774)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool list_length_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: list_length([],0) */
    {
        if (!unify(state, create_nil(), arg1)) return false;
        if (!unify(state, create_int(0), arg2)) return false;

        return true;
    }
    /* Clause 2: list_length([_2814|_2816],_2822) :- list_length(_2816,_2828),_2822 is _2828+1 */
    {
        term_t* var__2814 = create_var(0);
        term_t* var__2816 = create_var(1);
        term_t* var__2822 = create_var(2);
        term_t* var__2828 = create_var(3);
        if (!unify(state, create_list(var__2814, var__2816), arg1)) return false;
        if (!unify(state, var__2822, arg2)) return false;
    if (!list_length_2(state, var__2816, var__2828)) return false;
    if (!is_2(state, var__2822, create_compound("+", 2, (term_t*[]){var__2828, create_int(1)}))) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool reverse_list_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: reverse_list([],[]) */
    {
        if (!unify(state, create_nil(), arg1)) return false;
        if (!unify(state, create_nil(), arg2)) return false;

        return true;
    }
    /* Clause 2: reverse_list([_2880|_2882],_2888) :- reverse_list(_2882,_2894),append(_2894,[_2880],_2888) */
    {
        term_t* var__2880 = create_var(0);
        term_t* var__2882 = create_var(1);
        term_t* var__2888 = create_var(2);
        term_t* var__2894 = create_var(3);
        if (!unify(state, create_list(var__2880, var__2882), arg1)) return false;
        if (!unify(state, var__2888, arg2)) return false;
    if (!reverse_list_2(state, var__2882, var__2894)) return false;
    if (!append_3(state, var__2894, create_list(var__2880, create_nil()), var__2888)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool sum_list_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: sum_list([],0) */
    {
        if (!unify(state, create_nil(), arg1)) return false;
        if (!unify(state, create_int(0), arg2)) return false;

        return true;
    }
    /* Clause 2: sum_list([_2950|_2952],_2958) :- sum_list(_2952,_2964),_2958 is _2950+_2964 */
    {
        term_t* var__2950 = create_var(0);
        term_t* var__2952 = create_var(1);
        term_t* var__2958 = create_var(2);
        term_t* var__2964 = create_var(3);
        if (!unify(state, create_list(var__2950, var__2952), arg1)) return false;
        if (!unify(state, var__2958, arg2)) return false;
    if (!sum_list_2(state, var__2952, var__2964)) return false;
    if (!is_2(state, var__2958, create_compound("+", 2, (term_t*[]){var__2950, var__2964}))) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool between_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: between(_3000,_3002,_3000) :- _3000=<_3002 */
    {
        term_t* var__3000 = create_var(0);
        term_t* var__3002 = create_var(1);
        if (!unify(state, var__3000, arg1)) return false;
        if (!unify(state, var__3002, arg2)) return false;
        if (!unify(state, var__3000, arg3)) return false;
    if (!lte_2(state, var__3000, var__3002)) return false;

        return true;
    }
    /* Clause 2: between(_3028,_3030,_3032) :- _3028<_3030,_3048 is _3028+1,between(_3048,_3030,_3032) */
    {
        term_t* var__3028 = create_var(0);
        term_t* var__3030 = create_var(1);
        term_t* var__3032 = create_var(2);
        term_t* var__3048 = create_var(3);
        if (!unify(state, var__3028, arg1)) return false;
        if (!unify(state, var__3030, arg2)) return false;
        if (!unify(state, var__3032, arg3)) return false;
    if (!lt_2(state, var__3028, var__3030)) return false;
    if (!is_2(state, var__3048, create_compound("+", 2, (term_t*[]){var__3028, create_int(1)}))) return false;
    if (!between_3(state, var__3048, var__3030, var__3032)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool max_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: max(_3088,_3090,_3088) :- _3088>=_3090,! */
    {
        term_t* var__3088 = create_var(0);
        term_t* var__3090 = create_var(1);
        if (!unify(state, var__3088, arg1)) return false;
        if (!unify(state, var__3090, arg2)) return false;
        if (!unify(state, var__3088, arg3)) return false;
    if (!gte_2(state, var__3088, var__3090)) return false;
    perform_cut(state);

        return true;
    }
    /* Clause 2: max(_3122,_3124,_3124) */
    {
        term_t* var__3122 = create_var(0);
        term_t* var__3124 = create_var(1);
        if (!unify(state, var__3122, arg1)) return false;
        if (!unify(state, var__3124, arg2)) return false;
        if (!unify(state, var__3124, arg3)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool min_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: min(_3138,_3140,_3138) :- _3138=<_3140,! */
    {
        term_t* var__3138 = create_var(0);
        term_t* var__3140 = create_var(1);
        if (!unify(state, var__3138, arg1)) return false;
        if (!unify(state, var__3140, arg2)) return false;
        if (!unify(state, var__3138, arg3)) return false;
    if (!lte_2(state, var__3138, var__3140)) return false;
    perform_cut(state);

        return true;
    }
    /* Clause 2: min(_3172,_3174,_3174) */
    {
        term_t* var__3172 = create_var(0);
        term_t* var__3174 = create_var(1);
        if (!unify(state, var__3172, arg1)) return false;
        if (!unify(state, var__3174, arg2)) return false;
        if (!unify(state, var__3174, arg3)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool if_then_else_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: if_then_else(true,true,_3192) :- true,!,true */
    {
        term_t* var__3188 = create_var(0);
        term_t* var__3190 = create_var(1);
        term_t* var__3192 = create_var(2);
        if (!unify(state, var__3188, arg1)) return false;
        if (!unify(state, var__3190, arg2)) return false;
        if (!unify(state, var__3192, arg3)) return false;
    /* true */
    perform_cut(state);
    /* true */

        return true;
    }
    /* Clause 2: if_then_else(_3222,_3224,true) :- true */
    {
        term_t* var__3222 = create_var(0);
        term_t* var__3224 = create_var(1);
        term_t* var__3226 = create_var(2);
        if (!unify(state, var__3222, arg1)) return false;
        if (!unify(state, var__3224, arg2)) return false;
        if (!unify(state, var__3226, arg3)) return false;
    /* true */

        return true;
    }

    return false; /* No clause matched */
}
bool append_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: append([],_3246,_3246) */
    {
        term_t* var__3246 = create_var(0);
        if (!unify(state, create_nil(), arg1)) return false;
        if (!unify(state, var__3246, arg2)) return false;
        if (!unify(state, var__3246, arg3)) return false;

        return true;
    }
    /* Clause 2: append([_3262|_3264],_3278,[_3262|_3272]) :- append(_3264,_3278,_3272) */
    {
        term_t* var__3262 = create_var(0);
        term_t* var__3264 = create_var(1);
        term_t* var__3272 = create_var(2);
        term_t* var__3278 = create_var(3);
        if (!unify(state, create_list(var__3262, var__3264), arg1)) return false;
        if (!unify(state, var__3278, arg2)) return false;
        if (!unify(state, create_list(var__3262, var__3272), arg3)) return false;
    if (!append_3(state, var__3264, var__3278, var__3272)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool member_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: member(_3308,[_3308|_3310]) */
    {
        term_t* var__3308 = create_var(0);
        term_t* var__3310 = create_var(1);
        if (!unify(state, var__3308, arg1)) return false;
        if (!unify(state, create_list(var__3308, var__3310), arg2)) return false;

        return true;
    }
    /* Clause 2: member(_3336,[_3330|_3332]) :- member(_3336,_3332) */
    {
        term_t* var__3330 = create_var(0);
        term_t* var__3332 = create_var(1);
        term_t* var__3336 = create_var(2);
        if (!unify(state, var__3336, arg1)) return false;
        if (!unify(state, create_list(var__3330, var__3332), arg2)) return false;
    if (!member_2(state, var__3336, var__3332)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool last_elem_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: last_elem([_3364],_3364) */
    {
        term_t* var__3364 = create_var(0);
        if (!unify(state, create_list(var__3364, create_nil()), arg1)) return false;
        if (!unify(state, var__3364, arg2)) return false;

        return true;
    }
    /* Clause 2: last_elem([_3386|_3388],_3394) :- last_elem(_3388,_3394) */
    {
        term_t* var__3386 = create_var(0);
        term_t* var__3388 = create_var(1);
        term_t* var__3394 = create_var(2);
        if (!unify(state, create_list(var__3386, var__3388), arg1)) return false;
        if (!unify(state, var__3394, arg2)) return false;
    if (!last_elem_2(state, var__3388, var__3394)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool nth_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: nth(0,[_588|_590],_588) */
    {
        term_t* var__3420 = create_var(0);
        term_t* var__3422 = create_var(1);
        if (!unify(state, create_int(0), arg1)) return false;
        if (!unify(state, create_list(var__3420, var__3422), arg2)) return false;
        if (!unify(state, var__3420, arg3)) return false;

        return true;
    }
    /* Clause 2: nth(_610,[_604|_606],_614) :- _610>0,_630 is _610-1,nth(_630,_606,_614) */
    {
        term_t* var__604 = create_var(0);
        term_t* var__606 = create_var(1);
        term_t* var__610 = create_var(2);
        term_t* var__614 = create_var(3);
        term_t* var__630 = create_var(4);
        if (!unify(state, var__610, arg1)) return false;
        if (!unify(state, create_list(var__604, var__606), arg2)) return false;
        if (!unify(state, var__614, arg3)) return false;
    if (!gt_2(state, var__610, create_int(0))) return false;
    if (!is_2(state, var__630, create_compound("-", 2, (term_t*[]){var__610, create_int(1)}))) return false;
    if (!nth_3(state, var__630, var__606, var__614)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool positive_numbers_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: positive_numbers([],[]) */
    {
        if (!unify(state, create_nil(), arg1)) return false;
        if (!unify(state, create_nil(), arg2)) return false;

        return true;
    }
    /* Clause 2: positive_numbers([_670|_672],[_670|_680]) :- _670>0,!,positive_numbers(_672,_680) */
    {
        term_t* var__670 = create_var(0);
        term_t* var__672 = create_var(1);
        term_t* var__680 = create_var(2);
        if (!unify(state, create_list(var__670, var__672), arg1)) return false;
        if (!unify(state, create_list(var__670, var__680), arg2)) return false;
    if (!gt_2(state, var__670, create_int(0))) return false;
    perform_cut(state);
    if (!positive_numbers_2(state, var__672, var__680)) return false;

        return true;
    }
    /* Clause 3: positive_numbers([_722|_724],_730) :- positive_numbers(_724,_730) */
    {
        term_t* var__722 = create_var(0);
        term_t* var__724 = create_var(1);
        term_t* var__730 = create_var(2);
        if (!unify(state, create_list(var__722, var__724), arg1)) return false;
        if (!unify(state, var__730, arg2)) return false;
    if (!positive_numbers_2(state, var__724, var__730)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool map_double_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: map_double([],[]) */
    {
        if (!unify(state, create_nil(), arg1)) return false;
        if (!unify(state, create_nil(), arg2)) return false;

        return true;
    }
    /* Clause 2: map_double([_754|_756],[_762|_764]) :- _762 is _754*2,map_double(_756,_764) */
    {
        term_t* var__754 = create_var(0);
        term_t* var__756 = create_var(1);
        term_t* var__762 = create_var(2);
        term_t* var__764 = create_var(3);
        if (!unify(state, create_list(var__754, var__756), arg1)) return false;
        if (!unify(state, create_list(var__762, var__764), arg2)) return false;
    if (!is_2(state, var__762, create_compound("*", 2, (term_t*[]){var__754, create_int(2)}))) return false;
    if (!map_double_2(state, var__756, var__764)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool all_cities_1(prolog_state_t* state, term_t* arg1) {
    /* Clause 1: all_cities(_804) :- findall(_808,city(_808),_804) */
    {
        term_t* var__804 = create_var(0);
        term_t* var__808 = create_var(1);
        if (!unify(state, var__804, arg1)) return false;
    /* findall(_808, city(_808), _804) */
    {
        term_t** solutions = NULL;
        int solution_count = 0;
        prolog_state_t findall_state;
        init_state(&findall_state);
        
        /* Enumerate solutions */
        while (true) {
    if (!city_1(state, var__808)) return false;

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

        return true;
    }

    return false; /* No clause matched */
}
bool all_countries_1(prolog_state_t* state, term_t* arg1) {
    /* Clause 1: all_countries(_826) :- findall(_830,country(_830,_832),_826) */
    {
        term_t* var__826 = create_var(0);
        term_t* var__830 = create_var(1);
        term_t* var__832 = create_var(2);
        if (!unify(state, var__826, arg1)) return false;
    /* findall(_830, country(_830,_832), _826) */
    {
        term_t** solutions = NULL;
        int solution_count = 0;
        prolog_state_t findall_state;
        init_state(&findall_state);
        
        /* Enumerate solutions */
        while (true) {
    if (!country_2(state, var__830, var__832)) return false;

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

        return true;
    }

    return false; /* No clause matched */
}
bool find_capitals_1(prolog_state_t* state, term_t* arg1) {
    /* Clause 1: find_capitals(_850) :- findall(capital(_854,_856),capital(_854,_856),_850) */
    {
        term_t* var__850 = create_var(0);
        term_t* var__854 = create_var(1);
        term_t* var__856 = create_var(2);
        if (!unify(state, var__850, arg1)) return false;
    /* findall(capital(_854,_856), capital(_854,_856), _850) */
    {
        term_t** solutions = NULL;
        int solution_count = 0;
        prolog_state_t findall_state;
        init_state(&findall_state);
        
        /* Enumerate solutions */
        while (true) {
    if (!capital_2(state, var__854, var__856)) return false;

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

        return true;
    }

    return false; /* No clause matched */
}
bool same_country_pair_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: same_country_pair(_880,_882) :- country(_886,_880),country(_886,_882),_880\=_882 */
    {
        term_t* var__880 = create_var(0);
        term_t* var__882 = create_var(1);
        term_t* var__886 = create_var(2);
        if (!unify(state, var__880, arg1)) return false;
        if (!unify(state, var__882, arg2)) return false;
    if (!country_2(state, var__886, var__880)) return false;
    if (!country_2(state, var__886, var__882)) return false;
    if (!neq_2(state, var__880, var__882)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool factorial_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: factorial(0,1) :- ! */
    {
        if (!unify(state, create_int(0), arg1)) return false;
        if (!unify(state, create_int(1), arg2)) return false;
    perform_cut(state);

        return true;
    }
    /* Clause 2: factorial(_934,_936) :- _934>0,_952 is _934-1,factorial(_952,_960),_936 is _934*_960 */
    {
        term_t* var__934 = create_var(0);
        term_t* var__936 = create_var(1);
        term_t* var__952 = create_var(2);
        term_t* var__960 = create_var(3);
        if (!unify(state, var__934, arg1)) return false;
        if (!unify(state, var__936, arg2)) return false;
    if (!gt_2(state, var__934, create_int(0))) return false;
    if (!is_2(state, var__952, create_compound("-", 2, (term_t*[]){var__934, create_int(1)}))) return false;
    if (!factorial_2(state, var__952, var__960)) return false;
    if (!is_2(state, var__936, create_compound("*", 2, (term_t*[]){var__934, var__960}))) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool fibonacci_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: fibonacci(0,0) :- ! */
    {
        if (!unify(state, create_int(0), arg1)) return false;
        if (!unify(state, create_int(0), arg2)) return false;
    perform_cut(state);

        return true;
    }
    /* Clause 2: fibonacci(1,1) :- ! */
    {
        if (!unify(state, create_int(1), arg1)) return false;
        if (!unify(state, create_int(1), arg2)) return false;
    perform_cut(state);

        return true;
    }
    /* Clause 3: fibonacci(_1024,_1026) :- _1024>1,_1042 is _1024-1,_1054 is _1024-2,fibonacci(_1042,_1062),fibonacci(_1054,_1068),_1026 is _1062+_1068 */
    {
        term_t* var__1024 = create_var(0);
        term_t* var__1026 = create_var(1);
        term_t* var__1042 = create_var(2);
        term_t* var__1054 = create_var(3);
        term_t* var__1062 = create_var(4);
        term_t* var__1068 = create_var(5);
        if (!unify(state, var__1024, arg1)) return false;
        if (!unify(state, var__1026, arg2)) return false;
    if (!gt_2(state, var__1024, create_int(1))) return false;
    if (!is_2(state, var__1042, create_compound("-", 2, (term_t*[]){var__1024, create_int(1)}))) return false;
    if (!is_2(state, var__1054, create_compound("-", 2, (term_t*[]){var__1024, create_int(2)}))) return false;
    if (!fibonacci_2(state, var__1042, var__1062)) return false;
    if (!fibonacci_2(state, var__1054, var__1068)) return false;
    if (!is_2(state, var__1026, create_compound("+", 2, (term_t*[]){var__1062, var__1068}))) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool gcd_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: gcd(_1120,0,_1120) :- ! */
    {
        term_t* var__1120 = create_var(0);
        if (!unify(state, var__1120, arg1)) return false;
        if (!unify(state, create_int(0), arg2)) return false;
        if (!unify(state, var__1120, arg3)) return false;
    perform_cut(state);

        return true;
    }
    /* Clause 2: gcd(_1134,_1136,_1138) :- _1136>0,_1154 is _1134 mod _1136,gcd(_1136,_1154,_1138) */
    {
        term_t* var__1134 = create_var(0);
        term_t* var__1136 = create_var(1);
        term_t* var__1138 = create_var(2);
        term_t* var__1154 = create_var(3);
        if (!unify(state, var__1134, arg1)) return false;
        if (!unify(state, var__1136, arg2)) return false;
        if (!unify(state, var__1138, arg3)) return false;
    if (!gt_2(state, var__1136, create_int(0))) return false;
    if (!is_2(state, var__1154, create_compound("mod", 2, (term_t*[]){var__1134, var__1136}))) return false;
    if (!gcd_3(state, var__1136, var__1154, var__1138)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool insert_sorted_3(prolog_state_t* state, term_t* arg1, term_t* arg2, term_t* arg3) {
    /* Clause 1: insert_sorted(_1188,[],[_1188]) */
    {
        term_t* var__1188 = create_var(0);
        if (!unify(state, var__1188, arg1)) return false;
        if (!unify(state, create_nil(), arg2)) return false;
        if (!unify(state, create_list(var__1188, create_nil()), arg3)) return false;

        return true;
    }
    /* Clause 2: insert_sorted(_1212,[_1204|_1206],[_1212,_1204|_1206]) :- _1212=<_1204,! */
    {
        term_t* var__1204 = create_var(0);
        term_t* var__1206 = create_var(1);
        term_t* var__1212 = create_var(2);
        if (!unify(state, var__1212, arg1)) return false;
        if (!unify(state, create_list(var__1204, var__1206), arg2)) return false;
        if (!unify(state, create_list(var__1212, create_list(var__1204, var__1206)), arg3)) return false;
    if (!lte_2(state, var__1212, var__1204)) return false;
    perform_cut(state);

        return true;
    }
    /* Clause 3: insert_sorted(_1266,[_1252|_1254],[_1252|_1262]) :- _1266>_1252,insert_sorted(_1266,_1254,_1262) */
    {
        term_t* var__1252 = create_var(0);
        term_t* var__1254 = create_var(1);
        term_t* var__1262 = create_var(2);
        term_t* var__1266 = create_var(3);
        if (!unify(state, var__1266, arg1)) return false;
        if (!unify(state, create_list(var__1252, var__1254), arg2)) return false;
        if (!unify(state, create_list(var__1252, var__1262), arg3)) return false;
    if (!gt_2(state, var__1266, var__1252)) return false;
    if (!insert_sorted_3(state, var__1266, var__1254, var__1262)) return false;

        return true;
    }

    return false; /* No clause matched */
}
bool insertion_sort_2(prolog_state_t* state, term_t* arg1, term_t* arg2) {
    /* Clause 1: insertion_sort([],[]) */
    {
        if (!unify(state, create_nil(), arg1)) return false;
        if (!unify(state, create_nil(), arg2)) return false;

        return true;
    }
    /* Clause 2: insertion_sort([_1308|_1310],_1316) :- insertion_sort(_1310,_1322),insert_sorted(_1308,_1322,_1316) */
    {
        term_t* var__1308 = create_var(0);
        term_t* var__1310 = create_var(1);
        term_t* var__1316 = create_var(2);
        term_t* var__1322 = create_var(3);
        if (!unify(state, create_list(var__1308, var__1310), arg1)) return false;
        if (!unify(state, var__1316, arg2)) return false;
    if (!insertion_sort_2(state, var__1310, var__1322)) return false;
    if (!insert_sorted_3(state, var__1308, var__1322, var__1316)) return false;

        return true;
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
        term_t* var__1356 = create_var(0);
        term_t* var__1410 = create_var(1);
        term_t* var__1464 = create_var(2);
        term_t* var__1490 = create_var(3);
        term_t* var__1512 = create_var(4);
        term_t* var__1556 = create_var(5);
    if (!write_1(state, create_atom("=== PL2C Comprehensive Test ===\n\n"))) return false;
    if (!write_1(state, create_atom("Test 1: Facts and Rules\n"))) return false;
    if (!capital_2(state, create_atom("uk"), var__1356)) return false;
    if (!format_2(state, create_atom("  Capital of UK: ~w\n"), create_list(var__1356, create_nil()))) return false;
    if (!write_1(state, create_atom("\nTest 2: List Operations\n"))) return false;
    if (!append_3(state, create_list(create_int(1), create_list(create_int(2), create_nil())), create_list(create_int(3), create_list(create_int(4), create_nil())), var__1410)) return false;
    if (!format_2(state, create_atom("  append([1,2], [3,4]) = ~w\n"), create_list(var__1410, create_nil()))) return false;
    if (!member_2(state, create_int(2), create_list(create_int(1), create_list(create_int(2), create_list(create_int(3), create_nil()))))) return false;
    if (!write_1(state, create_atom("  member(2, [1,2,3]) succeeded\n"))) return false;
    if (!write_1(state, create_atom("\nTest 3: Arithmetic\n"))) return false;
    if (!factorial_2(state, create_int(5), var__1464)) return false;
    if (!format_2(state, create_atom("  factorial(5) = ~w\n"), create_list(var__1464, create_nil()))) return false;
    if (!write_1(state, create_atom("\nTest 4: Cut Behavior\n"))) return false;
    if (!max_3(state, create_int(10), create_int(5), var__1490)) return false;
    if (!format_2(state, create_atom("  max(10, 5) = ~w\n"), create_list(var__1490, create_nil()))) return false;
    if (!min_3(state, create_int(10), create_int(5), var__1512)) return false;
    if (!format_2(state, create_atom("  min(10, 5) = ~w\n"), create_list(var__1512, create_nil()))) return false;
    if (!write_1(state, create_atom("\nTest 5: Recursive Predicates\n"))) return false;
    if (!reverse_list_2(state, create_list(create_int(1), create_list(create_int(2), create_list(create_int(3), create_nil()))), var__1556)) return false;
    if (!format_2(state, create_atom("  reverse([1,2,3]) = ~w\n"), create_list(var__1556, create_nil()))) return false;
    if (!write_1(state, create_atom("\n=== All Tests Completed ===\n"))) return false;

        return true;
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
    
    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '~' && fmt[i+1] == 'w') {
            if (current_arg->type == TERM_LIST) {
                print_term(current_arg->data.list.head);
                current_arg = current_arg->data.list.tail;
            }
            i++; /* Skip the 'w' */
        } else {
            putchar(fmt[i]);
        }
    }
    
    return true;
}

/* State initialization and cleanup */
void init_state(prolog_state_t* state) {
    state->choice_stack = NULL;
    state->bindings.bindings = NULL;
    state->bindings.size = 0;
    state->bindings.capacity = 0;
    state->cut_level = 0;
    state->failed = false;
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
    
    /* Call compiled predicates here */
    
    free_state(&state);
    return 0;
}
