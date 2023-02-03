import itertools as it
import math
import numpy as np

# Can have 0, 1, or 2 rooks with castling rights
NUM_FIXED_ROOK_SCENARIOS = 3

# Dimension 0 represents the number of rooks with castling rights (fixed rooks).
# Dimension 1 represents the number of base pieces of each type. For 0 and 1
# fixed rooks this is (b, n, r, q) and for 2 fixed rooks it's (b, n, q, r). The
# reason for varying the order is we want to keep the values in decreasing order.
# This relates to how we generate unique sides of generic pieces.
NUM_NON_FIXED_CAPTURABLE_BASE_PIECES = [[2, 2, 2, 1], [2, 2, 1, 1], [2, 2, 1, 0]]
######################################## b, n, r, q    b, n, r, q    b, n, q, r
########################################################################## <-->
###################################### Notice how we switch queens and rooks ^

# A covered set is a tuple (a, b, c, d) where 2 >= a >= b >= c >= d >= 0.
# We convert a set of generic pieces to a covered set by enumerating over the
# set and capping the number of a particular piece at 2, because 2 is the
# maximum number of any one base piece. A permutation's cost arises from different
# piece types having different numbers of base pieces (1 in the case of queen,
# 2 for knights/bishops and varying amounts of free rooks due to castling rights),
# so naturally we only need to consider whether the number of a certain piece
# type is 0, 1 or >= 2.
NUM_COVERED_SETS = 15

# We ultimately need to output combinations and not permutations if there are at
# least any two pieces of the same colour and number. Given that our pieces are
# in decreasing order, there are 8 patterns we use to convert the perms to piece
# combinations: (a, b, c, d), (a, a, b, c), (a, b, b, c), (a, b, c, c)
#               (a, a, b, b), (a, a, a, b), (a, b, b, b), (a, a, a, a)
NUM_FUNDAMENTAL_SETS = 8

# We can only start with at most 2 of any piece type (of the same colour)
MAX_OF_ANY_ONE_BASE_PIECE = 2

# |{b, n, r, q}| == 4
NUM_PIECE_TYPES_LESS_KING = 4

# (2, 2, 1, 0) gives rise to permutation with greatest cost: (1, 0, 2, 2)
# which is base cost + 3. There is also base, base + 1 and base + 2,
# so 4 in total
NUM_UNIQUE_PERM_COSTS = 4

CS_INDEX_SHAPE = (
    MAX_OF_ANY_ONE_BASE_PIECE + 1,
    MAX_OF_ANY_ONE_BASE_PIECE + 1,
    MAX_OF_ANY_ONE_BASE_PIECE + 1,
    MAX_OF_ANY_ONE_BASE_PIECE + 1,
)
COMBS_SHAPE = (
    NUM_FIXED_ROOK_SCENARIOS,
    NUM_COVERED_SETS,
    NUM_FUNDAMENTAL_SETS,
    math.factorial(NUM_PIECE_TYPES_LESS_KING),
    NUM_PIECE_TYPES_LESS_KING,
)
ADDN_COST_TO_NPERMS_SHAPE = (
    NUM_FIXED_ROOK_SCENARIOS,
    NUM_COVERED_SETS,
    NUM_UNIQUE_PERM_COSTS,
)

# For (a, b, c, d), return an associated index
coveredSet_index = np.negative(np.ones(CS_INDEX_SHAPE, np.int32))

# For a fixed rook scenario, a covered set index, a fundamental set index and a
# comb number (which will be in [0, 24)), return the combination.
# Combinations are partially ordered by minimum additional promotions
fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb = np.negative(
    np.ones(COMBS_SHAPE, np.int32)
)

# For a fixed rook scenario, a covered set index, and a minimum additional promotions
# in {0, 1, 2, 3}, return the number of promotions with less than or equal to the
# input minimum additional promotions
fr_coveredSetIndex_permAddnCost_numPerms = np.negative(
    np.ones(ADDN_COST_TO_NPERMS_SHAPE, np.int32)
)


def compute_covered_sets():
    covered_sets = []
    s = [
        MAX_OF_ANY_ONE_BASE_PIECE,
        MAX_OF_ANY_ONE_BASE_PIECE,
        MAX_OF_ANY_ONE_BASE_PIECE,
        MAX_OF_ANY_ONE_BASE_PIECE,
    ]

    cset_num = 0
    coveredSet_index[s[0]][s[1]][s[2]][s[3]] = 0
    covered_sets.append((s.copy(), 0))

    cset_num += 1
    while s[0] != 0:
        # generate next covered set
        for i in range(NUM_PIECE_TYPES_LESS_KING - 1, -1, -1):
            if s[i] != 0:
                s[i] -= 1
                for j in range(i + 1, NUM_PIECE_TYPES_LESS_KING):
                    s[j] = s[i]

                # and assign it an index
                coveredSet_index[s[0]][s[1]][s[2]][s[3]] = cset_num
                covered_sets.append((s.copy(), cset_num))

                cset_num += 1
                break
    return covered_sets


def cost(s, b):
    c = 0
    for i in range(NUM_PIECE_TYPES_LESS_KING):
        diff = b[i] - s[i]
        if diff > 0:
            c += diff
    return c


def c_arr_literal_str_helper(a, depth):
    s = "{"
    if len(a.shape) == 1:
        for i in range(a.shape[0] - 1):
            s += str(a[i]) + ", "
        s += str(a[a.shape[0] - 1])
    else:
        for i in range(a.shape[0]):
            s += c_arr_literal_str_helper(a[i], depth + 1)
    s += "}"
    if depth != 0:
        s += ", "
    return s


def c_arr_literal_str(a):
    return c_arr_literal_str_helper(a, 0)


def c_dimensions_str(shape):
    s = ""
    for dim in shape:
        s += "[" + str(dim) + "]"
    return s


def format_c_arr_str(a, npname):
    s = "int "
    s += npname
    s += c_dimensions_str(np.shape(a))
    s += " = "
    s += c_arr_literal_str(a)
    s += ";"
    return s


def compute_fundamentalSet_variations(parent, child, orderIndex):
    _i = 0
    for i in range(math.factorial(NUM_PIECE_TYPES_LESS_KING)):
        for j in range(NUM_PIECE_TYPES_LESS_KING - 1):
            if parent[i][j] == orderIndex - 1:
                for k in range(NUM_PIECE_TYPES_LESS_KING):
                    child[_i][k] = parent[i][k]
                _i += 1
            elif parent[i][j] == orderIndex:
                break


covered_sets = compute_covered_sets()

for i in range(NUM_FIXED_ROOK_SCENARIOS):
    for (s, cset_num) in covered_sets:
        base_cost = cost(s, NUM_NON_FIXED_CAPTURABLE_BASE_PIECES[i])
        cost_to_perms = {}
        for p in it.permutations((0, 1, 2, 3)):
            permuted_set = [s[pe] for pe in p]
            c = cost(permuted_set, NUM_NON_FIXED_CAPTURABLE_BASE_PIECES[i]) - base_cost
            if c not in cost_to_perms:
                cost_to_perms[c] = [tuple(p)]
            else:
                cost_to_perms[c].append(tuple(p))
        c = 0
        pi = 0
        for c, perms in sorted(cost_to_perms.items()):
            for e in perms:
                for j in range(NUM_PIECE_TYPES_LESS_KING):
                    fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][
                        0
                    ][pi][j] = e[j]
                pi += 1

            fr_coveredSetIndex_permAddnCost_numPerms[i][cset_num][c] = pi

        to_be_copied = fr_coveredSetIndex_permAddnCost_numPerms[i][cset_num][c]
        for j in range(c + 1, NUM_UNIQUE_PERM_COSTS):
            fr_coveredSetIndex_permAddnCost_numPerms[i][cset_num][j] = to_be_copied

        # 0 = (a, b, c, d)
        # 1   = (a, a, b, c)
        # 2     = (a, a, a, b)
        # 3       = (a, a, a, a)
        # 4     = (a, a, b, b)
        # 5   = (a, b, b, c)
        # 6     = (a, b, b, b)
        # 7   = (a, b, c, c)

        # 1   = (a, a, b, c)
        compute_fundamentalSet_variations(
            parent=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][
                0
            ],
            child=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][1],
            orderIndex=1,
        )

        # 2     = (a, a, a, b)
        compute_fundamentalSet_variations(
            parent=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][
                1
            ],
            child=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][2],
            orderIndex=2,
        )

        # 3       = (a, a, a, a)
        compute_fundamentalSet_variations(
            parent=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][
                2
            ],
            child=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][3],
            orderIndex=3,
        )

        # 4     = (a, a, b, b)
        compute_fundamentalSet_variations(
            parent=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][
                1
            ],
            child=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][4],
            orderIndex=3,
        )

        # 5   = (a, b, b, c)
        compute_fundamentalSet_variations(
            parent=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][
                0
            ],
            child=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][5],
            orderIndex=2,
        )

        # 6     = (a, b, b, b)
        compute_fundamentalSet_variations(
            parent=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][
                5
            ],
            child=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][6],
            orderIndex=3,
        )

        # 7   = (a, b, c, c)
        compute_fundamentalSet_variations(
            parent=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][
                0
            ],
            child=fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb[i][cset_num][7],
            orderIndex=3,
        )

f = open("tree_piece_perm.c", "w")
f.write("// Generated by gen_prom_perms.py.\n")
f.write("// Formatted by clang thereafter\n\n")
f.write('#include "tree_common.h"\n\n')
f.write(format_c_arr_str(coveredSet_index, "coveredSet_index"))
f.write("\n\n")
f.write(
    format_c_arr_str(
        fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb,
        "fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb",
    )
)
f.write("\n\n")
f.write(
    format_c_arr_str(
        fr_coveredSetIndex_permAddnCost_numPerms,
        "fr_coveredSetIndex_permAddnCost_numPerms",
    )
)
