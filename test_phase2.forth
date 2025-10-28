cr ." === Phase 2 Function Tests ===" cr cr

cr ." --- Return Stack Tests ---" cr
5 >r 10 r@ . r> . cr
." Expected: 5 5 (r@ copies, r> retrieves)" cr

: test-return-stack 1 2 3 >r >r + r> + r> + ;
test-return-stack . cr
." Expected: 6 (1+2+3)" cr

cr ." --- Memory Enhancement Tests ---" cr
VARIABLE counter
0 counter !
1 counter +! counter @ . cr
5 counter +! counter @ . cr
." Expected: 1 6 (incremental adds)" cr

here . ." (current memory address)" cr
42 , here . ." (after comma)" cr
here 1- @ . cr
." Expected: 42 (value stored by comma)" cr

VARIABLE bytes
65 bytes c! bytes c@ . cr
." Expected: 65 (byte storage)" cr

cr ." --- Number Base Tests ---" cr
decimal 255 . cr
hex 255 . cr
decimal 255 . cr
." Expected: 255 ff 255 (same value, different bases)" cr

decimal 5 . cr
binary 5 . cr
decimal 5 . cr
." Expected: 5 101 5 (same value, different bases)" cr

cr ." --- Output Formatting Tests ---" cr
decimal
-5 u. cr
." Expected: large number (unsigned -5)" cr

42 10 .r cr
." Expected: right-justified 42 in 10 chars" cr

255 .h cr
." Expected: ff (hex output)" cr

cr ." --- Complex Example: Temporary Storage ---" cr
: compute-average 2dup + 2 / >r 2drop r> ;
10 20 compute-average . cr
." Expected: 15 (average of 10 and 20)" cr

: save-and-process >r dup * r> + ;
5 10 save-and-process . cr
." Expected: 35 (5*5 + 10)" cr

cr ." --- Memory Array Test ---" cr
CREATE numbers 5 cells allot
10 numbers !
20 numbers 1 cells + !
30 numbers 2 cells + !
numbers @ . cr
numbers 1 cells + @ . cr
numbers 2 cells + @ . cr
." Expected: 10 20 30" cr

cr ." --- Return Stack Complex Example ---" cr
: triple-temp >r >r >r r> r> r> ;
1 2 3 triple-temp . . . cr
." Expected: 1 2 3 (preserved through return stack)" cr

cr ." === All Phase 2 Tests Complete ===" cr
quit