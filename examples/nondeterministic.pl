% nondeterministic.pl - Examples of nondeterministic predicates

% Choice points example
color(red).
color(green).
color(blue).

% Nondeterministic search
path(a, b).
path(b, c).
path(c, d).
path(a, e).
path(e, d).

reachable(X, X).
reachable(X, Z) :- path(X, Y), reachable(Y, Z).

% findall example
all_colors(Colors) :- findall(C, color(C), Colors).

% Choice with cut
first_solution(X) :- color(X), !.

main :-
    write('Testing nondeterministic predicates\n'),
    findall(C, color(C), Colors),
    format('All colors: ~w\n', [Colors]),
    reachable(a, d),
    write('Path from a to d exists\n'),
    first_solution(C),
    format('First color: ~w\n', [C]).
