import sys, os
import tempfile

# install standard driver
import Image, TiffImagePlugin 

LZW      = "lzw:2"
ZIP      = "zip"
JPEG     = "jpeg"
PACKBITS = "packbits"
G3       = "g3"
G4       = "g4"

def _save(im, fp, filename):
    
    # check compression mode
    try:
        compression = im.encoderinfo["compression"]
    except KeyError:
        # use standard driver
        TiffImagePlugin._save(im, fp, filename)
    else:
        # compress via temporary file
        if compression not in (LZW, ZIP, JPEG, PACKBITS, G3, G4):
            raise IOError, "unknown compression mode"
        file = tempfile.mktemp()
        im.save(file, "TIFF")
        os.system("tiffcp -c %s %s %s" % (compression, file, filename))
        try: os.unlink(file)
        except: pass

Image.register_save(TiffImagePlugin.TiffImageFile.format, _save)

#TODO: support other filetypes (filename, type)
def compress(filename):
    #TODO: actual exceptions error handling
    
    try:
        im = Image.open(filename)
    except:
        raise "Could not open file."

    index = filename.rfind('/')
    if index < 0:
        path = './'
        name = filename
    else:
        path = filename[:index]
        name = filename[index+1:]

    temp = os.path.join(path, "copy_%s" % name)
    
    try:
        im.save(temp, "TIFF",  compression=LZW)
    except:
        raise "Could not save compressed file"

    os.unlink(filename)
    os.rename(temp, filename)

if __name__ == '__main__':
    compress(sys.argv[1])
    
    
