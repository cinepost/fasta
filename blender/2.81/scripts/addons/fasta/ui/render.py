from . import FST_Panel

class FST_RENDER_PT_quality(FST_Panel):
    bl_label = "Sampling"
    bl_context = 'render'

    def draw(self, context):
        self.layout.use_property_split = True
        self.layout.use_property_decorate = False

        fst = context.scene.fst
        quality = fst.quality

        col = self.layout.column(align=True)
        row = col.row()
        row.prop(quality, 'min_samples')
        col.prop(quality, 'max_samples')
        row = col.row()
        row.prop(quality, 'noise_threshold', slider = True)
        col.prop(quality, 'update_samples')

class FST_RENDER_PT_film_transparency(FST_Panel):
    bl_label = "Film"
    bl_options = {'DEFAULT_CLOSED'}

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False

        scene = context.scene

        layout.prop(scene.render, "film_transparent", text="Transparent Background")