import bpy
import time
import math
from array import array
from struct import pack

class Bounds:
    def __init__(self):
        self.min = [0.0, 0.0, 0.0]
        self.max = [0.0, 0.0, 0.0]
        self.center = [0.0, 0.0, 0.0]
        self.extents = [0.0, 0.0, 0.0]
        self.size = [0.0, 0.0, 0.0]

def get_bounding_box(verts):
    bounds = Bounds()

    if len(verts) > 0:
        min_x = 1e9
        max_x = -1e9
        min_y = 1e9
        max_y = -1e9
        min_z = 1e9
        max_z = -1e9
        
        for v in verts:
            if v.co.x < min_x: min_x = v.co.x
            elif v.co.x > max_x: max_x = v.co.x
            if v.co.z < min_y: min_y = v.co.z
            elif v.co.z > max_y: max_y = v.co.z
            if -v.co.y < min_z: min_z = -v.co.y
            elif -v.co.y > max_z: max_z = -v.co.y
        
        bounds = Bounds()
        bounds.min = [min_x, min_y, min_z]
        bounds.max = [max_x, max_y, max_z]
        bounds.center = [(min_x + max_x) * 0.5, (min_y + max_y) * 0.5, (min_z + max_z) * 0.5]
        bounds.extents = [(max_x - min_x) * 0.5, (max_y - min_y) * 0.5, (max_z - min_z) * 0.5]
        bounds.size = [max_x - min_x, max_y - min_y, max_z - min_z]
    
    return bounds

def process_mesh(mesh):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.triangulate(bm, faces = bm.faces)
    bm.to_mesh(mesh)
    bm.free()

def write(context, filepath, options):
    time_0 = time.time()

    obj = context.active_object
    if obj is None or obj.type != 'MESH':
        self.report({'INFO'}, "No active mesh object to get info from")
        return False

    # Get evaluated object from depsgraph
    depsgraph = context.evaluated_depsgraph_get()
    object_eval = obj.evaluated_get(depsgraph)

    # Create a new mesh from the active object
    mesh_data = object_eval.to_mesh()
    
    # Triangulate mesh copy
    process_mesh(mesh_data)
    
    # Calculate bounding box from processed mesh
    bounds = get_bounding_box(mesh_data.vertices)
    
    target_cell_vert_count = 32
    orig_vertex_count = len(mesh_data.vertices)

    division_count = 0
    axis_cells = [1, 1, 1]
    while orig_vertex_count / (axis_cells[0] * axis_cells[1] * axis_cells[2]) > target_cell_vert_count:
        axis_cells[division_count % 3] += 1
        division_count += 1
    
    cell_count = axis_cells[0] * axis_cells[1] * axis_cells[2]
    cell_vertices = [[] for c in range(cell_count)]

    bounding_box_array = array('f')
    bounding_box_array.extend(bounds.center)
    bounding_box_array.extend(bounds.extents)

    # Get vertex color and texture coordinate layer counts
    vert_color_count = len(mesh_data.vertex_colors)
    tex_coord_count = len(mesh_data.uv_layers)
    
    # Decide which vertex data components to save
    save_normal = options['save_normal']
    save_tangent = options['save_tangent']
    save_bitangent = options['save_bitangent']
    save_vert_color = options['save_vert_color'] and vert_color_count > 0
    save_tex_coord = options['save_tex_coord'] and tex_coord_count > 0

    class Vertex:
        pos = []
        nor = []
        tan = []
        bit = []
        col = []
        uv0 = []

    def calc_bitangent(n, t, sign):
        return [(n[1]*t[2] - n[2]*t[1])*sign, (n[2]*t[0] - n[0]*t[2])*sign, (n[0]*t[1] - n[1]*t[0])*sign]
        
    def vec2_eq(a, b):
        return (abs(a[0] - b[0]) <= 1e-7 and
            abs(a[1] - b[1]) <= 1e-7)
        
    def vec3_eq(a, b):
        return (abs(a[0] - b[0]) <= 1e-7 and
            abs(a[1] - b[1]) <= 1e-7 and
            abs(a[2] - b[2]) <= 1e-7)

    cell_vertices = [[] for c in range(cell_count)]
    cell_indices = []
    vert_indices = []
    
    if save_tangent or save_bitangent:
        mesh_data.calc_tangents()

    time_1 = time.time()
    
    x_inv_divisor = 1 / (bounds.size[0] * axis_cells[0]) if bounds.size[0] > 0 else 0
    y_inv_divisor = 1 / (bounds.size[1] * axis_cells[1]) if bounds.size[1] > 0 else 0
    z_inv_divisor = 1 / (bounds.size[2] * axis_cells[2]) if bounds.size[2] > 0 else 0

    for poly in mesh_data.polygons: # For each triangle in mesh
        for loop_idx in poly.loop_indices: # For each corner in triangle
            loop = mesh_data.loops[loop_idx]

            # Construct our vertex data for this triangle corner
            this_vert = Vertex()
            this_vert.pos = mesh_data.vertices[loop.vertex_index].co
            if save_normal:
                this_vert.nor = loop.normal
            if save_tangent:
                this_vert.tan = loop.tangent
            if save_bitangent:
                this_vert.bit = calc_bitangent(loop.normal, loop.tangent, loop.bitangent_sign)
            if save_vert_color:
                this_vert.col = mesh_data.vertex_colors.active.data[loop_idx].color[0:3]
            if save_tex_coord:
                this_vert.uv0 = mesh_data.uv_layers.active.data[loop_idx].uv

            x_idx = math.floor((this_vert.pos[0] - bounds.min[0]) * x_inv_divisor)
            x_idx = axis_cells[0] - 1 if x_idx >= axis_cells[0] else (0 if x_idx < 0 else x_idx)
            y_idx = math.floor((this_vert.pos[1] - bounds.min[1]) * y_inv_divisor)
            y_idx = axis_cells[1] - 1 if y_idx >= axis_cells[1] else (0 if y_idx < 0 else y_idx)
            z_idx = math.floor((this_vert.pos[2] - bounds.min[2]) * z_inv_divisor)
            z_idx = axis_cells[2] - 1 if z_idx >= axis_cells[2] else (0 if z_idx < 0 else z_idx)
            cell_index = x_idx * axis_cells[2] * axis_cells[1] + y_idx * axis_cells[2] + z_idx
            cell = cell_vertices[cell_index]
            # Try to find an identical vertex
            vert_idx = -1
            for index, vert in enumerate(cell):
                if (vec3_eq(this_vert.pos, vert.pos) and
                    (save_normal is False or vec3_eq(this_vert.nor, vert.nor)) and
                    (save_tangent is False or vec3_eq(this_vert.tan, vert.tan)) and
                    (save_bitangent is False or vec3_eq(this_vert.bit, vert.bit)) and
                    (save_vert_color is False or vec3_eq(this_vert.col, vert.col)) and
                    (save_tex_coord is False or vec2_eq(this_vert.uv0, vert.uv0))):
                    vert_idx = index
                    break
            
            # If identical vertex exists, just add a new index to be drawn
            # Otherwise, add the vertex and add its index to the index list
            if vert_idx == -1:
                vert_idx = len(cell)
                cell.append(this_vert)

            cell_indices.append(cell_index)
            vert_indices.append(vert_idx)

    time_2 = time.time()
        
    # 'f' for 4-byte floating point
    vertex_array = array('f')
    vertex_count = 0
    cumulative_cell_verts = []

    for cell in cell_vertices:
        cumulative_cell_verts.append(vertex_count)
        for vert in cell:
            vertex_count += 1
            vertex_array.append(vert.pos[0])
            vertex_array.append(vert.pos[2])
            vertex_array.append(-vert.pos[1])
            if save_normal:
                vertex_array.append(vert.nor[0])
                vertex_array.append(vert.nor[2])
                vertex_array.append(-vert.nor[1])
            if save_tangent:
                vertex_array.append(vert.tan[0])
                vertex_array.append(vert.tan[2])
                vertex_array.append(-vert.tan[1])
            if save_bitangent:
                vertex_array.append(vert.bit[0])
                vertex_array.append(vert.bit[2])
                vertex_array.append(-vert.bit[1])
            if save_vert_color:
                vertex_array.extend(vert.col)
            if save_tex_coord:
                vertex_array.extend(vert.uv0)

    final_indices = []
    for vert_index, cell_index in zip(vert_indices, cell_indices):
        final_indices.append(cumulative_cell_verts[cell_index] + vert_index)
            
    index_count = len(final_indices)
    
    # 'H': unsigned 2-byte int, 'I': unsigned 4-byte int
    if vertex_count <= (1 << 16):
        index_array = array('H', final_indices)
        idx_attr = 2
        index_size = 2
    else:
        index_array = array('I', final_indices)
        idx_attr = 3
        index_size = 4

    time_3 = time.time()

    # Prepare data for the header
    
    # Vertex data components
    pos_attr = 2 << 2 # 3 component position
    nor_attr = 1 << 4 # 1 normals
    tan_attr = 1 << 5 # 1 tangents
    bit_attr = 1 << 6 # 1 bitangents
    col_attr_count = 1 << 7 # 1 color attribute
    tex_attr_count = 1 << 13 # 1 texture coord attribute
    
    # Combine vertex data components to one integer
    attribute_info = pos_attr | idx_attr
    if save_normal: attribute_info = attribute_info | nor_attr
    if save_tangent: attribute_info = attribute_info | tan_attr
    if save_bitangent: attribute_info = attribute_info | tan_attr
    if save_vert_color: attribute_info = attribute_info | col_attr_count
    if save_tex_coord: attribute_info = attribute_info | tex_attr_count

    header_size = 32
    bounds_data_size = 24
    vertex_data_size = len(vertex_array) * vertex_array.itemsize
    index_data_size = len(index_array) * index_array.itemsize

    file_magic = 0x10101991
    version_info = 0x00010000 # Version 1.0.0
    bounds_offset = header_size
    vertex_offset = bounds_offset + bounds_data_size
    index_offset = vertex_offset + vertex_data_size
    
    # '=': native byte-order, standard packing
    # 'I': unsigned 4-byte integer
    header = pack('=IIIIIIII',
        file_magic, version_info,
        attribute_info, bounds_offset,
        vertex_offset, vertex_count,
        index_offset, index_count)
    
    # Remove the mesh we created earlier
    obj.to_mesh_clear()

    # Open output file
    with open(filepath, 'wb') as outfile:
        outfile.write(header)
        bounding_box_array.tofile(outfile)
        vertex_array.tofile(outfile)
        index_array.tofile(outfile)
    
    time_4 = time.time()
    fileSizeBytes = index_offset + index_data_size
    
    print("Kokko mesh exported to path {}.".format(filepath))
    print("File size {:.1f} kiB.".format(fileSizeBytes / 1024))
    print("{} vertices, {:.0f} triangles".format(vertex_count, index_count / 3))
    print("Prepare mesh: {:.2f} s".format(time_1 - time_0))
    print("Find unique vertices: {:.2f} s".format(time_2 - time_1))
    print("Output data to arrays: {:.2f} s".format(time_3 - time_2))
    print("Writing to file: {:.2f} s".format(time_4 - time_3))
    print("Total: {:.2f} s".format(time_4 - time_0))

    return True
