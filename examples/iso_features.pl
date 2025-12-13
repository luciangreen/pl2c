% iso_features.pl - Test ISO Prolog standard features

% Test type checking predicates
test_type_checking :-
    write('Testing type checking predicates:\n'),
    (atom(hello) -> write('  atom(hello) - passed\n') ; write('  atom(hello) - failed\n')),
    (integer(42) -> write('  integer(42) - passed\n') ; write('  integer(42) - failed\n')),
    (number(42) -> write('  number(42) - passed\n') ; write('  number(42) - failed\n')),
    X = 5,
    (nonvar(X) -> write('  nonvar(5) - passed\n') ; write('  nonvar(5) - failed\n')),
    (atomic(hello) -> write('  atomic(hello) - passed\n') ; write('  atomic(hello) - failed\n')),
    (is_list([1,2,3]) -> write('  is_list([1,2,3]) - passed\n') ; write('  is_list([1,2,3]) - failed\n')),
    (ground(foo(1,2)) -> write('  ground(foo(1,2)) - passed\n') ; write('  ground(foo(1,2)) - failed\n')).

% Test list predicates
test_list_predicates :-
    write('\nTesting list predicates:\n'),
    length([1,2,3], L),
    format('  length([1,2,3], ~w)\n', [L]),
    nth0(1, [a,b,c], E),
    format('  nth0(1, [a,b,c], ~w)\n', [E]),
    last([1,2,3], Last),
    format('  last([1,2,3], ~w)\n', [Last]),
    reverse([1,2,3], Rev),
    format('  reverse([1,2,3], ~w)\n', [Rev]).

% Test atom predicates
test_atom_predicates :-
    write('\nTesting atom predicates:\n'),
    atom_codes(hello, Codes),
    format('  atom_codes(hello, ~w)\n', [Codes]),
    atom_chars(world, Chars),
    format('  atom_chars(world, ~w)\n', [Chars]),
    atom_length(hello, Len),
    format('  atom_length(hello, ~w)\n', [Len]),
    atom_concat(hello, world, Result),
    format('  atom_concat(hello, world, ~w)\n', [Result]).

% Test term manipulation
test_term_manipulation :-
    write('\nTesting term manipulation:\n'),
    functor(foo(a,b,c), F, A),
    format('  functor(foo(a,b,c), ~w, ~w)\n', [F, A]),
    arg(2, foo(a,b,c), Arg),
    format('  arg(2, foo(a,b,c), ~w)\n', [Arg]),
    foo(a,b) =.. List,
    format('  foo(a,b) =.. ~w\n', [List]).

% Test arithmetic functions
test_arithmetic :-
    write('\nTesting arithmetic:\n'),
    X is abs(-5),
    format('  abs(-5) = ~w\n', [X]),
    Y is max(10, 5),
    format('  max(10, 5) = ~w\n', [Y]),
    Z is min(10, 5),
    format('  min(10, 5) = ~w\n', [Z]),
    P is 2 ^ 3,
    format('  2 ^ 3 = ~w\n', [P]),
    M is 10 mod 3,
    format('  10 mod 3 = ~w\n', [M]),
    S is sign(-10),
    format('  sign(-10) = ~w\n', [S]),
    R is round(3),
    format('  round(3) = ~w\n', [R]).

% Test term comparison
test_term_comparison :-
    write('\nTesting term comparison:\n'),
    (1 @< 2 -> write('  1 @< 2 - passed\n') ; write('  1 @< 2 - failed\n')),
    (atom @< foo(1) -> write('  atom @< foo(1) - passed\n') ; write('  atom @< foo(1) - failed\n')),
    compare(Order, 1, 2),
    format('  compare(~w, 1, 2)\n', [Order]).

% Test I/O predicates
test_io :-
    write('\nTesting I/O predicates:\n'),
    write('  write works'), nl,
    tab(5), write('  tab(5) works\n').

% Test sorting predicates
test_sorting :-
    write('\nTesting sorting predicates:\n'),
    sort([3,1,2,1,3], S1),
    format('  sort([3,1,2,1,3], ~w) - removes duplicates\n', [S1]),
    msort([3,1,2,1,3], S2),
    format('  msort([3,1,2,1,3], ~w) - keeps duplicates\n', [S2]).

% Main test predicate
main :-
    write('=== ISO Prolog Features Test ===\n\n'),
    test_type_checking,
    test_list_predicates,
    test_atom_predicates,
    test_term_manipulation,
    test_arithmetic,
    test_term_comparison,
    test_io,
    test_sorting,
    write('\n=== All Tests Completed ===\n').
