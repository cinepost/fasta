from .renderer import CustomRenderEngine, EGNINE_BL_IDNAME
import bpy

bl_info = {
	"name":  		"Fasta",
	"description":	"Dead simple, stupid fast.",
	"author":		"Parovoz",
	"version":		(0, 0, 1),
	"blender":		(2, 81, 0),
	"location":		"Info header. render engine menu",
	"category":		"Render",
	"warning":		"In-house development version.",
	"tracker_url": 	"",
	"wiki_url":		""
}

from . import (
	ui,
)

# RenderEngines also need to tell UI Panels that they are compatible with.
# We recommend to enable all panels marked as BLENDER_RENDER, and then
# exclude any panels that are replaced by custom panels registered by the
# render engine, or that are not supported.
def get_panels():
	exclude_panels = {
		'VIEWLAYER_PT_filter',
		'VIEWLAYER_PT_layer_passes',
	}

	panels = []
	for panel in bpy.types.Panel.__subclasses__():
		if hasattr(panel, 'COMPAT_ENGINES') and 'BLENDER_RENDER' in panel.COMPAT_ENGINES:
			if panel.__name__ not in exclude_panels:
				panels.append(panel)

	return panels

def register():
	# Register the RenderEngine
	bpy.utils.register_class(CustomRenderEngine)

	for panel in get_panels():
		panel.COMPAT_ENGINES.add(EGNINE_BL_IDNAME)

	ui.register()


def unregister():
	bpy.utils.unregister_class(CustomRenderEngine)

	for panel in get_panels():
		if EGNINE_BL_IDNAME in panel.COMPAT_ENGINES:
			panel.COMPAT_ENGINES.remove(EGNINE_BL_IDNAME)

	ui.unregister()