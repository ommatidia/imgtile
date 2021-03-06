#!/usr/bin/python

import os, sys, argparse, hashlib, simple_json

import shutil, tempfile, couchdbkit
from restkit import BasicAuth

from pprint import pprint

"""
Handles processing all images, potentially including by layers (group in tracking dict by deepest common folder)
"""
class AbstractTiler:

    def __init__(self, src_images, layer_folders=False, dict_opt=None):
        self.src_images = src_images
        self.layer_folders = layer_folders
        self.dict = dict_opt
        
    def process(self):
        tracking_dict = {}
        i = 0
        l = len(self.src_images)
        for imagefile in self.src_images:
            i = i+1

            sys.stdout.write("[%s:%s] " % (i, l));
            sys.stdout.flush();

            filehash = hashlib.md5(imagefile).hexdigest()
            index = imagefile.rfind('/')            
            imagename = imagefile[index+1:]
            meta = {
                "hash" : filehash,
                "imagename": imagename,
                "imagepath": imagefile
            }
            if self.layer_folders:
                #this will break with root storage, but why would you do that
                base = imagefile[:index]
                layers_group = base[base.rfind('/')+1:]
                meta["layer"] = layers_group
                meta["index"] = 0 if layers_group not in tracking_dict else len(tracking_dict[layers_group])
                if layers_group in tracking_dict:
                    tracking_dict[layers_group].append(meta)
                else:
                    tracking_dict[layers_group] = [ meta ]
            else:
                if filehash in tracking_dict:
                    tracking_dict.append(meta)
                else:
                    tracking_dict = [ meta ]

            if self.dict != True:
                self.handleImage(meta)

        if self.dict != False:
            #TODO: ensure directory creation
            self.handleTrackingDict(tracking_dict)
            
    def handleImage(self, meta):
        raise Exception("Not Yet Implemented")
    def handleTrackingDict(self, dictionary):
        raise Exception("Not Yet Implemented")

class FileTiler(AbstractTiler):
    def __init__(self, images, args):
        AbstractTiler.__init__.__call__(self, images, args.layer, args.dict)
        self.dest = args.dest
        
    def handleImage(self, meta):
        output_directory = self.dest
        output_directory = os.path.join(output_directory, meta['hash'])

        sys_command = "../imgtile %s %s" % (meta['imagepath'], output_directory)
        sys.stdout.write(sys_command + "\n");
        sys.stdout.flush();
        os.system(sys_command)
                                            
    def handleTrackingDict(self, dictionary):
        f = open(os.path.join(self.dest, 'metadata.json'), 'w')
        metadata = simple_json.dumps(dictionary, indent='    ', sort_keys = True)
        f.write("var metadata = ")
        f.write(metadata)
        f.write(';')
        f.flush()
        f.close()


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
    title = "Segment and Store Tiles from a Set of Images"
    parser = argparse.ArgumentParser(title)
    parser.add_argument('--version', action='version', version='%(prog)s 0.1a')

    parser.add_argument('images', nargs='*', default=None, \
                            help='Path to one or more images to tile.')
    parser.add_argument('-f', '--filename', \
                            help='Path to file containing newline separated paths of images.')
    parser.add_argument('-d', '--dest', default="./tiles/", \
                            help='root destination to output tiles.')
    parser.add_argument('-l', '--layer', action="store_const", default=False, const=True, \
                            help='Group images as layers, by immediate containing directory.')

    #TODO: more informative help
    parser.add_argument('-t', '--type', default="local", \
                            help='local | openstack')

    #TODO: akamai uploader
    parser.add_argument('-u', '--username', default=None)
    parser.add_argument('-p', '--password', default=None)
    #parser.add_argument('-b', '--database', default="test")
    
    
    #specifying neither will do both
    parser.add_argument('--no-dict', dest='dict', action='store_const', const=False, default=None, \
                            help='Tile images without outputting any metadata')
    parser.add_argument('--dict', dest='dict', action='store_const', const=True, default=None, \
                            help='Calculate and output metadata without processing images')
    
    return parser

def getTiler(ttype="local"):
    if ttype == "local":
        return FileTiler
    elif ttype == "cloudant":
        return CloudantTiler
    elif ttype == "akamai":
        return AkamaiTiler

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

    ttype = getTiler(args.type)
    tiler = ttype(images, args)
    tiler.process()
    
if __name__ == '__main__':
    init()


