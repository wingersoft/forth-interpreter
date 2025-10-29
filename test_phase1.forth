cr ." === Phase 1 Function Tests ===" cr cr

cr ." --- Stack Depth Tests ---" cr
5 10 15 depth . cr
drop drop drop

cr ." --- ?dup Tests ---" cr
0 ?dup depth . cr drop
5 ?dup depth . cr drop drop

cr ." --- 2dup Tests ---" cr
10 20 2dup .s cr
drop drop drop drop

cr ." --- 2drop Tests ---" cr
1 2 3 4 2drop .s cr
drop drop

cr ." --- 2swap Tests ---" cr
1 2 3 4 2swap .s cr
drop drop drop drop

cr ." --- Arithmetic Shortcuts ---" cr
-5 abs . cr
5 abs . cr
10 negate . cr
-7 negate . cr

5 10 min . cr
5 10 max . cr
-3 2 min . cr
-3 2 max . cr

5 1+ . cr
5 1- . cr
10 2* . cr
10 2/ . cr
11 2/ . cr

17 5 /mod . . cr
-17 5 /mod . . cr

cr ." --- Logical Operations ---" cr
12 10 xor . cr
1 2 lshift . cr
8 2 rshift . cr

cr ." --- Character I/O ---" cr
65 emit cr
space ." <-space" cr
." |" 5 spaces ." |" cr

cr ." --- Utility Tests ---" cr
here . cr
true . cr
false . cr

cr ." --- Zero Comparison Tests ---" cr
0 0= . cr
5 0= . cr
-5 0< . cr
5 0< . cr
5 0> . cr
-5 0> . cr
0 0> . cr

cr ." --- Complex Examples ---" cr
: abs-diff - abs ;
10 3 abs-diff . cr
3 10 abs-diff . cr

: test-depth depth 3 = if ." PASS" else ." FAIL" then ;
1 2 3 test-depth cr drop drop drop

cr ." === All Phase 1 Tests Complete ===" cr

