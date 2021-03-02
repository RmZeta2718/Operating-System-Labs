#!/bin/bash
find /bin/ -name "b*" -type f | sort | xargs ls -l | awk '{sub(/\/bin\//, "", $9); print $9, $3, $1}' > output
chmod o=r output
