#!/bin/bash

count=0
sum=0

for param in "$@"
do
count=$(( $count + 1 ))
sum=$(( $sum + $param ))
done

echo "Sum is" $sum
echo "Avg is" $(($sum/$count))
