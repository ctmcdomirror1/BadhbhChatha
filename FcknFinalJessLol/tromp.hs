-- Original version written by John Tromp and available at https://tromp.github.io/chess/chess.html
-- Modified by myself. It looks awkward because I've tried to stick to the original means of computation,
-- but I need to modify it to match my sample space size

import           Control.Monad
import           Control.Parallel
import           Data.Array
import           Debug.Trace

debug = flip trace

armies np ir = do
  q <- [0..1+np]
  let prom1 =         max (q-1) 0
  r <- [0..ir+np-prom1]
  let prom2 = prom1 + max (r-ir) 0
  b <- [0..2+np-prom2]
  let prom3 = prom2 + max (b-2) 0
  n <- [0..2+np-prom3]
  let proms = prom3 + max (n-2) 0
  guard $ proms <= np
  p <- [0..np-proms]
  return (q+r+b+n, p, proms, fac q*fac r*fac b*fac n*fac p)

fac :: Int -> Integer
fac n = fac64!n where fac64 = listArray (0,64) (scanl (*) 1 [1..64])

count fixwp fixbp addnfixwp pspace fixwr fixbr = sum $ do
  let wp_capacity = 8-fixwp-addnfixwp
  let bp_capacity = 8-fixbp
  (wpcs,wp,wproms,wprod) <- armies wp_capacity (2-fixwr)
  let wpx = wp_capacity-wp-wproms                 -- white p captured
  (bpcs,bp,bproms,bprod) <- armies bp_capacity (2-fixbr)
  let bpx = bp_capacity-bp-bproms                 -- black p captured
  let caps = 30-fixwp-fixbp-addnfixwp-fixwr-fixbr-wp-bp-wpcs-bpcs

  guard $ wproms <= bpx + caps
  guard $ bproms <= wpx + caps

  let space = 62-3*fixwp-fixbp-addnfixwp-fixwr-fixbr-wp-bp
  return $ (fac pspace `div` fac (pspace-wp-bp)) * (fac space `div` fac (space-wpcs-bpcs)) `div` (wprod * bprod)

-- Take care of multiplying non-edge kings in countep* functions.
-- In order not to count duplicate enpassant scenarios, we consider all adjacent square possibilities
--
-- 1st factor is file / adjacent pawn
-- 2nd factor is king0
-- 3rd factor is king1
--
-- No kings to place, so 1st factor is always 1
countep_2ek fwr fbr = do
  let no_enpassant =             (1)     * ((1)  * (1)  * (count 0 0 0 48 fwr fbr))
  let enpassant_right_and_edge = (7 + 1) * ((1)  * (1)  * (count 1 1 0 44 fwr fbr))
  let enpassant_left_less_edge = (8 - 2) * ((1)  * (1)  * (count 1 1 0 43 fwr fbr + count 1 1 1 43 fwr fbr))
  no_enpassant + enpassant_right_and_edge + enpassant_left_less_edge

-- One king to place.
countep_1ek fwr fbr = do
  let count_1_1_0_43_fwr_fbr = count 1 1 0 43 fwr fbr
  let no_enpassant =             (1)     * ((48) * (1)  * (count 0 0 0 47 fwr fbr))
  let enpassant_right_and_edge = (7 + 1) * ((44) * (1)  * (count_1_1_0_43_fwr_fbr))
  let enpassant_left_less_edge = (8 - 2) * ((43) * (1)  * (count 1 1 0 42 fwr fbr + count 1 1 1 42 fwr fbr) + (1)      * (1) * count_1_1_0_43_fwr_fbr)
  no_enpassant + enpassant_right_and_edge + enpassant_left_less_edge

countep_0ek fwr fbr = do
  let count_1_1_0_42_fwr_fbr = count 1 1 0 42 fwr fbr
  let no_enpassant =             (1)     * ((48) * (47) * (count 0 0 0 46 fwr fbr))
  let enpassant_right_and_edge = (7 + 1) * ((44) * (43) * (count_1_1_0_42_fwr_fbr))
  let enpassant_left_less_edge = (8 - 2) * ((43) * (42) * (count 1 1 0 41 fwr fbr + count 1 1 1 41 fwr fbr) + (2 * 43) * (1) * count_1_1_0_42_fwr_fbr)
  no_enpassant + enpassant_right_and_edge + enpassant_left_less_edge

-- Take care of multiplying edge kings here
main = do
   c0 `par` c1 `par` c2 `par` c3 `par` c4 `par` c5 `par` c6 `par` c7 `par` c8 `par` c9 `pseq` print (2 * (c9 + c8 + c7 + c6 + c5 + c4 + c3 + c2 + c1 + c0))
   -- 1st factor is for symmetry
   -- 2nd factor is a side0 factor
   -- 3rd factor is a side1 factor
   where c0 = (1) * (16)     * (15) * (countep_2ek 0 0)
         c1 = (1) * (16 - 2) * (2)  * (countep_2ek 0 1 + countep_2ek 1 0) -- (16 - king - fixed rook) == (16 - 2)
         c2 = (1) * (16 - 3) * (1)  * (countep_2ek 0 2 + countep_2ek 2 0) -- (16 - king - 2 fixed rooks) == (16 - 3)
         c3 = (1) * (2)      * (2)  * (countep_2ek 1 1)
         c4 = (1) * (2)      * (1)  * (countep_2ek 1 2 + countep_2ek 2 1)
         c5 = (1) * (1)      * (1)  * (countep_2ek 2 2)

         c6 = (2) * (16)     * (1)  * (countep_1ek 0 0)
         c7 = (1) * (2)      * (1)  * (countep_1ek 0 1 + countep_1ek 1 0)
         c8 = (1) * (1)      * (1)  * (countep_1ek 0 2 + countep_1ek 2 0)

         c9 = (1) * (1)      * (1)  * (countep_0ek 0 0)
