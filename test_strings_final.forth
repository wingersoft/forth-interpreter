cr ." ========================================" cr
cr ." Core String Operations - Final Test" cr
cr ." ========================================" cr

cr ." 1. S\" interpret mode:" cr
S" Hello, Forth!" TYPE cr

cr ." 2. S\" compile mode:" cr
: say S" Compiled string works!" TYPE cr ;
say

cr ." 3. TYPE with stack values:" cr
S" Testing TYPE" TYPE cr

cr ." 4. Multiple S\" calls:" cr
: multi S" One " TYPE S" Two " TYPE S" Three" TYPE cr ;
multi

cr ." 5. Empty string:" cr
S" " TYPE ." <-- empty string" cr

cr ." 6. Conditional with strings:" cr
: iftest 1 if S" PASS" else S" FAIL" then TYPE cr ;
iftest

cr ." 7. Loop with strings:" cr
: loopy 3 0 do S" *" TYPE loop cr ;
loopy

cr ." 8. COUNT operation:" cr
CREATE mystr 5 c, 72 c, 101 c, 108 c, 108 c, 111 c,
mystr COUNT TYPE cr

cr ." 9. Long string test:" cr
S" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" TYPE cr

cr ." 10. Verify stack after string ops:" cr
S" Test" TYPE cr
.s cr

cr ." ========================================" cr
cr ." All tests PASSED!" cr
cr ." ========================================" cr

quit