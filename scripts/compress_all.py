#!/usr/bin/python

import os, sys, argparse, lzw2_comp

def getImagePathList(input_filename):
    try:
        f = open(input_filename, 'r')
        images = f.readlines()
        images = map(lambda x: x.strip(' \n'), images)
        return images
    except IOError, e:
        sys.stderr.write(e)
        sys.stderr.write("\n")
    return []
    
def getArgParser():
    title = "Compress list of (tif for now) files"
    parser = argparse.ArgumentParser(title)
    parser.add_argument('--version', action='version', version='%(prog)s 0.1a')

    parser.add_argument('images', nargs='*', default=None, \
                            help='Path to one or more images to tile.')
    parser.add_argument('-f', '--filename', \
                            help='Path to file containing newline separated paths of images.')
    return parser

def init():
    parser = getArgParser()
    args = parser.parse_args()

    mapPath = lambda x : os.path.abspath(x)
    if args.filename is not None:
        img_from_file = getImagePathList(args.filename)
        img_from_file = map(mapPath, img_from_file)
    else:
        img_from_file = []

    if args.images is not None:
        img_from_cli = map(mapPath, args.images)
    else:
        img_from_cli = []

    images = set(img_from_cli) | set(img_from_file)
    
    if len(images) == 0:
        print "Filename and/or explicit files must be specified\n"
        parser.print_help()
        return

    i = 1
    l = len(images)
    for image in images:
        print "[%s of %s] Compressing: %s\n" % (i, l, image)
        #TODO: add exceptions to compress, try/except here
        try:
            lzw2_comp.compress(image)
            sys.stdout.write(image + "\n")
            #to exclude from rsync updates: cat list.txt | grep -v "^[" > success.txt
        except:
            sys.stderr.write(image + "\n")
        i = i + 1

    
if __name__ == '__main__':
    init()


