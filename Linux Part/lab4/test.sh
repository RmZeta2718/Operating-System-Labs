make disk_dump
# ./defrag datafile
# ./disk_dump datafile
# ./disk_dump datafile-defrag > mydflog
# ./disk_dump godweiyang/datafile-defrag > wydflog
# cmp -b mydflog wydflog
./disk_dump datafile > dflog
./disk_dump datafile-defrag > dfdlog
cmp -b dflog dfdlog
