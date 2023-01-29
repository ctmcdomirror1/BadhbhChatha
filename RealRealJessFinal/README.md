# Jessica
Compute the (likely) number of digits in the number of chess positions in under 1 minute on an average laptop.

We compute that the likely number of digits in the number of reachable chess positions is 44.
That is, it should look like:
<pre><strong>      XXXX
XXXXXXXXXX
XXXXXXXXXX
XXXXXXXXXX
XXXXXXXXXX
</strong></pre>
We use an average 4-core laptop with hyper-threading support. The program uses less than 1 GB of memory.

We compute a probabilistic upper bound (95% CI) of 1E45 by building a sample space and filtering out unreachable positions using tactics.  Inspecting the filtered positions it's apparent that > 1/10 of the positions are reachable on average, and hence the true number of chess positions is likely > 1E44.

### Dependencies
And the versions I've used:
* A processor which supports ABM, BMI and BMI2
* gcc 11.3.0
* GMP 6.2.1
* OR-tools, main/92eab40155d5566039c92662632f196174f8da47
* NumPy 1.24.1
* ClangFormat
* Black

