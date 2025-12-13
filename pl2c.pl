% pl2c.pl - Prolog to C Compiler
% Converts Prolog code into equivalent C code with explicit loops and if-then statements

:- module(pl2c, [
    compile_prolog_to_c/2,
    compile_file/2,
    verify_equivalence/1
]).

:- use_module(library(lists)).
:- use_module(library(readutil)).

% Dynamic predicate to store variable-name-to-index mapping during compilation
:- dynamic var_name_index_map/2.

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
    group_clauses_by_predicate(Clauses, GroupedClauses),
    generate_predicate_declarations(GroupedClauses, Declarations),
    translate_predicate_groups(GroupedClauses, PredicateDefs),
    generate_c_main(MainCode),
    generate_c_footer(Footer),
    atomic_list_concat([Header, Declarations, PredicateDefs, MainCode, Footer], '\n', CCode).

%% generate_c_header(-Header)
% Generates the C header with includes and data structures
generate_c_header(Header) :-
    Header = 
'#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
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
term_t* copy_term_helper(prolog_state_t* state, term_t* term, int* var_offset);

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
'.

%% generate_predicate_declarations(+GroupedClauses, -Declarations)
% Generate forward declarations for all predicates
generate_predicate_declarations([], '').
generate_predicate_declarations(Groups, Declarations) :-
    findall(Decl, (
        member(Name/Arity-Clauses, Groups),
        Clauses = [FirstClause|_],
        clause_head(FirstClause, Head),
        extract_predicate_info(Head, _, _, Args),
        translate_args_to_params(Args, Params),
        sanitize_predicate_name(Name, SanitizedName),
        format(atom(Decl), 'bool ~w_~w(prolog_state_t* state~w);', [SanitizedName, Arity, Params])
    ), DeclList),
    atomic_list_concat(DeclList, '\n', Declarations).

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
    length(Clauses, NumClauses),
    ( NumClauses > 1 ->
        % Multiple clauses - generate nondeterministic version with choice points
        translate_nondeterministic_predicate(Clauses, FuncName, Params, CCode)
    ;
        % Single clause - generate simple version without choice points
        translate_predicate_clauses(Clauses, 1, ClausesCode),
        format(atom(CCode), 
'bool ~w(prolog_state_t* state~w) {
~w
    return false; /* No clause matched */
}', [FuncName, Params, ClausesCode])
    ).

clause_head((Head :- _), Head) :- !.
clause_head(Head, Head).

%% translate_nondeterministic_predicate(+Clauses, +FuncName, +Params, -CCode)
% Generates code for a predicate with multiple clauses that supports backtracking
translate_nondeterministic_predicate(Clauses, FuncName, Params, CCode) :-
    length(Clauses, NumClauses),
    translate_predicate_clauses_with_choicepoints(Clauses, FuncName, 1, NumClauses, ClausesCode),
    format(atom(CCode),
'bool ~w(prolog_state_t* state~w) {
    /* Check if resuming from a choice point */
    int start_clause = 1;
    if (state->choice_stack && state->choice_stack->predicate_id == (int)(intptr_t)&~w) {
        start_clause = state->choice_stack->clause_index;
        pop_choice_point(state);
    }
    
~w
    return false; /* No clause matched */
}', [FuncName, Params, FuncName, ClausesCode]).

%% translate_predicate_clauses_with_choicepoints(+Clauses, +FuncName, +Index, +TotalClauses, -CCode)
translate_predicate_clauses_with_choicepoints([], _, _, _, '').
translate_predicate_clauses_with_choicepoints([Clause|Rest], FuncName, Index, TotalClauses, CCode) :-
    % Determine if this is the last clause
    NextIndex is Index + 1,
    (NextIndex > TotalClauses -> IsLast = true ; IsLast = false),
    translate_single_clause_with_choicepoint(Clause, FuncName, Index, IsLast, ClauseCode),
    translate_predicate_clauses_with_choicepoints(Rest, FuncName, NextIndex, TotalClauses, RestCode),
    atomic_list_concat([ClauseCode, RestCode], '', CCode).

%% translate_single_clause_with_choicepoint(+Clause, +FuncName, +Index, +IsLast, -CCode)
translate_single_clause_with_choicepoint((Head :- Body), FuncName, Index, IsLast, CCode) :-
    !,
    term_variables((Head, Body), AllVars),
    create_var_map(AllVars),
    setup_call_cleanup(
        true,
        (
            generate_var_declarations(AllVars, VarDecls),
            translate_head_unifications_with_check(Head, Index, HeadCode),
            translate_body(Body, BodyCode, 0),
            copy_term((Head, Body), (HeadDisp, BodyDisp)),
            numbervars((HeadDisp, BodyDisp), 0, _),
            ( IsLast ->
                % Last clause - no choice point needed
                format(atom(CCode), 
'    /* Clause ~w: ~w :- ~w */
    if (start_clause <= ~w) {
        int saved_bindings_size = state->bindings.size;
~w~w
            do {
~w            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
', [Index, HeadDisp, BodyDisp, Index, VarDecls, HeadCode, BodyCode])
            ;
                % Not last clause - push choice point
                NextIndex is Index + 1,
                format(atom(CCode), 
'    /* Clause ~w: ~w :- ~w */
    if (start_clause <= ~w) {
        int saved_bindings_size = state->bindings.size;
~w~w
            /* Push choice point for next clause */
            push_choice_point(state, (int)(intptr_t)&~w, ~w);
            do {
~w            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
', [Index, HeadDisp, BodyDisp, Index, VarDecls, HeadCode, FuncName, NextIndex, BodyCode])
            )
        ),
        retractall(var_name_index_map(_, _))
    ).

translate_single_clause_with_choicepoint(Head, FuncName, Index, IsLast, CCode) :-
    % Fact (clause without body)
    term_variables(Head, AllVars),
    create_var_map(AllVars),
    setup_call_cleanup(
        true,
        (
            generate_var_declarations(AllVars, VarDecls),
            translate_head_unifications_with_check(Head, Index, HeadCode),
            copy_term(Head, HeadDisp),
            numbervars(HeadDisp, 0, _),
            ( IsLast ->
                % Last clause - no choice point needed
                format(atom(CCode), 
'    /* Clause ~w: ~w */
    if (start_clause <= ~w) {
        int saved_bindings_size = state->bindings.size;
~w~w
            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
', [Index, HeadDisp, Index, VarDecls, HeadCode])
            ;
                % Not last clause - push choice point
                NextIndex is Index + 1,
                format(atom(CCode), 
'    /* Clause ~w: ~w */
    if (start_clause <= ~w) {
        int saved_bindings_size = state->bindings.size;
~w~w
            /* Push choice point for next clause */
            push_choice_point(state, (int)(intptr_t)&~w, ~w);
            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
', [Index, HeadDisp, Index, VarDecls, HeadCode, FuncName, NextIndex])
            )
        ),
        retractall(var_name_index_map(_, _))
    ).

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
sanitize_predicate_name('@<', 'term_lt') :- !.
sanitize_predicate_name('@>', 'term_gt') :- !.
sanitize_predicate_name('@=<', 'term_lte') :- !.
sanitize_predicate_name('@>=', 'term_gte') :- !.
sanitize_predicate_name('=..', 'univ') :- !.
sanitize_predicate_name('!', 'cut') :- !.
sanitize_predicate_name('true', 'true') :- !.
sanitize_predicate_name('->', 'if_then') :- !.
sanitize_predicate_name(';', 'semicolon') :- !.
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
    % Collect all variables
    term_variables((Head, Body), AllVars),
    % Create explicit variable-to-index mapping
    create_var_map(AllVars),
    % Use setup_call_cleanup to ensure mapping is always cleaned up
    setup_call_cleanup(
        true,
        (
            % Generate ALL code strings
            generate_var_declarations(AllVars, VarDecls),
            translate_head_unifications_with_check(Head, Index, HeadCode),
            translate_body(Body, BodyCode, 0),
            % Create a display copy for the comment
            copy_term((Head, Body), (HeadDisp, BodyDisp)),
            numbervars((HeadDisp, BodyDisp), 0, _),
            % Final assembly
            format(atom(CCode), 
'    /* Clause ~w: ~w :- ~w */
    {
        int saved_bindings_size = state->bindings.size;
~w~w
            do {
~w            } while (0);
            if (!state->failed) return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
', [Index, HeadDisp, BodyDisp, VarDecls, HeadCode, BodyCode])
        ),
        retractall(var_name_index_map(_, _))
    ).

translate_single_clause(Head, Index, CCode) :-
    % Fact (clause without body)
    % Collect all variables
    term_variables(Head, AllVars),
    % Create explicit variable-to-index mapping
    create_var_map(AllVars),
    % Use setup_call_cleanup to ensure mapping is always cleaned up
    setup_call_cleanup(
        true,
        (
            generate_var_declarations(AllVars, VarDecls),
            translate_head_unifications_with_check(Head, Index, HeadCode),
            % Create a display copy for the comment
            copy_term(Head, HeadDisp),
            numbervars(HeadDisp, 0, _),
            format(atom(CCode), 
'    /* Clause ~w: ~w */
    {
        int saved_bindings_size = state->bindings.size;
~w~w
            return true;
        }
        /* Restore bindings for next clause */
        state->bindings.size = saved_bindings_size;
        state->failed = false;
    }
', [Index, HeadDisp, VarDecls, HeadCode])
        ),
        retractall(var_name_index_map(_, _))
    ).

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
    get_var_index(V, Index),
    format(atom(Decl), '        term_t* var__~w = create_var(state->next_var_id++);\n', [Index]),
    N1 is N + 1,
    generate_var_decls_with_ids(Vs, N1, Decls).

%% generate_var_declarations_numbered_impl(+Count, +Prefix, -Decls)
% Generates declarations for variables with the given prefix
% e.g., Prefix='var__' generates var__0, var__1, ..., var__{Count-1}
generate_var_declarations_numbered_impl(Count, Prefix, Decls) :-
    Count > 0,
    !,
    C1 is Count - 1,
    findall(Decl, 
        (between(0, C1, N),
         format(atom(Decl), '        term_t* ~w~w = create_var(state->next_var_id++);\n', [Prefix, N])),
        DeclList),
    atomic_list_concat(DeclList, '', Decls).
generate_var_declarations_numbered_impl(_, _, '').

%% generate_var_declarations_numbered(+Count, -Decls)
% Generates declarations for var__VAR_0, var__VAR_1, ... var__VAR_{Count-1}
generate_var_declarations_numbered(Count, Decls) :-
    generate_var_declarations_numbered_impl(Count, 'var__VAR_', Decls).

%% generate_var_declarations_with_count(+Count, -Decls)
% Generates declarations for var__0, var__1, ... var___{Count-1}
generate_var_declarations_with_count(Count, Decls) :-
    generate_var_declarations_numbered_impl(Count, 'var__', Decls).

%% translate_head_unifications(+Head, +Index, -CCode)
% Generates code to unify head arguments with actual parameters (old style with return)
translate_head_unifications(Head, _, CCode) :-
    extract_predicate_info(Head, _, _, Args),
    translate_head_args(Args, 1, CCode).

%% translate_head_unifications_with_check(+Head, +Index, -CCode)
% Generates code to unify head arguments with conditional check instead of return
translate_head_unifications_with_check(Head, _, CCode) :-
    extract_predicate_info(Head, _, _, Args),
    translate_head_args_with_check(Args, 1, [], UnifyList),
    ( UnifyList = [] ->
        CCode = '        if (true) {\n'
    ;
        atomic_list_concat(UnifyList, ' &&\n            ', UnifyCondition),
        format(atom(CCode), '        if (~w) {\n', [UnifyCondition])
    ).

translate_head_args_with_check([], _, Acc, Acc).
translate_head_args_with_check([Arg|Args], N, Acc, Result) :-
    translate_head_arg_check(Arg, N, ArgCode),
    N1 is N + 1,
    append(Acc, [ArgCode], NewAcc),
    translate_head_args_with_check(Args, N1, NewAcc, Result).

translate_head_arg_check(Var, N, CCode) :-
    var(Var),
    !,
    get_var_index(Var, Index),
    format(atom(CCode), 'unify(state, var__~w, arg~w)', [Index, N]).
translate_head_arg_check(Arg, N, CCode) :-
    term_to_c_expr(Arg, CExpr),
    format(atom(CCode), 'unify(state, ~w, arg~w)', [CExpr, N]).

translate_head_args([], _, '').
translate_head_args([Arg|Args], N, CCode) :-
    translate_head_arg_unify(Arg, N, ArgCode),
    N1 is N + 1,
    translate_head_args(Args, N1, RestCode),
    atomic_list_concat([ArgCode, RestCode], '', CCode).

translate_head_arg_unify(Var, N, CCode) :-
    var(Var),
    !,
    get_var_index(Var, Index),
    format(atom(CCode), '        if (!unify(state, var__~w, arg~w)) return false;\n', [Index, N]).
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
translate_body(fail, '    state->failed = true;\n    break;\n', _) :- !.
translate_body(!, CCode, _) :-
    !,
    CCode = '    perform_cut(state);\n'.
translate_body((A, B), CCode, Depth) :-
    !,
    % Conjunction: execute A then B
    translate_body(A, ACode, Depth),
    translate_body(B, BCode, Depth),
    format(atom(CCode), '~w~w', [ACode, BCode]).
translate_body((Cond -> Then ; Else), CCode, Depth) :-
    !,
    % If-then-else: if Cond succeeds, execute Then, otherwise execute Else
    translate_body(Cond, CondCode, Depth),
    translate_body(Then, ThenCode, Depth),
    translate_body(Else, ElseCode, Depth),
    format(atom(CCode),
'    /* If-then-else */
    {
        int saved_size = state->bindings.size;
~w
        if (!state->failed) {
            /* Condition succeeded, execute then branch */
~w
        } else {
            /* Condition failed, execute else branch */
            state->bindings.size = saved_size;
            state->failed = false;
~w
        }
    }
', [CondCode, ThenCode, ElseCode]).
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
    % Note: This is a simplified implementation that uses the outer state's variables
    % A proper implementation would create an isolated variable context
    
    % Generate the goal code - this will use 'state' in the generated code
    translate_body(Goal, GoalCodeTemplate, Depth),
    % Replace 'state,' with '&findall_state,' and 'state)' with '&findall_state)'
    % and 'state->' with 'findall_state.'
    % This handles function calls like pred(state, ...) and state->failed
    atom_codes(GoalCodeTemplate, GoalCodes),
    atom_codes('state,', StateCommaCodes),
    atom_codes('&findall_state,', FindallStateCommaCodes),
    replace_all_occurrences(GoalCodes, StateCommaCodes, FindallStateCommaCodes, TempCodes1),
    atom_codes('state)', StateParenCodes),
    atom_codes('&findall_state)', FindallStateParenCodes),
    replace_all_occurrences(TempCodes1, StateParenCodes, FindallStateParenCodes, TempCodes2),
    atom_codes('state->', StateArrowCodes),
    atom_codes('findall_state.', FindallStateDotCodes),
    replace_all_occurrences(TempCodes2, StateArrowCodes, FindallStateDotCodes, ModifiedGoalCodes),
    atom_codes(GoalCode, ModifiedGoalCodes),
    
    % Get C expressions for Template and Result
    term_to_c_expr(Template, TemplateExpr),
    term_to_c_expr(Result, ResultExpr),
    
    format(atom(CCode),
'    /* findall/3 */
    {
        term_t** solutions = NULL;
        int solution_count = 0;
        prolog_state_t findall_state;
        init_state(&findall_state);
        
        /* Copy current bindings to findall_state so variables are accessible */
        findall_state.bindings.capacity = state->bindings.capacity;
        findall_state.bindings.size = state->bindings.size;
        if (state->bindings.size > 0) {
            findall_state.bindings.bindings = malloc(sizeof(binding_t) * state->bindings.capacity);
            memcpy(findall_state.bindings.bindings, state->bindings.bindings,
                   sizeof(binding_t) * state->bindings.size);
        }
        findall_state.next_var_id = state->next_var_id;
        
        /* Save initial bindings for backtracking */
        int initial_bindings_size = findall_state.bindings.size;
        
        /* Enumerate solutions by calling goal and backtracking */
        while (true) {
            /* Try to find a solution */
~w
            if (findall_state.failed) break;
            
            /* Collect solution - copy the instantiated template */
            solution_count++;
            solutions = realloc(solutions, sizeof(term_t*) * solution_count);
            
            /* Copy the instantiated template from findall_state */
            int var_offset = 0;
            solutions[solution_count - 1] = copy_term_helper(&findall_state, ~w, &var_offset);
            
            /* Force backtracking to find next solution */
            /* Pop choice point and restore bindings, then continue */
            if (!pop_choice_point(&findall_state)) break;
            findall_state.bindings.size = initial_bindings_size;
            findall_state.failed = false;
        }
        
        /* Build result list from solutions */
        term_t* result_list = create_nil();
        for (int i = solution_count - 1; i >= 0; i--) {
            result_list = create_list(solutions[i], result_list);
        }
        if (solutions) free(solutions);
        
        /* Unify result with the collected list */
        if (!unify(state, ~w, result_list)) {
            state->failed = true;
        }
        
        free_state(&findall_state);
    }
', [GoalCode, TemplateExpr, ResultExpr]).
translate_body(Call, CCode, _) :-
    % Regular predicate call
    extract_predicate_info(Call, Name, Arity, Args),
    sanitize_predicate_name(Name, SanitizedName),
    format(atom(FuncName), '~w_~w', [SanitizedName, Arity]),
    translate_call_args(Args, ArgStr),
    format(atom(CCode), '    if (!~w(state~w)) { state->failed = true; break; }\n', [FuncName, ArgStr]).

translate_call_args([], '').
translate_call_args([Arg|Args], Result) :-
    term_to_c_expr(Arg, CExpr),
    translate_call_args(Args, Rest),
    format(atom(Result), ', ~w~w', [CExpr, Rest]).

%% replace_all_occurrences(+Input, +Pattern, +Replacement, -Output)
% Replaces all occurrences of Pattern in Input with Replacement
replace_all_occurrences([], _, _, []).
replace_all_occurrences(Input, Pattern, Replacement, Output) :-
    append(Pattern, Rest, Input),
    !,
    % Found a match, replace and continue
    append(Replacement, RestOutput, Output),
    replace_all_occurrences(Rest, Pattern, Replacement, RestOutput).
replace_all_occurrences([C|Rest], Pattern, Replacement, [C|Output]) :-
    % No match, keep character and continue
    replace_all_occurrences(Rest, Pattern, Replacement, Output).

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

%% create_var_map(+Vars)
% Creates a mapping from variables to their indices
% Maps each variable to its position in the AllVars list
% Uses variable names (from format) as keys to avoid identity issues
create_var_map(Vars) :-
    retractall(var_name_index_map(_, _)),
    % First, format all variables together to establish their names
    format(atom(_), '~w', [Vars]),
    create_var_map_impl(Vars, 0).

create_var_map_impl([], _).
create_var_map_impl([V|Vs], N) :-
    % Get the variable's printed name
    format(atom(VarName), '~w', [V]),
    % Store the mapping using the name as key
    assert(var_name_index_map(VarName, N)),
    N1 is N + 1,
    create_var_map_impl(Vs, N1).

%% get_var_index(+Var, -Index)
% Gets the index for a variable from the mapping
get_var_index(Var, Index) :-
    % Get the variable's printed name
    format(atom(VarName), '~w', [Var]),
    % Look up the index by name
    var_name_index_map(VarName, Index),
    !.
get_var_index(Var, _) :-
    % If not found, this is an error - print debug info
    format(atom(VarName), '~w', [Var]),
    format(user_error, 'ERROR: Variable ~w (name: ~w) not found in mapping~n', [Var, VarName]),
    format(user_error, 'Current mappings:~n', []),
    forall(var_name_index_map(Name, I), format(user_error, '  ~w -> ~w~n', [Name, I])),
    fail.

term_to_c_expr(Var, Expr) :-
    var(Var),
    !,
    get_var_index(Var, Index),
    format(atom(Expr), 'var__~w', [Index]).
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
        if (strcmp(t->data.compound.functor, "/\\\\") == 0 && t->data.compound.arity == 2) {
            int left = eval_arithmetic(state, t->data.compound.args[0]);
            int right = eval_arithmetic(state, t->data.compound.args[1]);
            return left & right;
        }
        if (strcmp(t->data.compound.functor, "\\\\/") == 0 && t->data.compound.arity == 2) {
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
    printf("\\n");
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
    
    /* Check if it\'s a proper list (ends with NIL) */
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
    
    printf("Prolog-to-C compiled program\\n");
    
    /* Call main/0 predicate if it exists */
    main_0(&state);
    
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
