relax
=====

reinforcement-learning algorithm experiments

TODO:
 * cmake build-system
 * logging & timing facilities
 * multi-threading (multiple policies per thread)

NOTES:
 * code under src/ should be compiled with
        g++ -Wall -Wextra -g -O2  -o relax  -DDEBUG  *.cpp learners/*.cpp tasks/*.cpp util/*.cpp  -llua5.1
 * output data can be visualized with gnuplot
        // reward per learning episode
        set xlabel 'policy-learning episode [1, N]'
        set ylabel 'policy-learning reward [-inf, +inf],'
        plot 'random-train-avg.dat' with lines, 'chosen-train-avg.dat' with lines

        // reward per policy evaluation
        set xlabel 'policy-evaluation trial [1, N]'
        set ylabel 'policy-evaluation reward [-inf, +inf]'
        plot 'random-trial-avg.dat' with lines, 'chosen-trial-avg.dat' with lines

 * "learning" means roughly the same as "training" does in other ML contexts
 * "evaluating" means roughly the same as "testing" does in other ML contexts
   (executing a policy from an initial state, taking the actions it specifies and gathering reward)
 * "training" a policy P means to execute a learning episode for P
 * "trialing" a policy P means to execute an evaluation round of P

