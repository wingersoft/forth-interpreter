cr ." === String Tests ===" cr

cr ." Test 1: Basic S\" and TYPE" cr
S" Hello, World!" TYPE cr

cr ." Test 2: Empty string" cr
S" " TYPE ." (empty)" cr

cr ." Test 3: S\" in compiled word" cr
: greet S" Hello from Forth!" TYPE cr ;
greet

cr ." Test 4: Multiple strings" cr
: multi S" First " TYPE S" Second " TYPE S" Third" TYPE cr ;
multi

cr ." Test 5: String in conditional" cr
: cond 1 if S" TRUE path" else S" FALSE path" then TYPE cr ;
cond

cr ." Test 6: String in loop" cr
: looptest 3 0 do S" Loop! " TYPE loop cr ;
looptest

cr ." Test 7: Long string" cr
S" This is a longer string to test buffer capacity." TYPE cr

cr ." Test 8: Stack check" cr
.s cr
S" Test" 2dup TYPE cr
." Address: " . cr
." Length: " . cr
.s cr

cr ." Test 9: COUNT operation" cr
CREATE cstr 5 c, 72 c, 101 c, 108 c, 108 c, 111 c,
cstr COUNT TYPE cr

cr ." === All tests done ===" cr
quit