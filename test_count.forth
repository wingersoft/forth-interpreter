cr ." === COUNT Test ===" cr

cr ." Creating counted string manually:" cr
CREATE cstr
5 c,
72 c,
101 c,
108 c,
108 c,
111 c,

cr ." Checking what we created:" cr
cstr @ . cr
cstr 1 + c@ . cr
cstr 2 + c@ . cr

cr ." Using COUNT:" cr
cstr COUNT
." Length: " . cr
." Address: " . cr

cr ." Using COUNT and TYPE together:" cr
cstr COUNT TYPE cr

cr ." === Done ===" cr
quit