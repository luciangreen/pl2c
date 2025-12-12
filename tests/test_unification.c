/* test_unification.c - Tests for the unification algorithm */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

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

typedef struct {
    int var_id;
    term_t* value;
} binding_t;

typedef struct {
    binding_t* bindings;
    int size;
    int capacity;
} bindings_t;

typedef struct choice_point {
    int predicate_id;
    int clause_index;
    bindings_t saved_bindings;
    struct choice_point* prev;
} choice_point_t;

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
void init_state(prolog_state_t* state);
void free_state(prolog_state_t* state);

/* Term creation functions */
term_t* create_var(int id) {
    term_t* t = malloc(sizeof(term_t));
    if (!t) return NULL;
    t->type = TERM_VAR;
    t->data.var_id = id;
    return t;
}

term_t* create_atom(const char* name) {
    term_t* t = malloc(sizeof(term_t));
    if (!t) return NULL;
    t->type = TERM_ATOM;
    t->data.atom = strdup(name);
    if (!t->data.atom) {
        free(t);
        return NULL;
    }
    return t;
}

term_t* create_int(int val) {
    term_t* t = malloc(sizeof(term_t));
    if (!t) return NULL;
    t->type = TERM_INT;
    t->data.int_val = val;
    return t;
}

term_t* create_compound(const char* functor, int arity, term_t** args) {
    term_t* t = malloc(sizeof(term_t));
    if (!t) return NULL;
    t->type = TERM_COMPOUND;
    t->data.compound.functor = strdup(functor);
    if (!t->data.compound.functor) {
        free(t);
        return NULL;
    }
    t->data.compound.arity = arity;
    t->data.compound.args = malloc(sizeof(term_t*) * arity);
    if (!t->data.compound.args) {
        free(t->data.compound.functor);
        free(t);
        return NULL;
    }
    for (int i = 0; i < arity; i++) {
        t->data.compound.args[i] = args[i];
    }
    return t;
}

term_t* create_list(term_t* head, term_t* tail) {
    term_t* t = malloc(sizeof(term_t));
    if (!t) return NULL;
    t->type = TERM_LIST;
    t->data.list.head = head;
    t->data.list.tail = tail;
    return t;
}

term_t* create_nil() {
    term_t* t = malloc(sizeof(term_t));
    if (!t) return NULL;
    t->type = TERM_NIL;
    return t;
}

term_t* deref(prolog_state_t* state, term_t* term) {
    if (term->type != TERM_VAR) return term;
    
    for (int i = 0; i < state->bindings.size; i++) {
        if (state->bindings.bindings[i].var_id == term->data.var_id) {
            return deref(state, state->bindings.bindings[i].value);
        }
    }
    return term;
}

bool unify(prolog_state_t* state, term_t* t1, term_t* t2) {
    t1 = deref(state, t1);
    t2 = deref(state, t2);
    
    if (t1->type == TERM_VAR) {
        if (state->bindings.size >= state->bindings.capacity) {
            int new_capacity = state->bindings.capacity * 2 + 1;
            binding_t* new_bindings = realloc(state->bindings.bindings, 
                sizeof(binding_t) * new_capacity);
            if (!new_bindings) return false;
            state->bindings.bindings = new_bindings;
            state->bindings.capacity = new_capacity;
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

void init_state(prolog_state_t* state) {
    state->choice_stack = NULL;
    state->bindings.bindings = NULL;
    state->bindings.size = 0;
    state->bindings.capacity = 0;
    state->cut_level = 0;
    state->failed = false;
}

void free_state(prolog_state_t* state) {
    free(state->bindings.bindings);
}

/* Test functions */
void test_unify_atoms() {
    printf("Test: Unify atoms\n");
    prolog_state_t state;
    init_state(&state);
    
    term_t* a1 = create_atom("hello");
    term_t* a2 = create_atom("hello");
    term_t* a3 = create_atom("world");
    
    assert(unify(&state, a1, a2) == true);
    assert(unify(&state, a1, a3) == false);
    
    free_state(&state);
    printf("  PASSED\n");
}

void test_unify_integers() {
    printf("Test: Unify integers\n");
    prolog_state_t state;
    init_state(&state);
    
    term_t* i1 = create_int(42);
    term_t* i2 = create_int(42);
    term_t* i3 = create_int(99);
    
    assert(unify(&state, i1, i2) == true);
    assert(unify(&state, i1, i3) == false);
    
    free_state(&state);
    printf("  PASSED\n");
}

void test_unify_variables() {
    printf("Test: Unify variables\n");
    prolog_state_t state;
    init_state(&state);
    
    term_t* v1 = create_var(1);
    term_t* v2 = create_var(2);
    term_t* a = create_atom("test");
    
    assert(unify(&state, v1, a) == true);
    term_t* result = deref(&state, v1);
    assert(result->type == TERM_ATOM);
    assert(strcmp(result->data.atom, "test") == 0);
    
    free_state(&state);
    printf("  PASSED\n");
}

void test_unify_lists() {
    printf("Test: Unify lists\n");
    prolog_state_t state;
    init_state(&state);
    
    term_t* l1 = create_list(create_int(1), 
                  create_list(create_int(2), create_nil()));
    term_t* l2 = create_list(create_int(1), 
                  create_list(create_int(2), create_nil()));
    
    assert(unify(&state, l1, l2) == true);
    
    free_state(&state);
    printf("  PASSED\n");
}

void test_unify_compounds() {
    printf("Test: Unify compound terms\n");
    prolog_state_t state;
    init_state(&state);
    
    term_t* args1[] = {create_atom("a"), create_int(1)};
    term_t* args2[] = {create_atom("a"), create_int(1)};
    term_t* args3[] = {create_atom("b"), create_int(1)};
    
    term_t* c1 = create_compound("foo", 2, args1);
    term_t* c2 = create_compound("foo", 2, args2);
    term_t* c3 = create_compound("foo", 2, args3);
    
    assert(unify(&state, c1, c2) == true);
    assert(unify(&state, c1, c3) == false);
    
    free_state(&state);
    printf("  PASSED\n");
}

int main() {
    printf("Running unification tests...\n\n");
    
    test_unify_atoms();
    test_unify_integers();
    test_unify_variables();
    test_unify_lists();
    test_unify_compounds();
    
    printf("\nAll tests passed!\n");
    return 0;
}
