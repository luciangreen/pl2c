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
int eval_arithmetic(prolog_state_t* state, term_t* expr);

/* Built-in operators */
bool gt_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool lt_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool gte_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool lte_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool eq_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
bool is_2(prolog_state_t* state, term_t* arg1, term_t* arg2);

/* Built-in I/O predicates */
void print_term(term_t* term);
bool write_1(prolog_state_t* state, term_t* arg1);
bool format_2(prolog_state_t* state, term_t* arg1, term_t* arg2);
'.

%% translate_clauses(+Clauses, -CCode)
% Translates each clause to C function code
translate_clauses([], '').
translate_clauses(Clauses, CCode) :-
    Clauses \= [],
    group_clauses_by_predicate(Clauses, GroupedClauses),
    translate_predicate_groups(GroupedClauses, CCode).

%% group_clauses_by_predicate(+Clauses, -GroupedClauses)
% Groups clauses by their predicate name/arity
group_clauses_by_predicate([], []).
group_clauses_by_predicate(Clauses, Grouped) :-
    Clauses \= [],
    collect_predicate_signatures(Clauses, Signatures),
    group_by_signature(Signatures, Clauses, Grouped).

collect_predicate_signatures([], []).
collect_predicate_signatures([Clause|Rest], [Sig|Sigs]) :-
    clause_signature(Clause, Sig),
    collect_predicate_signatures(Rest, Sigs).

clause_signature((Head :- _), Name/Arity) :-
    !,
    extract_predicate_info(Head, Name, Arity, _).
clause_signature(Head, Name/Arity) :-
    extract_predicate_info(Head, Name, Arity, _).

group_by_signature([], [], []).
group_by_signature(Sigs, Clauses, [Group|RestGroups]) :-
    Sigs \= [],
    Sigs = [FirstSig|_],
    collect_matching_clauses(FirstSig, Sigs, Clauses, MatchingClauses, RemainingClauses, RemainingSigs),
    Group = FirstSig-MatchingClauses,
    group_by_signature(RemainingSigs, RemainingClauses, RestGroups).

collect_matching_clauses(_, [], [], [], [], []).
collect_matching_clauses(TargetSig, [Sig|Sigs], [Clause|Clauses], [Clause|Matching], Remaining, RemainingSigs) :-
    Sig = TargetSig,
    !,
    collect_matching_clauses(TargetSig, Sigs, Clauses, Matching, Remaining, RemainingSigs).
collect_matching_clauses(TargetSig, [Sig|Sigs], [Clause|Clauses], Matching, [Clause|Remaining], [Sig|RemainingSigs]) :-
    collect_matching_clauses(TargetSig, Sigs, Clauses, Matching, Remaining, RemainingSigs).

%% translate_predicate_groups(+Groups, -CCode)
translate_predicate_groups([], '').
translate_predicate_groups([Group|Rest], CCode) :-
    translate_predicate_group(Group, GroupCode),
    translate_predicate_groups(Rest, RestCode),
    atomic_list_concat([GroupCode, RestCode], '\n', CCode).

%% translate_predicate_group(+Group, -CCode)
% Translates all clauses for a single predicate into one C function
translate_predicate_group(Name/Arity-Clauses, CCode) :-
    % Get parameters from first clause
    Clauses = [FirstClause|_],
    clause_head(FirstClause, Head),
    extract_predicate_info(Head, _, _, Args),
    translate_args_to_params(Args, Params),
    sanitize_predicate_name(Name, SanitizedName),
    format(atom(FuncName), '~w_~w', [SanitizedName, Arity]),
    translate_predicate_clauses(Clauses, 1, ClausesCode),
    format(atom(CCode), 
'bool ~w(prolog_state_t* state~w) {
~w
    return false; /* No clause matched */
}', [FuncName, Params, ClausesCode]).

clause_head((Head :- _), Head) :- !.
clause_head(Head, Head).

%% sanitize_predicate_name(+Name, -SanitizedName)
% Converts Prolog operators to valid C identifiers
sanitize_predicate_name('>', 'gt') :- !.
sanitize_predicate_name('<', 'lt') :- !.
sanitize_predicate_name('>=', 'gte') :- !.
sanitize_predicate_name('=<', 'lte') :- !.
sanitize_predicate_name('=', 'eq') :- !.
sanitize_predicate_name('==', 'eqeq') :- !.
sanitize_predicate_name('\\=', 'neq') :- !.
sanitize_predicate_name('\\==', 'neqeq') :- !.
sanitize_predicate_name('is', 'is') :- !.
sanitize_predicate_name('+', 'plus') :- !.
sanitize_predicate_name('-', 'minus') :- !.
sanitize_predicate_name('*', 'times') :- !.
sanitize_predicate_name('/', 'div') :- !.
sanitize_predicate_name('//', 'intdiv') :- !.
sanitize_predicate_name('mod', 'mod') :- !.
sanitize_predicate_name(Name, Name).

%% translate_predicate_clauses(+Clauses, +Index, -CCode)
translate_predicate_clauses([], _, '').
translate_predicate_clauses([Clause|Rest], Index, CCode) :-
    translate_single_clause(Clause, Index, ClauseCode),
    NextIndex is Index + 1,
    translate_predicate_clauses(Rest, NextIndex, RestCode),
    atomic_list_concat([ClauseCode, RestCode], '', CCode).

%% translate_single_clause(+Clause, +Index, -CCode)
translate_single_clause((Head :- Body), Index, CCode) :-
    !,
    collect_variables(Head, HeadVars),
    collect_variables(Body, BodyVars),
    append(HeadVars, BodyVars, AllVars),
    sort(AllVars, UniqueVars),
    generate_var_declarations(UniqueVars, VarDecls),
    translate_head_unifications(Head, Index, HeadCode),
    translate_body(Body, BodyCode, 0),
    format(atom(CCode), 
'    /* Clause ~w: ~w :- ~w */
    {
~w~w~w
        return true;
    }
', [Index, Head, Body, VarDecls, HeadCode, BodyCode]).

translate_single_clause(Head, Index, CCode) :-
    % Fact (clause without body)
    collect_variables(Head, HeadVars),
    sort(HeadVars, UniqueVars),
    generate_var_declarations(UniqueVars, VarDecls),
    translate_head_unifications(Head, Index, HeadCode),
    format(atom(CCode), 
'    /* Clause ~w: ~w */
    {
~w~w
        return true;
    }
', [Index, Head, VarDecls, HeadCode]).

%% collect_variables(+Term, -Vars)
% Collects all variables in a term
collect_variables(Var, [Var]) :- var(Var), !.
collect_variables(Term, Vars) :-
    compound(Term),
    !,
    Term =.. [_|Args],
    collect_variables_list(Args, Vars).
collect_variables(_, []).

collect_variables_list([], []).
collect_variables_list([H|T], Vars) :-
    collect_variables(H, HVars),
    collect_variables_list(T, TVars),
    append(HVars, TVars, Vars).

%% generate_var_declarations(+Vars, -Decls)
generate_var_declarations([], '').
generate_var_declarations(Vars, Decls) :-
    Vars \= [],
    generate_var_decls_with_ids(Vars, 0, DeclList),
    atomic_list_concat(DeclList, '', Decls).

generate_var_decls_with_ids([], _, []).
generate_var_decls_with_ids([V|Vs], N, [Decl|Decls]) :-
    format(atom(Decl), '        term_t* var_~w = create_var(~w);\n', [V, N]),
    N1 is N + 1,
    generate_var_decls_with_ids(Vs, N1, Decls).

%% translate_head_unifications(+Head, +Index, -CCode)
% Generates code to unify head arguments with actual parameters
translate_head_unifications(Head, _, CCode) :-
    extract_predicate_info(Head, _, _, Args),
    translate_head_args(Args, 1, CCode).

translate_head_args([], _, '').
translate_head_args([Arg|Args], N, CCode) :-
    translate_head_arg_unify(Arg, N, ArgCode),
    N1 is N + 1,
    translate_head_args(Args, N1, RestCode),
    atomic_list_concat([ArgCode, RestCode], '', CCode).

translate_head_arg_unify(Var, N, CCode) :-
    var(Var),
    !,
    format(atom(CCode), '        if (!unify(state, var_~w, arg~w)) return false;\n', [Var, N]).
translate_head_arg_unify(Arg, N, CCode) :-
    term_to_c_expr(Arg, CExpr),
    format(atom(CCode), '        if (!unify(state, ~w, arg~w)) return false;\n', [CExpr, N]).

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
    sanitize_predicate_name(Name, SanitizedName),
    format(atom(FuncName), '~w_~w', [SanitizedName, Arity]),
    translate_call_args(Args, ArgStr),
    format(atom(CCode), '    if (!~w(state~w)) return false;\n', [FuncName, ArgStr]).

translate_call_args([], '').
translate_call_args([Arg|Args], Result) :-
    term_to_c_expr(Arg, CExpr),
    translate_call_args(Args, Rest),
    format(atom(Result), ', ~w~w', [CExpr, Rest]).

%% escape_c_string(+InputCodes, -OutputCodes)
% Escapes special characters for C string literals
escape_c_string([], []).
escape_c_string([10|Rest], [92, 110|EscapedRest]) :- % \n
    !,
    escape_c_string(Rest, EscapedRest).
escape_c_string([13|Rest], [92, 114|EscapedRest]) :- % \r
    !,
    escape_c_string(Rest, EscapedRest).
escape_c_string([9|Rest], [92, 116|EscapedRest]) :- % \t
    !,
    escape_c_string(Rest, EscapedRest).
escape_c_string([34|Rest], [92, 34|EscapedRest]) :- % \"
    !,
    escape_c_string(Rest, EscapedRest).
escape_c_string([92|Rest], [92, 92|EscapedRest]) :- % \\
    !,
    escape_c_string(Rest, EscapedRest).
escape_c_string([C|Rest], [C|EscapedRest]) :-
    escape_c_string(Rest, EscapedRest).

term_to_c_expr(Var, Expr) :-
    var(Var),
    !,
    format(atom(Expr), 'var_~w', [Var]).
term_to_c_expr(Atom, Expr) :-
    atom(Atom),
    !,
    atom_codes(Atom, Codes),
    escape_c_string(Codes, EscapedCodes),
    atom_codes(EscapedAtom, EscapedCodes),
    format(atom(Expr), 'create_atom("~w")', [EscapedAtom]).
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
    
    for (int i = 0; fmt[i] != \'\\0\'; i++) {
        if (fmt[i] == \'~\' && fmt[i+1] == \'w\') {
            if (current_arg->type == TERM_LIST) {
                print_term(current_arg->data.list.head);
                current_arg = current_arg->data.list.tail;
            }
            i++; /* Skip the \'w\' */
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
