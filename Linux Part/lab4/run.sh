make
# init datafile
cp datafile-frag.txt datafile
# run with memory leak check
valgrind -s --leak-check=full --show-reachable=yes ./defrag datafile

# normal run
# ./defrag datafile
