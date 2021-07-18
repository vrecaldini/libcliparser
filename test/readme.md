# Test 

The `test.cpp` source code contains a small test of `libcliparser`.

After compiling it as `test` or `test.exe`, its output may show some insight. If `test.cpp` has been compiled without defining `NDEBUG`, some tests will be tried and their output will be shown on the console.

This program requires the options `-f`, `-n`, `-d`, and `-b`, and it defines the optional options `--flag` and `-q`.

A suitable command line invokation of `test` should look like this

    ./test -f "hello.txt" -n 32 -d 44 -b true

