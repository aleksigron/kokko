import bpy
from array import array

def write(context, filepath, applyMods=True):
    try:
        outfile = open(filepath, "wb")

        # File type magic number
        outfile.write(b'\x10\x10\x19\x91');

        vertexPos = array('f')

        for v in bpy.context.active_object.data.vertices:
            vertexPos.extend([v.co.x, v.co.y, v.co.z])

        vertexPositions.tofile(outfile)

    except Exception as err:
        return "An error occurred:\n{}".format(err)
        
    finally:
        outfile.close()

    return "Success"
