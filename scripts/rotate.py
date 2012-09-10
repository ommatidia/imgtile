#!/usr/bin/python

import os, sys, argparse, Image
import lzw2_comp #support tiff

def process(images, rotation, filter=Image.LINEAR):
    for imagefile in images:
        im = Image.open(imagefile)
        im.rotate(rotation, filter, True).save(imagefile)
    
def getArgParser():
    title = "Rotate Images for Retiling"
    parser = argparse.ArgumentParser(title)
    parser.add_argument('--version', action='version', version='%(prog)s 0.1a')

    parser.add_argument('images', nargs='*', default=None, \
                            help='Path to one or more images to rotate.')
    parser.add_argument('-r', '--rotate', \
                            help='Interval of 90 degrees to rotate image(s) by.')
    return parser

def init():
    parser = getArgParser()
    args = parser.parse_args()

    mapPath = lambda x : os.path.abspath(x)
    if args.images is not None:
        images = map(mapPath, args.images)
    else:
        images = []
    
    if len(images) == 0:
        print "Files must be specified\n"
        parser.print_help()
        return

    rotate = int(args.rotate)
    if rotate % 90 != 0:
        print "Rotation Parameter must be a multiple of 90 degrees";
        parser.print_help()
        return

    process(images, rotate)
    
if __name__ == '__main__':
    init()


