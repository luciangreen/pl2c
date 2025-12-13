% cut_example.pl - Demonstrates cut (!) behavior

% Without cut - finds all solutions
%likes(mary, food).
%likes(mary, wine).
%likes(john, wine).
%likes(john, mary).

% With cut - commits to first solution
%first_like(Person, Thing) :-
%    likes(Person, Thing), !.

% Cut in conditional
max(X, Y, X) :- X >= Y, !.
max(_, Y, Y).

% Cut to prevent infinite recursion
safe_member(_, []) :- !, fail.
safe_member(X, [X|_]) :- !.
safe_member(X, [_|T]) :- safe_member(X, T).

% Negation as failure using cut
not_member(_, []).
not_member(X, [H|T]) :- X \= H, not_member(X, T).

% Green cut example - doesn't change semantics
green_cut(X, Y, Z) :-
    X >= Y,
    !,
    Z = X.
green_cut(_, Y, Y).

main :-
    write('Testing cut behavior\n'),
    %first_like(mary, Thing),
    %format('First thing Mary likes: ~w\n', [Thing]),
    max(5, 3, M1),
    format('max(5, 3) = ~w\n', [M1]),
    max(2, 7, M2),
    format('max(2, 7) = ~w\n', [M2]),
    safe_member(A,[2,3,4]),
    format('safe_member(A,[2,3,4]) = ~w\n', [A]),
    green_cut(3,2,B),
    format('green_cut(3,2,B) = ~w\n', [B]),
    safe_member([c,C],[[a,a],[b,b],[c,c]]),
    format('member([c,C],[[a,a],[b,b],[c,c]]) = ~w\n', [C]).