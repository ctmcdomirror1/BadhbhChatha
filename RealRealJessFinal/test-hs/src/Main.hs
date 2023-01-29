import Control.Monad
import System(getArgs)
import Array
 
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
 
count fixwp fixbp pspace fixwr fixbr = sum $ do
  let nwp = 8-fixwp
  let nbp = 8-fixbp
  (wpcs,wp,wproms,wprod) <- armies nwp (2-fixwr)
  let wpx = np-wp-wproms                 -- white p captured
  (bpcs,bp,bproms,bprod) <- armies nbp (2-fixbr)
  let bpx = np-bp-bproms                 -- black p captured
  let caps = 30-2*fixp-fixwr-fixbr-wp-bp-wpcs-bpcs
  guard $ wproms <= bpx + caps
  guard $ bproms <= wpx + caps
  let space = 62-4*fixp-fixwr-fixbr-wp-bp
  return $ (fac pspace `div` fac (pspace-wp-bp)) *
           (fac space `div` fac (space-wpcs-bpcs)) `div` (wprod * bprod)
 
count0 (mul,ek) = mul * count 0 (46+ek) 0 0
 
countep (mul,ek,fwr,fbr) = mul *
  (count 0 0 (46+ek) fwr fbr + (8*2-2) * count 1 1 (42+ek) fwr fbr - (8 - 6) * count 2 1 (41+ek) fwr wbr)
 
main = print . (* 2) . sum $ map countep
  [(1,2,2,2),(2*2,2,1,2),(2*2,2,1,1),
   (2*13,2,0,2),(2*48,1,0,2),(2*2*14,2,0,1),(4*48,1,0,1),
   (16*15,2,0,0),(2*16*48,1,0,0),(64*63,0,0,0)]
