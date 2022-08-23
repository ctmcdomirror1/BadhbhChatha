# Badhbh Chatha
An incomplete attempt at sampling for an upperbound on number of positions in chess.

* I haven't documented much
* there's probably a bug in the sample tree
* I've tried to optmise the size of the sample tree by generalising piece types while also taking into account differences in base piece numbers
* the filters are unfinished, and the check filter could do with some more test cases
* the pawn filter should use a constraint solver to account for forced captures (due to check constraints) and just-promoted pieces, then there'd be no need for the Hungarian algorithm dependency
* I'd personally add a function which takes a chess position by means of what's on each row (defined by a string) so test cases can be written visually without having to configure the position 'manually' as I do
