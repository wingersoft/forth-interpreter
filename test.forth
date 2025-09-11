cr ." --- Arithmetic ---" cr
5 3 + . cr
10 4 - . cr
6 7 * . cr
20 4 / . cr
17 5 mod . cr

cr ." --- Stack Manipulation ---" cr
5 dup .s cr drop
10 20 drop .s cr
10 20 swap .s cr drop drop
10 20 over .s cr drop drop drop
10 20 30 rot .s cr drop drop drop
10 20 nip .s cr drop
10 20 tuck .s cr drop drop drop

cr ." --- Comparison ---" cr
5 5 = . cr
5 3 > . cr
3 5 < . cr
3 5 <= . cr
5 3 >= . cr
3 5 <> . cr

cr ." --- Logical Operations ---" cr
1 0 and . cr
1 0 or . cr
1 not . cr

cr ." --- Literals and Stack ---" cr
42 . cr
5 10 15 .s cr drop drop drop
cr


." --- Colon Definitions ---" cr
: square dup * ;
5 square . cr

." --- Conditional ---" cr
: test-if 5 3 > if 1 else 0 then . cr ;
test-if

." --- Loops ---" cr
: countdown 3 begin dup . cr 1 - dup 0 = until drop ;
countdown

: countup 0 begin dup 3 < while dup . cr 1 + repeat drop ;
countup

: loop-test 3 0 do i . loop cr ;
loop-test

: nested-loop-test 2 0 do 2 0 do i . j . loop loop cr ;
nested-loop-test


." --- Memory ---" cr
CREATE myarray 10 cells allot
20 myarray !
myarray @ . cr

." --- Constants ---" cr
42 CONSTANT answer
answer . cr

." --- Strings ---" cr
." Hello, world!" cr

quit
