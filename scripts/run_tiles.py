#!/usr/bin/python

import os, sys, argparse, hashlib, simple_json

import shutil, tempfile, couchdbkit
from restkit import BasicAuth

"""
Handles processing all images, potentially including by layers (group in tracking dict by deepest common folder)
"""
class AbstractTiler:

    def __init__(self, src_images, layer_folders=False):
        self.src_images = src_images
        self.layer_folders = layer_folders
        
    def process(self):
        tracking_dict = {}
        for imagefile in self.src_images:
            print imagefile + "\n"
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

            self.handleImage(meta)

        self.handleTrackingDict(tracking_dict)
            
    def handleImage(self, meta):
        raise Exception("Not Yet Implemented")
    def handleTrackingDict(self, dictionary):
        raise Exception("Not Yet Implemented")

"""
#actually need to figure out the schema we desire!!
class CloudantTiler(AbstractTiler):

    class Meta(couchdbkit.Document):
        _id = couchdbkit.StringProperty()
        imagepath = couchdbkit.StringProperty()
        imagename = couchdbkit.StringProperty()

    def __init__(self, images, args):
        AbstractTiler.__init__.__call__(self, images, args.layer)
        self.dest = args.dest #todo: validate server url
        self.username = args.username
        self.password = args.password
                
        auth = BasicAuth(self.username, self.password)
        self.server = Server(self.dest, filters = [ auth ])
        self.db = self.server.get_or_create_db(args.database)

        Meta.set_db(self.db)

    def handleImage(self, meta):
        document = Meta(_id = meta['hash'], imagepath=meta['imagepath'], imagename=meta['imagename'])
        if 'layer' in meta:
            document.layer = meta['layer']
        if 'index' in meta:
            document.index = meta['index']
        
        output_directory = tempfile.mkdtemp()
        sys_command = "./imgtile %s %s" % ( meta['imagepath'], output_directory)
        
        os.system(sys_command)
        
        #post to db
        
        #shutil.rmtree(output_directory)
        
    def handleTrackingDict(self, dictionary):
        #build json doc
        #post to db
        pass
"""

class FileTiler(AbstractTiler):
    def __init__(self, images, args):
        AbstractTiler.__init__.__call__(self, images, args.layer)
        self.dest = args.dest
        
    def handleImage(self, meta):
        output_directory = self.dest
        if 'layer' in meta:
            output_directory = os.path.join(output_directory, meta['layer'])
        output_directory = os.path.join(output_directory, meta['hash'])

        sys_command = "./imgtile %s %s" % (meta['imagepath'], output_directory)
        print sys_command
        #os.system(sys_command)
                                            
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

    parser.add_argument('images', nargs='*', default=None)
    parser.add_argument('-f', '--filename', help='file containing newline separated paths of images if not explicitly specified.')
    parser.add_argument('-d', '--dest', default="./tiles/", help="root destination to output tiles.")
    parser.add_argument('-t', '--type', default="local", help="local | cloudant | openstack")
    parser.add_argument('-l', '--layer', action="store_const", default=False, const=True)
    parser.add_argument('-u', '--username', default=None)
    parser.add_argument('-p', '--password', default=None)
    parser.add_argument('-b', '--database', default="test")
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
        print "Filename or explicit files must be specified\n"
        parser.print_help()
        return

    ttype = getTiler(args.type)
    tiler = ttype(images, args)
    tiler.process()
    
if __name__ == '__main__':
    init()


