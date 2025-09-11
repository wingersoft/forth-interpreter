." --- Loops ---" cr
: countdown 3 begin dup . cr 1 - dup 0 = until drop ;
countdown           ." 3 2 1"

: countup 0 begin dup 3 < while dup . cr 1 + repeat drop ;
countup             ." 0 1 2"

: loop-test 3 0 do i . loop cr ;
loop-test           ." 0 1 2"
