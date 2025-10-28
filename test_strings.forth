\ String Handling Test Suite
\ Tests for TYPE, COUNT, and S" operations

cr ." ==================================" cr
cr ." String Handling Test Suite" cr
cr ." ==================================" cr

\ Test 1: S" in interpret mode with TYPE
cr ." --- Test 1: S\" in interpret mode with TYPE ---" cr
S" Hello, World!" TYPE cr
S" Forth strings!" TYPE cr
S" " TYPE ." (empty string test)" cr

\ Test 2: S" in compile mode (user-defined word)
cr ." --- Test 2: S\" in user-defined word ---" cr
: greet S" Hello from Forth!" TYPE cr ;
greet

: multistring S" First " TYPE S" Second " TYPE S" Third" TYPE cr ;
multistring

\ Test 3: Empty strings
cr ." --- Test 3: Empty string handling ---" cr
: testempty S" " TYPE ." (empty)" cr ;
testempty

\ Test 4: COUNT with manually created counted string
cr ." --- Test 4: COUNT operation ---" cr
\ Create a counted string: length byte followed by characters
\ Using c! to build counted string manually
CREATE cstring
5 c,          \ Length = 5
72 c,         \ 'H'
101 c,        \ 'e'
108 c,        \ 'l'
108 c,        \ 'l'
111 c,        \ 'o'

." Counted string test: " cstring COUNT TYPE cr

\ Test 5: Using S" result for multiple operations
cr ." --- Test 5: S\" result manipulation ---" cr
: showlength S" Testing" dup . cr ." Length is: " . cr TYPE ;
showlength

\ Test 6: Nested string usage
cr ." --- Test 6: Nested strings in control flow ---" cr
: conditional 1 if S" TRUE branch" else S" FALSE branch" then TYPE cr ;
conditional

0 if S" Not shown" TYPE cr else S" ELSE works" TYPE cr then

\ Test 7: S" with loops
cr ." --- Test 7: Strings in loops ---" cr
: repeat3 3 0 do S" Loop! " TYPE loop cr ;
repeat3

\ Test 8: Long strings (test buffer capacity)
cr ." --- Test 8: Longer strings ---" cr
S" This is a longer string to test the transient buffer capacity and wrapping behavior." TYPE cr

\ Test 9: Multiple S" in same line (interpret mode)
cr ." --- Test 9: Multiple S\" in interpret ---" cr
S" First, " TYPE S" second, " TYPE S" third!" TYPE cr

\ Test 10: String and arithmetic combined
cr ." --- Test 10: Mixed operations ---" cr
: compute 5 3 + S" The result is: " TYPE . cr ;
compute

\ Test 11: Verify stack manipulation with strings
cr ." --- Test 11: Stack effects ---" cr
.s cr
S" Test" .s cr    \ Should show address and length on stack
TYPE cr
.s cr             \ Stack should be empty again

cr ." ==================================" cr
cr ." All string tests completed!" cr
cr ." ==================================" cr

quit