bl_info = {
    "name": "Custom mesh export",
    "description": "Export mesh to custom file format",
    "author": "Aleksi Grön",
    "version": (0, 2),
    "blender": (2, 57, 0),
    "location": "File > Export",
    "category": "Import-Export"
}

import bpy
from bpy.props import StringProperty, BoolProperty
from bpy_extras.io_utils import ExportHelper

class CustomExportFormat(bpy.types.Operator, ExportHelper):
    """Custom format mesh export script"""
    bl_idname = "export_mesh.gtk_mesh"
    bl_label = "Export custom mesh"
    
    filename_ext = ".mesh"
    filter_glob = StringProperty(default="*.mesh", options={'HIDDEN'})
    
    apply_modifiers = BoolProperty(
        name = "Apply Modifiers",
        description = "Apply mesh modifiers before exporting",
        default = True)

    def execute(self, context):
        from . import export
        err = export.write(context, self.filepath, self.apply_modifiers)
        if err: print(err)
        
        return {'FINISHED'}


def menuItemFunc_Export(self, context):
    self.layout.operator(CustomExportFormat.bl_idname, text="Custom file (.mesh)")

def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_export.append(menuItemFunc_Export)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_export.remove(menuItemFunc_Export)

if __name__ == "__main__":
    register()
