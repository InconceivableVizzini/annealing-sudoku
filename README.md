# A sudoku solver

This program solves sudoku puzzles via simulated annealing. This is a quickly produced program intended to learn about monte carlo simulation, not an exercise in producing quality portable software. The code was automatically formatted by clang-format using the Chromium project's style.

This same program can be represented in a brief and clearly expressed manner using finite domain constraint solvers,

```
(defn sudoku [hints]
  (let [vars (repeatedly 81 lvar)
        rows (->rows vars)
		cols (->cols rows)
		sqs  (->squares rows)]
    (run-nc 1 [q]
	  (== q vars)
	  (everyg #(fd/in % (fd/domain 1 2 3 4 5 6 7 8 9)) vars)
	  (init vars hints)
	  (everyg fd/distinct rows)
	  (everyg fd/distinct cols)
	  (everyg fd/distinct sqs))))
```
