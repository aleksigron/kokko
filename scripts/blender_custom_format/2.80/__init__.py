bl_info = {
    "name": "Export Kokko Engine mesh",
    "description": "Export meshes to custom file format to be used by Kokko Engine",
    "author": "Aleksi Grön",
    "version": (0, 3, 0),
    "blender": (2, 80, 0),
    "location": "File > Export",
    "category": "Import-Export"
}

import bpy
from bpy.props import StringProperty, BoolProperty
from bpy_extras.io_utils import ExportHelper

class CUSTOM_OT_export_format(bpy.types.Operator, ExportHelper):
    """Export Kokko Engine mesh"""
    bl_idname = "export_mesh.gtk_mesh"
    bl_label = "Export Kokko mesh"
    
    filename_ext = ".mesh"
    filter_glob = StringProperty(default="*.mesh", options={'HIDDEN'})
    
    save_normal: BoolProperty(
        name = "Save vertex normals",
        default = True)
        
    save_tangent: BoolProperty(
        name = "Save vertex tangents",
        default = True)
        
    save_bitangent: BoolProperty(
        name = "Save vertex bitangents",
        default = False)
        
    save_vert_color: BoolProperty(
        name = "Save vertex colors (if available)",
        default = False)
        
    save_tex_coord: BoolProperty(
        name = "Save texture coordinates (if available)",
        default = True)

    def execute(self, context):
        options = {}
        options["save_normal"] = self.save_normal
        options["save_tangent"] = self.save_tangent
        options["save_bitangent"] = self.save_bitangent
        options["save_tex_coord"] = self.save_tex_coord
        options["save_vert_color"] = self.save_vert_color
        
        from . import export
        export.write(context, self.filepath, options)
        
        return {'FINISHED'}


def menuItemFunc_Export(self, context):
    self.layout.operator(CUSTOM_OT_export_format.bl_idname, text="Kokko Engine mesh (.mesh)")

def register():
    from bpy.utils import register_class
    register_class(CUSTOM_OT_export_format)
    bpy.types.TOPBAR_MT_file_export.append(menuItemFunc_Export)

def unregister():
    from bpy.utils import unregister_class
    unregister_class(CUSTOM_OT_export_format)
    bpy.types.TOPBAR_MT_file_export.remove(menuItemFunc_Export)

if __name__ == "__main__":
    register()
