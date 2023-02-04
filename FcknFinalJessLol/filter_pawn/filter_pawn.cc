#include "filter_pawn.h"

#include <immintrin.h>
#include <inttypes.h>
#include <nmmintrin.h>
#include <stdint.h>
#include <stdlib.h>

#include "ortools/base/logging.h"
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model.pb.h"
#include "ortools/sat/cp_model_solver.h"
#include "ortools/util/sorted_interval_list.h"
#include <algorithm>
#include <vector>

using namespace std;

#define BOARD_SIDE_LENGTH                    8
#define MAX_COST_PSIDE                       25
#define NUM_CAPTURABLE_CHESSMEN_PSIDE        15
#define NUM_SQUARES                          (BOARD_SIDE_LENGTH * BOARD_SIDE_LENGTH)
#define NUM_PAWN_OCCUPIABLE_SQUARES_PER_FILE (BOARD_SIDE_LENGTH - 2)

#define FORWARD_SLASH 1
#define BACKSLASH     0

namespace operations_research
{
namespace sat
{

const Domain BOARD_SIDE_LENGTH_DOMAIN(0, BOARD_SIDE_LENGTH);
const Domain NUM_CAPTURABLE_CHESSMEN_PSIDE_DOMAIN(0, NUM_CAPTURABLE_CHESSMEN_PSIDE);
const Domain COST_PSIDE_DOMAIN(0, MAX_COST_PSIDE);
const Domain PAWN_RELATIVE_RANGE_DOMAIN(-NUM_PAWN_OCCUPIABLE_SQUARES_PER_FILE + 1,
                                        NUM_PAWN_OCCUPIABLE_SQUARES_PER_FILE
                                            - 1); // their magnitude less 1 to account
                                                  // for the file they're currently on
const Domain PROMOTIONS_DOMAIN(-BOARD_SIDE_LENGTH, BOARD_SIDE_LENGTH);
const Domain SINGLE_PAWN_COST_DOMAIN(0, NUM_PAWN_OCCUPIABLE_SQUARES_PER_FILE - 1);
const Domain ZERO_DOMAIN(0, 0);

extern "C"
{
    int get_row_num(int n)
    {
        return (n / BOARD_SIDE_LENGTH);
    }
    int get_col_num(int n)
    {
        return (n % BOARD_SIDE_LENGTH);
    }
}

extern "C" bool FilterPawn(uint64_t _pawns[NUM_SIDES],
                           uint64_t enpassant,
                           int      numCapturedBasePieces[NUM_SIDES],
                           int      minPromotions[NUM_SIDES])
{
    CpModelBuilder cp_model;
    // The subset of the starting squares (a2...h2 and a7...h7) which
    // pawns currently on the board are mapped to, i.e. we consider it
    // possible that these pawns started on these squares
    BoolVar        matchedStartingSquares[NUM_SIDES][BOARD_SIDE_LENGTH];
    // We consider a starting square 'covered' if it is in between an
    // opp pawn and its starting square match, i.e. if a pawn on file 'a'
    // is matched to starting square on file 'c' then the opposition
    // starting square on 'b' is covered. It means if our pawn which
    // started on 'b' is consumed we can consider it pawn-captured
    BoolVar        coveredStartingSquares[NUM_SIDES][BOARD_SIDE_LENGTH];
    // e.g. pawn currently on 'e' is matched to 'e'. Note that we actually
    // allow pawns of opposite colours to pass through each other for
    // matching purposes, so if a white pawn on a7 is considered to have
    // started on 'a' and a black pawn on a2 is also considered to start
    // on 'a' then there's no need for a capture to take place. This of
    // course isn't true in practice.
    BoolVar        fileSamePawnFileMatches[NUM_SIDES][BOARD_SIDE_LENGTH];
    // Pawns which didn't have to capture to promote
    BoolVar        zeroCostPromotions[NUM_SIDES][BOARD_SIDE_LENGTH];
    for (int i = 0; i < NUM_SIDES; i++)
    {
        for (int j = 0; j < BOARD_SIDE_LENGTH; j++)
        {
            matchedStartingSquares[i][j]  = cp_model.NewBoolVar();
            coveredStartingSquares[i][j]  = cp_model.NewBoolVar();
            fileSamePawnFileMatches[i][j] = cp_model.NewBoolVar();
            zeroCostPromotions[i][j]      = cp_model.NewBoolVar();
        }
    }

    vector<BoolVar> startingSquarePotentialMatches[NUM_SIDES][BOARD_SIDE_LENGTH];
    vector<BoolVar> startingSquarePotentialMatchesNegated[NUM_SIDES][BOARD_SIDE_LENGTH];
    vector<BoolVar> startingSquarePotentialSameFileMatches[NUM_SIDES][BOARD_SIDE_LENGTH];
    vector<BoolVar> startingSquarePotentialSameFileMatchesNegated[NUM_SIDES][BOARD_SIDE_LENGTH];
    vector<BoolVar> startingSquareCoverLiterals[NUM_SIDES][BOARD_SIDE_LENGTH];
    vector<BoolVar> startingSquareCoverLiteralsNegated[NUM_SIDES][BOARD_SIDE_LENGTH];

    vector<IntVar> pawnVars[NUM_SIDES];
    // Cost is the distance between a pawn file and its starting file.
    // So if a pawn is on 'a' and is matched to 'c' then the cost is |1 - 3| = 2
    vector<IntVar> pawnCosts[NUM_SIDES];
    IntVar         pawnCostSum[NUM_SIDES];
    uint64_t       pawns[] = { _pawns[0], _pawns[1] };
    for (int i = 0; i < NUM_SIDES; i++)
    {
        if (pawns[i] == 0)
        {
            pawnCostSum[i] = cp_model.NewIntVar(ZERO_DOMAIN);
            continue;
        }

        int opp       = (i + 1) % NUM_SIDES;
        int num_pawns = _mm_popcnt_u64(pawns[i]);
        pawnVars[i].reserve(num_pawns);
        pawnCosts[i].reserve(num_pawns);

        for (int p = 0; p < num_pawns; p++)
        {
            int tz   = __tzcnt_u64(pawns[i]);
            pawns[i] = __blsr_u64(pawns[i]);

            int col = get_col_num(tz);
            if (enpassant == (1UL << tz))
            {
                const Domain single_col_domain(col, col);
                pawnVars[0].push_back(cp_model.NewIntVar(single_col_domain));
                startingSquarePotentialMatches[0][col].push_back(cp_model.TrueVar());
                cp_model.FixVariable(matchedStartingSquares[0][col], true);
                cp_model.FixVariable(fileSamePawnFileMatches[0][col], true);
                pawnCosts[0].push_back(cp_model.NewIntVar(ZERO_DOMAIN));
            }
            else
            {
                int row = get_row_num(tz);
                int start;
                int end;
                if (i == 0)
                {
                    start = std::max(col - (row - 1), 0);
                    end   = std::min(col + (row - 1), BOARD_SIDE_LENGTH - 1);
                }
                else
                {
                    start = std::max(col - ((BOARD_SIDE_LENGTH - 2) - row), 0);
                    end   = std::min(col + ((BOARD_SIDE_LENGTH - 2) - row), BOARD_SIDE_LENGTH - 1);
                }
                const Domain d(start, end);
                pawnVars[i].push_back(cp_model.NewIntVar(d));
                for (int k = start; k < col; k++)
                {
                    BoolVar pIsMatchedToK = cp_model.NewBoolVar();
                    // https://developers.google.com/optimization/cp/channeling
                    // "Channeling is usually implemented using half-reified linear constraints:
                    // one constraint implies another (a → b),
                    // but not necessarily the other way around (a ← b)"
                    cp_model.AddEquality(pawnVars[i][p], k).OnlyEnforceIf(pIsMatchedToK);
                    cp_model.AddNotEqual(pawnVars[i][p], k).OnlyEnforceIf(pIsMatchedToK.Not());
                    startingSquarePotentialMatches[i][k].push_back(pIsMatchedToK);
                    startingSquarePotentialMatchesNegated[i][k].push_back(pIsMatchedToK.Not());

                    BoolVar pCoversK = cp_model.NewBoolVar();
                    cp_model.AddLessThan(pawnVars[i][p], k).OnlyEnforceIf(pCoversK);
                    cp_model.AddGreaterOrEqual(pawnVars[i][p], k).OnlyEnforceIf(pCoversK.Not());
                    startingSquareCoverLiterals[opp][k].push_back(pCoversK);
                    startingSquareCoverLiteralsNegated[opp][k].push_back(pCoversK.Not());
                }

                BoolVar pIsMatchedToFile = cp_model.NewBoolVar();
                cp_model.AddEquality(pawnVars[i][p], col).OnlyEnforceIf(pIsMatchedToFile);
                cp_model.AddNotEqual(pawnVars[i][p], col).OnlyEnforceIf(pIsMatchedToFile.Not());
                startingSquarePotentialMatches[i][col].push_back(pIsMatchedToFile);
                startingSquarePotentialMatchesNegated[i][col].push_back(pIsMatchedToFile.Not());
                startingSquarePotentialSameFileMatches[i][col].push_back(pIsMatchedToFile);
                startingSquarePotentialSameFileMatchesNegated[i][col].push_back(
                    pIsMatchedToFile.Not());

                startingSquareCoverLiterals[opp][col].push_back(pIsMatchedToFile.Not());
                startingSquareCoverLiteralsNegated[opp][col].push_back(pIsMatchedToFile);

                for (int k = col + 1; k <= end; k++)
                {
                    BoolVar pIsMatchedToK = cp_model.NewBoolVar();
                    cp_model.AddEquality(pawnVars[i][p], k).OnlyEnforceIf(pIsMatchedToK);
                    cp_model.AddNotEqual(pawnVars[i][p], k).OnlyEnforceIf(pIsMatchedToK.Not());
                    startingSquarePotentialMatches[i][k].push_back(pIsMatchedToK);
                    startingSquarePotentialMatchesNegated[i][k].push_back(pIsMatchedToK.Not());

                    BoolVar pCoversK = cp_model.NewBoolVar();
                    cp_model.AddGreaterThan(pawnVars[i][p], k).OnlyEnforceIf(pCoversK);
                    cp_model.AddLessOrEqual(pawnVars[i][p], k).OnlyEnforceIf(pCoversK.Not());
                    startingSquareCoverLiterals[opp][k].push_back(pCoversK);
                    startingSquareCoverLiteralsNegated[opp][k].push_back(pCoversK.Not());
                }

                pawnCosts[i].push_back(cp_model.NewIntVar(SINGLE_PAWN_COST_DOMAIN));
                cp_model.AddAbsEquality(pawnCosts[i][p], pawnVars[i][p] - col);
            }
        }
        cp_model.AddAllDifferent(pawnVars[i]);

        pawnCostSum[i] = cp_model.NewIntVar(COST_PSIDE_DOMAIN);
        cp_model.AddEquality(pawnCostSum[i], LinearExpr::Sum(pawnCosts[i]));
    }

    for (int i = 0; i < NUM_SIDES; i++)
    {
        for (int j = 0; j < BOARD_SIDE_LENGTH; j++)
        {
            // I filed a bug related to this check in ortools:
            // https://github.com/google/or-tools/issues/3591
            if (startingSquarePotentialMatches[i][j].size() > 0)
            {
                cp_model.AddBoolOr(startingSquarePotentialMatches[i][j])
                    .OnlyEnforceIf(matchedStartingSquares[i][j]);
                cp_model.AddBoolAnd(startingSquarePotentialMatchesNegated[i][j])
                    .OnlyEnforceIf(matchedStartingSquares[i][j].Not());
            }
            else
            {
                cp_model.FixVariable(matchedStartingSquares[i][j], false);
            }

            if (startingSquarePotentialSameFileMatches[i][j].size() > 0)
            {
                cp_model.AddBoolOr(startingSquarePotentialSameFileMatches[i][j])
                    .OnlyEnforceIf(fileSamePawnFileMatches[i][j]);
                cp_model.AddBoolAnd(startingSquarePotentialSameFileMatchesNegated[i][j])
                    .OnlyEnforceIf(fileSamePawnFileMatches[i][j].Not());
            }
            else
            {
                cp_model.FixVariable(fileSamePawnFileMatches[i][j], false);
            }

            if (startingSquareCoverLiterals[i][j].size() > 0)
            {
                cp_model.AddBoolOr(startingSquareCoverLiterals[i][j])
                    .OnlyEnforceIf(coveredStartingSquares[i][j]);
                cp_model.AddBoolAnd(startingSquareCoverLiteralsNegated[i][j])
                    .OnlyEnforceIf(coveredStartingSquares[i][j].Not());
            }
            else
            {
                cp_model.FixVariable(coveredStartingSquares[i][j], false);
            }
        }
    }

    BoolVar pawnConsumedPawnWhichStartedOn[NUM_SIDES][BOARD_SIDE_LENGTH];
    IntVar  numPawnsConsumedByPawnsOnBoard[] = { cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN),
                                                 cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN) };
    for (int i = 0; i < NUM_SIDES; i++)
    {
        for (int j = 0; j < BOARD_SIDE_LENGTH; j++)
        {
            // For a pawn which started on some starting square to be considered consumed,
            // it can't be matched to a pawn currently on the board of course, and it has to
            // be 'covered' by an opposition pawn: see definition of covered above the def^n
            // of the "coveredStartingSquares" variable
            BoolVar notMatchedButCovered[]
                = { matchedStartingSquares[i][j].Not(), coveredStartingSquares[i][j] };
            pawnConsumedPawnWhichStartedOn[i][j] = cp_model.NewBoolVar();
            cp_model.AddBoolAnd(notMatchedButCovered)
                .OnlyEnforceIf(pawnConsumedPawnWhichStartedOn[i][j]);
        }
        cp_model.AddEquality(numPawnsConsumedByPawnsOnBoard[i],
                             LinearExpr::Sum(pawnConsumedPawnWhichStartedOn[i]));
    }

    // Pawns on adjacent files where one pawn captured the other in order
    // to promote. We don't consider the square colour on which the pawn would
    // promote (which would matter for bishop promotions) so we might as well
    // only consider adjacent files. With this simplification it doesn't make
    // sense for a pawn to capture an opp piece or pawn in order to capture
    // someother pawn: it can promote on initial capture. And hence if considering
    // only pawn captures, then we only need to consider adjacent files.
    BoolVar pawnWhichStartedOn_isPaired[2][NUM_SIDES][BOARD_SIDE_LENGTH];
    for (int d = 0; d < 2; d++)
    { // d for direction which is either left or right
        for (int j = 0; j < NUM_SIDES; j++)
        {
            for (int k = 0; k < BOARD_SIDE_LENGTH; k++)
            {
                pawnWhichStartedOn_isPaired[d][j][k] = cp_model.NewBoolVar();
            }
        }
    }
    cp_model.FixVariable(pawnWhichStartedOn_isPaired[BACKSLASH][0][BOARD_SIDE_LENGTH - 1], false);
    cp_model.FixVariable(pawnWhichStartedOn_isPaired[BACKSLASH][1][0], false);

    cp_model.FixVariable(pawnWhichStartedOn_isPaired[FORWARD_SLASH][0][0], false);
    cp_model.FixVariable(pawnWhichStartedOn_isPaired[FORWARD_SLASH][1][BOARD_SIDE_LENGTH - 1],
                         false);

    int d               = 1; // FORWARD_SLASH
    int file_lim        = BOARD_SIDE_LENGTH;
    int other_direction = BACKSLASH;
    for (int i = -1; i <= 1; i += 2)
    {
        for (int j = d; j < file_lim; j++)
        {
            cp_model.AddEquality(pawnWhichStartedOn_isPaired[d][0][j],
                                 pawnWhichStartedOn_isPaired[d][1][j + i]);

            BoolVar notMatchedNorConsumedNorPairedInOtherDirection[]
                = { matchedStartingSquares[0][j].Not(),
                    matchedStartingSquares[1][j + i].Not(),
                    pawnConsumedPawnWhichStartedOn[0][j].Not(),
                    pawnConsumedPawnWhichStartedOn[1][j + i].Not(),
                    pawnWhichStartedOn_isPaired[other_direction][0][j].Not(),
                    pawnWhichStartedOn_isPaired[other_direction][1][j + i].Not() };
            cp_model.AddBoolAnd(notMatchedNorConsumedNorPairedInOtherDirection)
                .OnlyEnforceIf(pawnWhichStartedOn_isPaired[d][0][j]);
        }
        d               = 0; // BACKSLASH
        file_lim        = BOARD_SIDE_LENGTH - 1;
        other_direction = FORWARD_SLASH;
    }
    IntVar num_backslash_pairs = cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN);
    cp_model.AddEquality(num_backslash_pairs,
                         LinearExpr::Sum(pawnWhichStartedOn_isPaired[BACKSLASH][0]));
    IntVar num_forward_slash_pairs = cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN);
    cp_model.AddEquality(num_forward_slash_pairs,
                         LinearExpr::Sum(pawnWhichStartedOn_isPaired[FORWARD_SLASH][0]));
    IntVar num_pairs = cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN);
    cp_model.AddEquality(num_pairs, num_backslash_pairs + num_forward_slash_pairs);
    IntVar pairsGoTo[] = { cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN),
                           cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN) };
    cp_model.AddLessOrEqual(pairsGoTo[0], num_pairs);
    cp_model.AddEquality(pairsGoTo[1], num_pairs - pairsGoTo[0]);

    IntVar numZeroCostPromotions[] = { cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN),
                                       cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN) };
    for (int i = 0; i < NUM_SIDES; i++)
    {
        int opp = (i + 1) % NUM_SIDES;
        for (int j = 0; j < BOARD_SIDE_LENGTH; j++)
        {
            BoolVar zeroCostPromotionLiterals[] = {
                matchedStartingSquares[i][j].Not(),
                pawnConsumedPawnWhichStartedOn[i][j].Not(),
                pawnWhichStartedOn_isPaired[BACKSLASH][i][j].Not(),
                pawnWhichStartedOn_isPaired[FORWARD_SLASH][i][j].Not(),
                fileSamePawnFileMatches[opp][j].Not(),
                zeroCostPromotions[opp][j].Not(),
            };
            cp_model.AddBoolAnd(zeroCostPromotionLiterals).OnlyEnforceIf(zeroCostPromotions[i][j]);
        }
        cp_model.AddEquality(numZeroCostPromotions[i], LinearExpr::Sum(zeroCostPromotions[i]));
    }

    BoolVar promsViaCapturedPiece[NUM_SIDES][BOARD_SIDE_LENGTH];
    IntVar  numPromsViaCapturedPiece[]     = { cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN),
                                               cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN) };
    IntVar  numPromsViaCapturedBasePiece[] = { cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN),
                                               cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN) };
    IntVar  numPromsViaCapturedPromotion[] = { cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN),
                                               cp_model.NewIntVar(BOARD_SIDE_LENGTH_DOMAIN) };
    // Both sides capturing their opposition's promotions isn't optimal
    BoolVar atLeastOnePromCapturedByPawn[] = { cp_model.NewBoolVar(), cp_model.NewBoolVar() };
    for (int i = 0; i < NUM_SIDES; i++)
    {
        int opp = (i + 1) % NUM_SIDES;
        for (int j = 0; j < BOARD_SIDE_LENGTH; j++)
        {
            BoolVar promViaCapturedPieceLiterals[] = {
                matchedStartingSquares[i][j].Not(),
                pawnConsumedPawnWhichStartedOn[i][j].Not(),
                pawnWhichStartedOn_isPaired[BACKSLASH][i][j].Not(),
                pawnWhichStartedOn_isPaired[FORWARD_SLASH][i][j].Not(),
                zeroCostPromotions[i][j].Not(),
            };
            promsViaCapturedPiece[i][j] = cp_model.NewBoolVar();
            cp_model.AddBoolAnd(promViaCapturedPieceLiterals)
                .OnlyEnforceIf(promsViaCapturedPiece[i][j]);
        }
        cp_model.AddEquality(numPromsViaCapturedPiece[i],
                             LinearExpr::Sum(promsViaCapturedPiece[i]));

        cp_model.AddLessOrEqual(numPromsViaCapturedBasePiece[i], numCapturedBasePieces[opp]);
        // The source of the captured opp promotion can't be via captured pawn, otherwise we
        // could have promoted via that pair initially
        cp_model.AddLessOrEqual(numPromsViaCapturedPromotion[i],
                                numZeroCostPromotions[opp] + numPromsViaCapturedBasePiece[opp]);
        cp_model.AddEquality(numPromsViaCapturedPiece[i],
                             numPromsViaCapturedBasePiece[i] + numPromsViaCapturedPromotion[i]);

        cp_model.AddGreaterThan(numPromsViaCapturedPromotion[i], 0)
            .OnlyEnforceIf(atLeastOnePromCapturedByPawn[i]);
        cp_model.AddEquality(numPromsViaCapturedPromotion[i], 0)
            .OnlyEnforceIf(atLeastOnePromCapturedByPawn[i].Not());
    }
    cp_model.AddAtMostOne(atLeastOnePromCapturedByPawn);

    // Slack left for the cost of the opposition diagram, i.e. matching distances
    IntVar promsLeftForOppDiagram[NUM_SIDES];
    for (int i = 0; i < NUM_SIDES; i++)
    {
        int opp                   = (i + 1) % NUM_SIDES;
        promsLeftForOppDiagram[i] = cp_model.NewIntVar(PROMOTIONS_DOMAIN);
        cp_model.AddEquality(promsLeftForOppDiagram[i],
                             pairsGoTo[i] + numZeroCostPromotions[i] + numPromsViaCapturedPiece[i]
                                 - numPromsViaCapturedPromotion[opp] - minPromotions[i]);
        cp_model.AddGreaterOrEqual(promsLeftForOppDiagram[i], 0);
    }

    IntVar pawnDiagramPieceCaptureBudgets[]
        = { cp_model.NewIntVar(NUM_CAPTURABLE_CHESSMEN_PSIDE_DOMAIN),
            cp_model.NewIntVar(NUM_CAPTURABLE_CHESSMEN_PSIDE_DOMAIN) };
    for (int i = 0; i < NUM_SIDES; i++)
    {
        int opp = (i + 1) % NUM_SIDES;
        cp_model.AddEquality(pawnDiagramPieceCaptureBudgets[i],
                             (numCapturedBasePieces[opp] - numPromsViaCapturedBasePiece[i])
                                 + promsLeftForOppDiagram[opp]);

        cp_model.AddGreaterOrEqual(numPawnsConsumedByPawnsOnBoard[opp]
                                       + pawnDiagramPieceCaptureBudgets[i],
                                   pawnCostSum[i]);
    }

    Model model;
    // Useful for debugging
    model.Add(NewFeasibleSolutionObserver([&](const CpSolverResponse& r) {}));

    // Just one thread please: we already thread from main
    model.GetOrCreate<SatParameters>()->set_num_search_workers(1);
    const CpSolverResponse response = SolveCpModel(cp_model.Build(), &model);
    if (response.status() == CpSolverStatus::OPTIMAL
        || response.status() == CpSolverStatus::FEASIBLE)
    {
        return true;
    }
    else if (response.status() == CpSolverStatus::MODEL_INVALID)
    {
        cout << "Unexpected invalid model\n";
        exit(1);
    }
    return false;
} // namespace sat

} // namespace sat
} // namespace operations_research
