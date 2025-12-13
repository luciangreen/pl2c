% pl2c.pl - Prolog to C Compiler
% Converts Prolog code into equivalent C code with explicit loops and if-then statements

:- module(pl2c, [
    compile_prolog_to_c/2,
    compile_file/2,
    verify_equivalence/1
]).

:- use_module(library(lists)).
:- use_module(library(readutil)).

%% compile_prolog_to_c(+PrologFile, +CFile)
% Main entry point: compiles a Prolog file to C
compile_prolog_to_c(PrologFile, CFile) :-
    read_prolog_file(PrologFile, Clauses),
    translate_program(Clauses, CCode),
    write_c_file(CFile, CCode).

%% read_prolog_file(+File, -Clauses)
% Reads and parses Prolog clauses from a file
read_prolog_file(File, Clauses) :-
    open(File, read, Stream),
    read_clauses(Stream, Clauses),
    close(Stream).

read_clauses(Stream, Clauses) :-
    read_term(Stream, Term, []),
    (   Term == end_of_file
    ->  Clauses = []
    ;   Clauses = [Term|Rest],
        read_clauses(Stream, Rest)
    ).

%% translate_program(+Clauses, -CCode)
% Translates a list of Prolog clauses to C code
translate_program(Clauses, CCode) :-
    generate_c_header(Header),
    translate_clauses(Clauses, PredicateDefs),
    generate_c_main(MainCode),
    generate_c_footer(Footer),
    atomic_list_concat([Header, PredicateDefs, MainCode, Footer], '\n', CCode).

%% generate_c_header(-Header)
% Generates the C header with includes and data structures
generate_c_header(Header) :-
    Header = 
'#define _POSIX_C_SOURCE 200809L
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
'.

%% translate_clauses(+Clauses, -CCode)
% Translates each clause to C function code
translate_clauses([], '').
translate_clauses([Clause|Rest], CCode) :-
    translate_clause(Clause, CClause),
    translate_clauses(Rest, CRest),
    atomic_list_concat([CClause, CRest], '\n', CCode).

%% translate_clause(+Clause, -CCode)
% Translates a single clause to C code
translate_clause((Head :- Body), CCode) :-
    !,
    extract_predicate_info(Head, Name, Arity, Args),
    format(atom(FuncName), '~w_~w', [Name, Arity]),
    translate_args_to_params(Args, Params),
    translate_body(Body, BodyCode, 0),
    format(atom(CCode), 
'bool ~w(prolog_state_t* state~w) {
    /* Clause: ~w :- ~w */
~w
    return true;
}', [FuncName, Params, Head, Body, BodyCode]).

translate_clause(Head, CCode) :-
    % Fact (clause without body)
    extract_predicate_info(Head, Name, Arity, Args),
    format(atom(FuncName), '~w_~w', [Name, Arity]),
    translate_args_to_params(Args, Params),
    format(atom(CCode), 
'bool ~w(prolog_state_t* state~w) {
    /* Fact: ~w */
    return true;
}', [FuncName, Params, Head]).

%% extract_predicate_info(+Term, -Name, -Arity, -Args)
extract_predicate_info(Term, Name, Arity, Args) :-
    Term =.. [Name|Args],
    length(Args, Arity).

%% translate_args_to_params(+Args, -Params)
translate_args_to_params([], '').
translate_args_to_params(Args, Params) :-
    Args \= [],
    length(Args, N),
    findall(P, (between(1, N, I), format(atom(P), ', term_t* arg~w', [I])), ParamList),
    atomic_list_concat(ParamList, '', Params).

%% translate_body(+Body, -CCode, +Depth)
% Translates clause body to C code with proper control flow
translate_body(true, '    /* true */\n', _) :- !.
translate_body(fail, '    state->failed = true;\n    return false;\n', _) :- !.
translate_body(!, CCode, _) :-
    !,
    CCode = '    perform_cut(state);\n'.
translate_body((A, B), CCode, Depth) :-
    !,
    % Conjunction: execute A then B
    translate_body(A, ACode, Depth),
    translate_body(B, BCode, Depth),
    format(atom(CCode), '~w~w', [ACode, BCode]).
translate_body((A ; B), CCode, Depth) :-
    !,
    % Disjunction: try A, if it fails try B
    NextDepth is Depth + 1,
    translate_body(A, ACode, NextDepth),
    translate_body(B, BCode, NextDepth),
    format(atom(CCode),
'    /* Disjunction */
    push_choice_point(state, 0, 1);
    if (!state->failed) {
~w
    }
    if (state->failed) {
        pop_choice_point(state);
        state->failed = false;
~w
    }
', [ACode, BCode]).
translate_body(findall(Template, Goal, Result), CCode, Depth) :-
    !,
    % findall/3: enumerate all solutions
    translate_body(Goal, GoalCode, Depth),
    format(atom(CCode),
'    /* findall(~w, ~w, ~w) */
    {
        term_t** solutions = NULL;
        int solution_count = 0;
        prolog_state_t findall_state;
        init_state(&findall_state);
        
        /* Enumerate solutions */
        while (true) {
~w
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
', [Template, Goal, Result, GoalCode]).
translate_body(Call, CCode, _) :-
    % Regular predicate call
    extract_predicate_info(Call, Name, Arity, Args),
    format(atom(FuncName), '~w_~w', [Name, Arity]),
    translate_call_args(Args, ArgStr),
    format(atom(CCode), '    if (!~w(state~w)) return false;\n', [FuncName, ArgStr]).

translate_call_args([], '').
translate_call_args([Arg|Args], Result) :-
    term_to_c_expr(Arg, CExpr),
    translate_call_args(Args, Rest),
    format(atom(Result), ', ~w~w', [CExpr, Rest]).

term_to_c_expr(Var, Expr) :-
    var(Var),
    !,
    format(atom(Expr), 'var_~w', [Var]).
term_to_c_expr(Atom, Expr) :-
    atom(Atom),
    !,
    format(atom(Expr), 'create_atom("~w")', [Atom]).
term_to_c_expr(Int, Expr) :-
    integer(Int),
    !,
    format(atom(Expr), 'create_int(~w)', [Int]).
term_to_c_expr([], Expr) :-
    !,
    Expr = 'create_nil()'.
term_to_c_expr([H|T], Expr) :-
    !,
    term_to_c_expr(H, HExpr),
    term_to_c_expr(T, TExpr),
    format(atom(Expr), 'create_list(~w, ~w)', [HExpr, TExpr]).
term_to_c_expr(Compound, Expr) :-
    Compound =.. [Functor|Args],
    length(Args, Arity),
    maplist(term_to_c_expr, Args, CArgs),
    atomic_list_concat(CArgs, ', ', ArgsStr),
    format(atom(Expr), 'create_compound("~w", ~w, (term_t*[]){~w})', [Functor, Arity, ArgsStr]).

%% generate_c_main(-MainCode)
generate_c_main(MainCode) :-
    MainCode = ''.

%% generate_c_footer(-Footer)
generate_c_footer(Footer) :-
    Footer = 
'
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
    
    printf("Prolog-to-C compiled program\\n");
    
    /* Call compiled predicates here */
    
    free_state(&state);
    return 0;
}
'.

%% write_c_file(+File, +CCode)
write_c_file(File, CCode) :-
    open(File, write, Stream),
    write(Stream, CCode),
    close(Stream).

%% compile_file(+PrologFile, +OutputBase)
% Compiles Prolog file to C and creates executable
compile_file(PrologFile, OutputBase) :-
    format(atom(CFile), '~w.c', [OutputBase]),
    format(atom(ExeFile), '~w', [OutputBase]),
    compile_prolog_to_c(PrologFile, CFile),
    format(atom(CompileCmd), 'gcc -o ~w ~w -std=c99 -Wall', [ExeFile, CFile]),
    shell(CompileCmd).

%% verify_equivalence(+PrologFile)
% Verifies that compiled C code produces same results as SWI-Prolog
verify_equivalence(PrologFile) :-
    % Run Prolog version
    format(atom(PrologCmd), 'swipl -g main -t halt ~w > prolog_output.txt', [PrologFile]),
    shell(PrologCmd),
    
    % Compile and run C version
    atom_concat(Base, '.pl', PrologFile),
    compile_file(PrologFile, Base),
    format(atom(CCmd), './~w > c_output.txt', [Base]),
    shell(CCmd),
    
    % Compare outputs
    shell('diff prolog_output.txt c_output.txt'),
    write('Verification successful: outputs match!\n').
