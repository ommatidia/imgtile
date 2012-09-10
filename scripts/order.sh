#!/bin/sh

#Simple script to create an ordering from the output metadata. It is probably recommended that you come up with your own ordering, but since one is required this is a quick solution until you can put together something better, more suitable to your application

meta=$1
output_file="ordering.json"

echo var ordering = [ > $output_file && cat $meta | grep ': \[$' | grep -o '".*"' | paste -d ',' -s >> $output_file && echo ]\; >> $output_file
