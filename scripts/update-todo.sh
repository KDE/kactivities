#!/bin/bash

grep -n -I -r TODO src | sed 's/^\([^:]*:[^:]*\):[- \/#]*\(.*\)/\2\t(\1)/' | sed 's/^TODO: //' | sed 's/(TODO)//' | sed 's/BLOCKER/!!! BLOCKER !!!/' | sort | tee TODO
