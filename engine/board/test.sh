#!/usr/bin/env bash

for f in ./tests/*; do
	echo "Running test: $f"
	cat $f | ./test || exit 1
	echo "PASS"
done
exit 0
