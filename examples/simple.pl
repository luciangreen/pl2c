% simple.pl - Simple Prolog examples for testing

% Facts
parent(tom, bob).
parent(tom, liz).
parent(bob, ann).
parent(bob, pat).
parent(pat, jim).

% Rules
grandparent(X, Z) :- parent(X, Y), parent(Y, Z).

% List member
member(X, [X|_]).
member(X, [_|T]) :- member(X, T).

% List append
append([], L, L).
append([H|T1], L2, [H|T3]) :- append(T1, L2, T3).

% Factorial
factorial(0, 1).
factorial(N, F) :- 
    N > 0,
    N1 is N - 1,
    factorial(N1, F1),
    F is N * F1.

% Test with cut
max(X, Y, X) :- X >= Y, !.
max(_, Y, Y).

main :-
    write('Testing simple predicates\n'),
    member(2, [1, 2, 3]),
    write('member(2, [1,2,3]) succeeded\n'),
    factorial(5, F),
    format('factorial(5, ~w)\n', [F]),
    grandparent(tom, ann),
    write('grandparent(tom, ann) succeeded\n').
