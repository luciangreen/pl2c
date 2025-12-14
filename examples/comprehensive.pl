% comprehensive.pl - Comprehensive example demonstrating all PL2C features

%% 1. FACTS AND SIMPLE RULES
% Basic facts
city(london).
city(paris).
city(tokyo).
city(newYork).

country(uk, london).
country(france, paris).
country(japan, tokyo).
country(usa, newYork).

% Simple rule
capital(Country, City) :- country(Country, City).

%% 2. RECURSIVE PREDICATES
% List length
list_length([], 0).
list_length([_|T], N) :- 
    list_length(T, N1),
    N is N1 + 1.

% List reversal
reverse_list([], []).
reverse_list([H|T], R) :- 
    reverse_list(T, RT),
    append(RT, [H], R).

% Sum of list
sum_list([], 0).
sum_list([H|T], Sum) :-
    sum_list(T, RestSum),
    Sum is H + RestSum.

%% 3. NONDETERMINISTIC PREDICATES
% Generate numbers
between(Low, High, Low) :- Low =< High.
between(Low, High, X) :-
    Low < High,
    Low1 is Low + 1,
    between(Low1, High, X).

%% 4. CUT EXAMPLES
% Deterministic maximum
max(X, Y, X) :- X >= Y, !.
max(_, Y, Y).

% Deterministic minimum
min(X, Y, X) :- X =< Y, !.
min(_, Y, Y).

% If-then-else using cut
if_then_else(Cond, Then, _) :- Cond, !, Then.
if_then_else(_, _, Else) :- Else.

%% 5. LIST OPERATIONS
% Basic append (already defined)
append([], L, L).
append([H|T], L2, [H|R]) :- append(T, L2, R).

% Member check
member(X, [X|_]).
member(X, [_|T]) :- member(X, T).

% Last element
last_elem([X], X).
last_elem([_|T], X) :- last_elem(T, X).

% Nth element (0-indexed)
nth(0, [H|_], H).
nth(N, [_|T], X) :- 
    N > 0,
    N1 is N - 1,
    nth(N1, T, X).

%% 6. HIGHER-ORDER PATTERNS
% Filter positive numbers
positive_numbers([], []).
positive_numbers([H|T], [H|R]) :- 
    H > 0, 
    !,
    positive_numbers(T, R).
positive_numbers([_|T], R) :- 
    positive_numbers(T, R).

% Map double
map_double([], []).
map_double([H|T], [H2|R]) :-
    H2 is H * 2,
    map_double(T, R).

%% 7. FINDALL EXAMPLES
% Collect all cities
all_cities(Cities) :- findall(C, city(C), Cities).

% Collect all countries
all_countries(Countries) :- findall(C, country(C, _), Countries).

%% 8. COMPLEX QUERIES
% Find all capitals
find_capitals(Capitals) :- 
    findall(capital(Country, City), capital(Country, City), Capitals).

% Pairs of cities in same country
same_country_pair(C1, C2) :-
    country(Country, C1),
    country(Country, C2),
    C1 \= C2.

%% 9. ARITHMETIC PREDICATES
% Factorial
factorial(0, 1) :- !.
factorial(N, F) :-
    N > 0,
    N1 is N - 1,
    factorial(N1, F1),
    F is N * F1.

% Fibonacci
fibonacci(0, 0) :- !.
fibonacci(1, 1) :- !.
fibonacci(N, F) :-
    N > 1,
    N1 is N - 1,
    N2 is N - 2,
    fibonacci(N1, F1),
    fibonacci(N2, F2),
    F is F1 + F2.

% GCD (Greatest Common Divisor)
gcd(X, 0, X) :- !.
gcd(X, Y, G) :-
    Y > 0,
    R is X mod Y,
    gcd(Y, R, G).

%% 10. SORTING (Insertion Sort)
insert_sorted(X, [], [X]).
insert_sorted(X, [H|T], [X,H|T]) :- X =< H, !.
insert_sorted(X, [H|T], [H|R]) :- 
    X > H,
    insert_sorted(X, T, R).

insertion_sort([], []).
insertion_sort([H|T], Sorted) :-
    insertion_sort(T, SortedT),
    insert_sorted(H, SortedT, Sorted).

%% MAIN TEST PREDICATE
main :-
    write('=== PL2C Comprehensive Test ===\n\n'),
    
    % Test 1: Facts and rules
    write('Test 1: Facts and Rules\n'),
    capital(uk, C1),
    format('  Capital of UK: ~w\n', [C1]),
    
    % Test 2: List operations
    write('\nTest 2: List Operations\n'),
    append([1,2], [3,4], L1),
    format('  append([1,2], [3,4]) = ~w\n', [L1]),
    member(2, [1,2,3]),
    write('  member(2, [1,2,3]) succeeded\n'),
    
    % Test 3: Arithmetic
    write('\nTest 3: Arithmetic\n'),
    factorial(5, F5),
    format('  factorial(5) = ~w\n', [F5]),
    
    % Test 4: Cut behavior
    write('\nTest 4: Cut Behavior\n'),
    max(10, 5, Max1),
    format('  max(10, 5) = ~w\n', [Max1]),
    min(10, 5, Min1),
    format('  min(10, 5) = ~w\n', [Min1]),
    
    % Test 5: Recursive predicates
    write('\nTest 5: Recursive Predicates\n'),
    reverse_list([1,2,3], Rev),
    format('  reverse([1,2,3]) = ~w\n', [Rev]),

	%all_cities(Cities),
    %format('  all_cities(Cities) = ~w\n', [Cities]),

	%all_countries(Countries),
    %format('  all_countries(Countries) = ~w\n', [Countries]),

	%find_capitals(Capitals), 
    %format('  find_capitals(Capitals) = ~w\n', [Capitals]),
    
    write('\n=== All Tests Completed ===\n').
