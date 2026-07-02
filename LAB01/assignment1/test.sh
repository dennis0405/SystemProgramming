# ./test.sh
#!/bin/bash

gcc800 -E ./src/decomment.c > ./src/decomment.i
gcc800 -S ./src/decomment.i -o ./src/decomment.s
gcc800 -c ./src/decomment.s -o ./src/decomment.o
gcc800 ./src/decomment.s -lc -o ./src/decomment

test_files=("test0.c" "test1.c" "test2.c" "test3.c" "test4.c" "test5.c" "../src/decomment.c")

for file in "${test_files[@]}"; do
    echo "============= $file ============="
    bash run.sh ./test_files/$file
done
