# Jessica
Compute the (likely) number of chess positions to be ~10^44.

We compute, in under 1 minute on an average laptop, that the order of magnitude of the number of reachable positions in chess is **44**, i.e. it looks like:
<pre>000000<strong>XXXX
XXXXXXXXXX
XXXXXXXXXX
XXXXXXXXXX
XXXXXXXXXX
</strong></pre>
where every **X** is a decimal digit.

Or if each digit is a different letter / capitalised letter:

> <strong>abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQR</strong>

To put this in perspective, 10^44 is greater than the square root of the estimated number of atoms in the universe.

### Details
We compute a probabilistic upper bound (95% CI) of just less than 1E45 by building a sample space (which contains at least every reachable position) of size 5E46 and filtering out unreachable positions using tactics.  Glancing at the filtered positions it's apparent that > 1/10 of the positions are reachable on average, and hence the true number of chess positions is > 1.0E44.

We use an average 4-core laptop with hyper-threading support. Building the state space / tree takes 0.5 seconds: the majority of the remaining time is spent in OR-tools, which is probably due to both my implementation and the cost of using a general solver.

The program requires less than 1 GB of memory.

### Dependencies
And the versions I've used:
* A processor which supports ABM, BMI and BMI2
* gcc 11.3.0
* GMP 6.2.1
* OR-tools, master/92eab40155d5566039c92662632f196174f8da47
* NumPy 1.24.1
* ClangFormat
* Black
