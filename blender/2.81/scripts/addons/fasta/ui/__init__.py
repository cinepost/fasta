import bpy

__all__ = ('FST_Panel', 'register', 'unregister')

class FST_Panel(bpy.types.Panel):
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = 'render'
    COMPAT_ENGINES = {'XENON'}

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES

    @staticmethod
    def create_ui_autosize_column(context, column, single=False):
        if context.region.width > PANEL_WIDTH_FOR_COLUMN:
            row = column.row()
            split = row.split(factor=0.5)
            column1 = split.column(align=True)
            split = split.split()
            column2 = split.column(align=True)
            is_row = False
        else:
            column1 = column.row().column(align=True)
            if not single:
                column.separator()
            column2 = column.row().column(align=True)
            is_row = True
        return column1, column2, is_row

from . import render

register_classes, unregister_classes = bpy.utils.register_classes_factory([
	render.FST_RENDER_PT_quality,
	render.FST_RENDER_PT_film_transparency,
])

def register():
	register_classes()

def unregister():
	unregister_classes()