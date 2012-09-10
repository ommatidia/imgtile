#!/usr/bin/python

""" 
This tool is meant to take piped input from an ls -R command

usage: ls -R [DIR] | python formatPath.py 1> selected.txt 2> discarded.txt
     : ls -R [DIR] | grep 'pattern' | python formatPath.py > pretiled.txt
     : ls -R [DIR] | python formatPath.py > selected.txt 2> /dev/null
"""

import os, sys, argparse

def get_parser():
    title = "Utility for listing paths of all files recursively in a directory that match by file type"
    parser = argparse.ArgumentParser(title)
    
    #TODO: better help docs

    parser.add_argument(dest='ext', nargs='*', default=['.tif'], \
                            help="File extensions, default is '.tif'") 

    return parser


#TODO: error handling for invalid inputs


if __name__ == '__main__':
    parser = get_parser()
    args = parser.parse_args()

    file_selectors = args.ext
    currentPath = None
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
        
