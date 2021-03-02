#!/bin/bash
autotest() {
    # rename for readability
    proc=$1
    max_p=$2
    iters=$3
    lock=$4
    folder="../data/${proc}_data"
    file="${proc}_${lock}.csv"
    echo $proc $lock
    echo "cnt_p,time" > $file       # clear file, add table header
    for cnt_p in $(seq 1 $max_p)    # run with different pthread number
    do
        echo running $cnt_p threads
        for epoch in $(seq 1 $iters)        # run several times
        do
            ./$proc $cnt_p $lock >> $file   # redir to the end of the file
        done
    done
    # print results
    # echo ===================
    # echo $file :
    # cat $file
    echo ===================
    mkdir $folder -p       # do not print error if dir already exist
    mv $file $folder/$file
}

# make source files
cd ../sources 
make

# make testers
cd ../testers
make

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../sources/

# tmp test
autotest hts 50 20 pthread_mutex
autotest hts 50 20 pthread_spinlock
autotest hts 50 20 spinlock
autotest hts 50 20 my_mutex

python plot.py


# counter tests with different locks
# autotest ct 50 20 pthread_mutex
# autotest ct 50 10 my_mutex
# autotest ct 30 40 pthread_spinlock
# autotest ct 30 40 spinlock

# list insert test
# autotest lti 30 5 pthread_mutex
# autotest lti 30 30 pthread_spinlock
# autotest lti 30 30 spinlock
# autotest lti 30 10 my_mutex

# list delete test: expect complexity O(1)
# autotest ltd 20 20 pthread_mutex
# autotest ltd 20 20 pthread_spinlock
# autotest ltd 20 20 spinlock
# autotest ltd 20 20 my_mutex

# list delete test: expect complexity O(n)
# autotest ltdon 20 20 pthread_mutex
# autotest ltdon 20 20 pthread_spinlock
# autotest ltdon 20 20 spinlock
# autotest ltdon 20 20 my_mutex

# list random test
# autotest ltr 50 10 pthread_mutex
# autotest ltr 50 40 pthread_spinlock
# autotest ltr 50 40 spinlock
# autotest ltr 50 40 my_mutex

# hash table insert test
# autotest hti 50 5 pthread_mutex
# autotest hti 50 30 pthread_spinlock
# autotest hti 50 30 spinlock
# autotest hti 50 5 my_mutex

# hash table random test
# autotest htr 30 5 pthread_mutex
# autotest htr 30 30 pthread_spinlock
# autotest htr 30 30 spinlock
# autotest htr 30 5 my_mutex

# hash table with full lock insert test
# autotest hflti 30 5 pthread_mutex
# autotest hflti 30 30 pthread_spinlock
# autotest hflti 30 30 spinlock
# autotest hflti 30 10 my_mutex

# hfl test random
# autotest hfltr 30 10 pthread_mutex
# autotest hfltr 30 30 pthread_spinlock
# autotest hfltr 30 30 spinlock
# autotest hfltr 30 30 my_mutex
