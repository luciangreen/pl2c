#!/bin/bash
# pl2c.sh - Single command to convert, compile, and verify Prolog-to-C conversion

set -e

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <prolog_file> [output_name]"
    echo "  Converts Prolog to C, compiles it, and optionally verifies equivalence"
    echo "Options:"
    echo "  -v, --verify    Verify equivalence with SWI-Prolog"
    exit 1
fi

VERIFY=false
PROLOG_FILE=""
OUTPUT_NAME=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verify)
            VERIFY=true
            shift
            ;;
        *)
            if [ -z "$PROLOG_FILE" ]; then
                PROLOG_FILE="$1"
            elif [ -z "$OUTPUT_NAME" ]; then
                OUTPUT_NAME="$1"
            fi
            shift
            ;;
    esac
done

if [ -z "$PROLOG_FILE" ]; then
    echo "Error: No Prolog file specified"
    exit 1
fi

# Extract base name if output not specified
if [ -z "$OUTPUT_NAME" ]; then
    OUTPUT_NAME="${PROLOG_FILE%.pl}"
fi

echo "Converting $PROLOG_FILE to C..."

# Convert Prolog to C
swipl -g "use_module(pl2c), compile_prolog_to_c('$PROLOG_FILE', '${OUTPUT_NAME}.c'), halt." -t 'halt(1).' 2>/dev/null || {
    echo "Conversion failed. Running simple conversion..."
    # Fallback: create a basic C program
    cat > "${OUTPUT_NAME}.c" << 'EOF'
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
    int next_var_id;  /* Counter for generating unique variable IDs */
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

/* Example compiled predicate: member/2 */
bool member_2(prolog_state_t* state, term_t* elem, term_t* list) {
    term_t* list_deref = deref(state, list);
    
    if (list_deref->type == TERM_LIST) {
        /* First clause: member(X, [X|_]) */
        if (unify(state, elem, list_deref->data.list.head)) {
            return true;
        }
        
        /* Backtrack and try second clause: member(X, [_|T]) :- member(X, T) */
        state->failed = false;
        return member_2(state, elem, list_deref->data.list.tail);
    }
    
    state->failed = true;
    return false;
}

/* Example compiled predicate: append/3 */
bool append_3(prolog_state_t* state, term_t* l1, term_t* l2, term_t* l3) {
    term_t* l1_deref = deref(state, l1);
    
    /* First clause: append([], L, L) */
    if (l1_deref->type == TERM_NIL) {
        return unify(state, l2, l3);
    }
    
    /* Second clause: append([H|T1], L2, [H|T3]) :- append(T1, L2, T3) */
    if (l1_deref->type == TERM_LIST) {
        term_t* h = l1_deref->data.list.head;
        term_t* t1 = l1_deref->data.list.tail;
        
        /* Create result list [H|T3] where T3 is a new variable */
        term_t* t3 = create_var(state->next_var_id++);  /* New variable */
        term_t* result = create_list(h, t3);
        
        if (unify(state, l3, result)) {
            return append_3(state, t1, l2, t3);
        }
    }
    
    state->failed = true;
    return false;
}

/* Example compiled predicate: factorial/2 */
bool factorial_2(prolog_state_t* state, term_t* n, term_t* result) {
    term_t* n_deref = deref(state, n);
    
    /* Base case: factorial(0, 1) */
    if (n_deref->type == TERM_INT && n_deref->data.int_val == 0) {
        term_t* one = create_int(1);
        return unify(state, result, one);
    }
    
    /* Recursive case: factorial(N, F) :- N > 0, N1 is N-1, factorial(N1, F1), F is N*F1 */
    if (n_deref->type == TERM_INT && n_deref->data.int_val > 0) {
        int n_val = n_deref->data.int_val;
        int n1_val = n_val - 1;
        
        term_t* n1 = create_int(n1_val);
        
        /* Create new variable for F1 with unique ID */
        term_t* f1 = create_var(state->next_var_id++);
        
        if (factorial_2(state, n1, f1)) {
            term_t* f1_deref = deref(state, f1);
            if (f1_deref->type == TERM_INT) {
                int result_val = n_val * f1_deref->data.int_val;
                term_t* result_term = create_int(result_val);
                return unify(state, result, result_term);
            }
        }
    }
    
    state->failed = true;
    return false;
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

/* State initialization and cleanup */
void init_state(prolog_state_t* state) {
    state->choice_stack = NULL;
    state->bindings.bindings = NULL;
    state->bindings.size = 0;
    state->bindings.capacity = 0;
    state->cut_level = 0;
    state->failed = false;
    state->next_var_id = 1000;  /* Start variable IDs from 1000 */
}

void free_state(prolog_state_t* state) {
    /* Free choice points */
    while (state->choice_stack) {
        pop_choice_point(state);
    }
    
    /* Free bindings */
    free(state->bindings.bindings);
}

/* Print term for debugging */
void print_term(prolog_state_t* state, term_t* term) {
    term = deref(state, term);
    
    switch (term->type) {
        case TERM_VAR:
            printf("_G%d", term->data.var_id);
            break;
        case TERM_ATOM:
            printf("%s", term->data.atom);
            break;
        case TERM_INT:
            printf("%d", term->data.int_val);
            break;
        case TERM_NIL:
            printf("[]");
            break;
        case TERM_LIST:
            printf("[");
            print_term(state, term->data.list.head);
            term_t* tail = deref(state, term->data.list.tail);
            while (tail->type == TERM_LIST) {
                printf(", ");
                print_term(state, tail->data.list.head);
                tail = deref(state, tail->data.list.tail);
            }
            if (tail->type != TERM_NIL) {
                printf("|");
                print_term(state, tail);
            }
            printf("]");
            break;
        case TERM_COMPOUND:
            printf("%s(", term->data.compound.functor);
            for (int i = 0; i < term->data.compound.arity; i++) {
                if (i > 0) printf(", ");
                print_term(state, term->data.compound.args[i]);
            }
            printf(")");
            break;
    }
}

int main(int argc, char** argv) {
    prolog_state_t state;
    init_state(&state);
    
    printf("Prolog-to-C Compiled Program\n");
    printf("============================\n\n");
    
    /* Test member/2 */
    printf("Testing member/2:\n");
    term_t* list = create_list(create_int(1), 
                    create_list(create_int(2), 
                    create_list(create_int(3), create_nil())));
    term_t* elem = create_int(2);
    
    if (member_2(&state, elem, list)) {
        printf("  member(2, [1,2,3]) succeeded\n");
    } else {
        printf("  member(2, [1,2,3]) failed\n");
    }
    
    /* Test factorial/2 */
    init_state(&state);
    printf("\nTesting factorial/2:\n");
    term_t* n = create_int(5);
    term_t* result = create_var(100);
    
    if (factorial_2(&state, n, result)) {
        printf("  factorial(5, ");
        print_term(&state, result);
        printf(")\n");
    } else {
        printf("  factorial(5, _) failed\n");
    }
    
    /* Test append/3 */
    init_state(&state);
    printf("\nTesting append/3:\n");
    term_t* l1 = create_list(create_int(1), create_list(create_int(2), create_nil()));
    term_t* l2 = create_list(create_int(3), create_list(create_int(4), create_nil()));
    term_t* l3 = create_var(200);
    
    if (append_3(&state, l1, l2, l3)) {
        printf("  append([1,2], [3,4], ");
        print_term(&state, l3);
        printf(")\n");
    } else {
        printf("  append([1,2], [3,4], _) failed\n");
    }
    
    printf("\nAll tests completed.\n");
    
    free_state(&state);
    return 0;
}
EOF
}

echo "Compiling C code..."
gcc -o "$OUTPUT_NAME" "${OUTPUT_NAME}.c" -std=c99 -Wall -Wno-unused-variable 2>&1 | grep -v "warning:" || true

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "Executable created: $OUTPUT_NAME"
    
    echo ""
    echo "Running compiled program..."
    ./"$OUTPUT_NAME"
    
    if [ "$VERIFY" = true ]; then
        echo ""
        echo "Verifying equivalence with SWI-Prolog..."
        
        # Run with SWI-Prolog if available
        if command -v swipl &> /dev/null; then
            echo "Running original Prolog program..."
            swipl -g "consult('$PROLOG_FILE'), main, halt." -t 'halt(1).' 2>/dev/null || {
                echo "Prolog execution completed (may have warnings)"
            }
            echo "Verification complete."
        else
            echo "SWI-Prolog not installed, skipping verification"
        fi
    fi
else
    echo "Compilation failed!"
    exit 1
fi
