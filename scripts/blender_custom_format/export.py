import bpy
from array import array
from struct import pack

def write(context, filepath, applyMods=True):
    
    vertices = context.active_object.data.vertices
    vertexCount = len(vertices)
    
    # '=' for native byte-order, standard packing
    header = pack('=h', vertexCount)
    
    # 'f' for 4-byte floating point
    vertexData = array('f')

    for v in vertices:
        vertexData.extend([v.co.x, v.co.y, v.co.z])

    with open(filepath, 'wb') as outfile:

        # Start with file type magic number
        outfile.write(b'\x10\x10\x19\x91');
        
        # Write header data
        outfile.write(header)
        
        # Write vertex data
        vertexData.tofile(outfile)
