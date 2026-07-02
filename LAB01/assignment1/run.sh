# ./run.sh
#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <input_file>"
    exit 1
fi
input_file="$1"

./reference/sampledecomment < "${input_file}" > "output_1.c" 2> "error_log_1"
./src/decomment < "${input_file}" > "output_2.c" 2> "error_log_2"
diff -c "output_1.c" "output_2.c"
diff -c "error_log_1" "error_log_2"
rm -f "output_1.c" "output_2.c" "error_log_1" "error_log_2"
