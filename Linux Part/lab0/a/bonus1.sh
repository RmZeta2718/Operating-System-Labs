#!/bin/bash
awk 'BEGIN {FS = "\n"; a=1} {if (a == 10) print $0; a += 1}'