#!/usr/bin/python

""" 
This tool is meant to take piped input from an ls -R command

usage: ls -R [DIR] | python formatPath.py 1> selected.txt 2> discarded.txt
     : ls -R [DIR] | grep 'pattern' | python formatPath.py > pretiled.txt
     : ls -R [DIR] | python formatPath.py > selected.txt 2> /dev/null
"""

import os, sys

#file_selectors = ['.tif', '.png', '.gif', '.jpg']
file_selectors = ['.tif']

currentPath = None
if __name__ == '__main__':
    for line in sys.stdin:
        line = line.strip()
        
        valid_extension = reduce(
            lambda accum, ext: accum or line.endswith(ext),
            file_selectors, False)

        #directories in recursive ls end with colon:
        if line.endswith(':'): 
            currentPath = line[:len(line)-1]
        elif valid_extension:
            sys.stdout.write(os.path.join(currentPath, line) + "\n")
        else:
            sys.stderr.write('path: %s, line: %s\n' % (currentPath, line))
        
