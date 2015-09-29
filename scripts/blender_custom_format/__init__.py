bl_info = {
    "name": "Custom mesh export",
    "description": "Export mesh to custom file format",
    "author": "Aleksi Grön",
    "version": (0, 2),
    "blender": (2, 63, 0),
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

    def execute(self, context):
        from . import export
        export.write(context, self.filepath)
        
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
