import bpy
import bgl
import time
import numpy as np
import moderngl
import ghalton
import threading
import logging

from PIL import Image

EGNINE_BL_IDNAME = "FASTA"

logger = logging.getLogger(__name__)

class FastaRenderer():
    def __init__(self):
        if not mgl_context:
            # create default for offscreen rendering
            try:
                self.ctx = moderngl.create_standalone_context(require=330)
            except Exception as ex:
                logger.error("Error creating standalone mgl context: %s" % ex)
        else:
            self.ctx = mgl_context

        self.initialized = False
        self.scene = []
        self.background = None
        self._camera = Camera()
        self._film_is_transparent = False

        self.width, self.height = None, None
        self._frame_buffer, self._render_buffer, self._dof_buffer = None, None, None

        print("Fasta Renderer init done!")

    def __del__(self):
        print("Fasta Renderer destroyed!")

    def init(self, width, height):
        if not self.initialized:
            # World background
            self.background = SimpleBackground(self.ctx)

            # A fullscreen quad just for rendering one pass to offscreen textures
            self.quad_fs = QuadFS(self.ctx)
            self.quad_fs.m_model.write(Matrix44.identity().astype('f4').tobytes())
            self.quad_fs.m_view.write(Matrix44.identity().astype('f4').tobytes())
            self.quad_fs.m_proj.write(Matrix44.orthogonal_projection(-1, 1, 1, -1, 1, 10).astype('f4').tobytes())
            self.quad_fs.flip_v.value = 0

            #DOF
            self.quad_dof = QuadFS_DOF(self.ctx)
            self.quad_dof.m_model.write(Matrix44.identity().astype('f4').tobytes())
            self.quad_dof.m_view.write(Matrix44.identity().astype('f4').tobytes())
            self.quad_dof.m_proj.write(Matrix44.orthogonal_projection(-1, 1, 1, -1, 1, 10).astype('f4').tobytes())

            # default aa sampler
            self.setRenderSamples(16)

            self.initialized = True
            print("Fasta Renderer initialized!")

        if (self.width, self.height) != (width, height):
            try:
                self.resize(width, height)
            except:
                logger.exception("Unable to resize render buffers")

    def setScene(self, depsgraph):
        # iterate depsgraph
        for ob_inst in depsgraph.object_instances:
            if ob_inst.is_instance:  # Real dupli instance
                ob = ob_inst.instance_object.original
                parent = ob_inst.parent.original
                mat = ob_inst.matrix_world.copy()
            else:  # Usual object
                ob = ob_inst.object.original
                self.scene.append(ObjectDrawable(self.ctx, ob))

    def setCamera(self, bl_camera):
        if bl_camera is None:
            logger.debug("No scene camera provided !!! Using renderer default ...")
            return
        elif bl_camera.type == 'CAMERA':
            # blender scene camera
            self._camera.setBlenderCamera(bl_camera)
        else:
            # strange thing or object as camera
            return

    def setRenderSamples(self, samples):
        self.render_samples = samples
        self.render_sample_num = 0
        self.quad_fs.program['aa_passes'].value = self.render_samples

        sequencer = ghalton.GeneralizedHalton(ghalton.EA_PERMS[:2])
        self.sampling_points = sequencer.get(samples)

    def renderSample(self, append_to_image_data=False):
        #print("Fasta Renderer render sample...")
        m_view = self._camera.getWorldTransformMatrix()
        m_proj = self._camera.getProjection(jittered=True, point=self.sampling_points[self.render_sample_num])

        # Render the scene to offscreen buffer
        #self.ctx.multisample = False
        self._render_buffer.use()
        self._render_buffer.clear(0.0, 0.0, 0.0, 0.0)

        # Render world
        if not self._film_is_transparent:
            self.ctx.disable(moderngl.DEPTH_TEST)
            self.background.draw()

        # Render scene
        self.ctx.enable(moderngl.DEPTH_TEST)
        #print("Drawing drawables")
        for drawable in self.scene:
            drawable.surface_prog["model"].write(drawable._xform.astype('f4').tobytes())
            drawable.surface_prog["view"].write(m_view.astype('f4').tobytes())
            drawable.surface_prog["projection"].write(m_proj.astype('f4').tobytes())
            drawable.draw()

        # DOF pass
        self._dof_buffer.use()
        self._dof_buffer.clear(0.0, 0.0, 0.0, 0.0)
        self._render_buffer_albedo.use(location=0) # ????
        self._render_buffer_depth.use(location=1)
        self.ctx.disable(moderngl.DEPTH_TEST)
        self.quad_dof.render()

        # Render final render quad
        # clear if it's first pass
        if self.render_sample_num == 0:
            self._frame_buffer.clear(0.0, 0.0, 0.0, 0.0, 0.0)

        self.ctx.disable(moderngl.DEPTH_TEST)

        self._frame_buffer.use()
        self._dof_buffer_color.use(location=0)

        self.quad_fs.program['aa_pass'].value = self.render_sample_num

        self.ctx.disable(moderngl.DEPTH_TEST)        
        self.ctx.enable(moderngl.BLEND)
        self.ctx.blend_equation = moderngl.FUNC_ADD
        self.ctx.blend_func = (
                    moderngl.SRC_ALPHA, moderngl.ONE_MINUS_SRC_ALPHA,
                    moderngl.ONE, moderngl.ONE_MINUS_SRC_ALPHA
                )
        self.quad_fs.flip_v.value = 1
        self.quad_fs.render()
        self.ctx.disable(moderngl.BLEND)
        
        self.ctx.finish()

        # adjust render samples
        self.render_sample_num += 1


        # IPR stuff
        if append_to_image_data: # for IPR
            self._frame_buffer.read_into(self._image_data, components=4, attachment=0, alignment=1, dtype='f2')


    def renderFrame(self, samples=8):
        self.setRenderSamples(samples)

        start = time.time()
        for sample in range(samples):
            self.renderSample()

        self._frame_buffer.read_into(self._image_data, components=4, attachment=0, alignment=1, dtype='f2')
        self._frame_buffer.read_into(self._depth_data, components=1, attachment=1, alignment=1, dtype='f2')
        #Image.frombytes('L', (self.width, self.height), self._frame_buffer.read(components=1, attachment=1)).show()

        end = time.time()
        print("Frame with %s samples rendered in %s sec." % (samples, (end - start)))
        return self._image_data, self._depth_data

    def resize(self, width, height):
        self.width, self.height = width, height
        buffer_size = (width, height)

        # offscreen render target
        if self._render_buffer:
            self._render_buffer.release()
            self._render_buffer_albedo.release()
            self._render_buffer_metalness.release()
            self._render_buffer_roughness.release()
            self._render_buffer_normals.release()
            self._render_buffer_depth.release()

        self._render_buffer_albedo = self.ctx.texture(buffer_size, 4, dtype='f2') # RGBA albedo(RGB)/alpha(A) layer
        self._render_buffer_albedo.repeat_x = False
        self._render_buffer_albedo.repeat_y = False

        self._render_buffer_metalness = self.ctx.texture(buffer_size, 1, dtype='i1') # metalness layer
        self._render_buffer_roughness = self.ctx.texture(buffer_size, 1, dtype='i1') # roughness layer
        self._render_buffer_normals = self.ctx.texture(buffer_size, 3, dtype='f2') # Normals layer (16 bit floats)
        self._render_buffer_depth = self.ctx.depth_texture(buffer_size) # Texture for storing depth values
        self._render_buffer_depth.compare_func = ''
        self._render_buffer_depth.repeat_x = False
        self._render_buffer_depth.repeat_y = False

        # create a framebuffer we can render to
        self._render_buffer = self.ctx.framebuffer(
            color_attachments=[
                self._render_buffer_albedo,
                self._render_buffer_metalness,
                self._render_buffer_roughness,
                self._render_buffer_normals
            ],
            depth_attachment=self._render_buffer_depth,
        )
        self._render_buffer.viewport = (0, 0, width, height)

        # dof pass render target
        if self._dof_buffer:
            self._dof_buffer.release()
            self._dof_buffer_color.release()
            self._dof_buffer_depth.release()

        self._dof_buffer_color = self.ctx.texture(buffer_size, 4, dtype='f2') # RGBA albedo(RGB)/alpha(A) layer
        self._dof_buffer_depth = self.ctx.depth_texture(buffer_size) # Texture for storing depth values

        # create a framebuffer we can render to
        self._dof_buffer = self.ctx.framebuffer(
            color_attachments=[
                self._dof_buffer_color
            ],
            depth_attachment=self._dof_buffer_depth,
        )
        self._dof_buffer.viewport = (0, 0, width, height)

        # frame/accumulation buffer
        if self._frame_buffer:
            self._frame_buffer.release()
            self._frame_buffer_diffuse.release()
            self._frame_buffer_depth.release()

        self._frame_buffer_diffuse = self.ctx.texture(buffer_size, 4, dtype='f2') # RGBA color layer
        self._frame_buffer_depth = self.ctx.depth_texture(buffer_size) # Texture for storing depth values

        # create a framebuffer we can render to
        self._frame_buffer = self.ctx.framebuffer(
            color_attachments=[
                self._frame_buffer_diffuse,
            ],
            depth_attachment=self._frame_buffer_depth,
        )
        self._frame_buffer.viewport = (0, 0, width, height)

        self._image_data = np.empty((width * height,), dtype="4float16")
        self._depth_data = np.empty((width * height, 1), dtype="float16")

        self._camera.setViewportDimensions(width, height)

    @property
    def image_data(self):        
        return self._image_data

class IPRThread(threading.Thread):
    def __init__(self, renderer):
        threading.Thread.__init__(self)

        self._renderer = renderer
        self._stop_event = threading.Event()

    def stop(self):
        self._stop_event.set()

    def stopped(self):
        return self._stop_event.isSet()

    def run(self):
        while not self._stop_event.is_set():
            self._renderer.renderSample()
            time.sleep(1.0)

        print("Fasta IPR stopped!")


class FastaRendererIPR():
    def __init__(self):
        self._renderer = FastaRenderer(moderngl.create_context())
        self._ipr_thread = None

        print("Fasta IPR init done!")

    def __del__(self):
        if self._ipr_thread:
            self._ipr_thread.stop()
            self._ipr_thread.join()

        print("Fasta IPR destroyed!")

    def stop(self):
        if self._ipr_thread:
            self._ipr_thread.stop()
            self._ipr_thread = None

    def init(self, w, h):
        self._renderer.init(w, h)

    def isRunning(self):
        if self._ipr_thread:
            return True

    def start(self):
        print("Starting/restarting Fasta IPR.")
        #if self._ipr_thread:
        #   self._ipr_thread.stop()
        #   self._ipr_thread.join()

        #self._ipr_thread = IPRThread(self._renderer)
        #self._ipr_thread.start()
        self._renderer.renderSample()

    @property
    def image_data(self):
        return self._renderer.image_data


class CustomRenderEngine(bpy.types.RenderEngine):
    # These three members are used by blender to set up the
    # RenderEngine; define its internal name, visible name and capabilities.
    bl_idname = EGNINE_BL_IDNAME
    bl_label = "Fasta"
    bl_use_preview = True

    # Init is called whenever a new render engine instance is created. Multiple
    # instances may exist at the same time, for example for a viewport and final
    # render.
    def __init__(self):
        self.scene_data = None
        self.draw_data = None

        self.ipr_renderer = FastaRendererIPR()

    # When the render engine instance is destroy, this is called. Clean up any
    # render engine data here, for example stopping running render threads.
    def __del__(self):
        pass

    # This is the method called by Blender for both final renders (F12) and
    # small preview for materials, world and lights.
    def render(self, depsgraph):
        print("Fasta render frame")
        start = time.time()

        scene = depsgraph.scene
        scale = scene.render.resolution_percentage / 100.0
        self.size_x = int(scene.render.resolution_x * scale)
        self.size_y = int(scene.render.resolution_y * scale)

        # renderer instance
        renderer = FastaRenderer()

        r_start = time.time()
        try:
            renderer.init(self.size_x, self.size_y)
            renderer.setScene(depsgraph)
            renderer.setCamera(scene.camera)
            r_end = time.time()
        except:
            raise

        # render frame
        image_data, depth_data = renderer.renderFrame(1)
        print("Pure rendering time is: %s sec." % (r_end - r_start))

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, self.size_x, self.size_y)
        layer = result.layers[0].passes["Combined"]
        layer.rect = image_data.tolist()

        layer = result.layers[0].passes["Depth"]
        layer.rect = depth_data.tolist()

        self.end_result(result)

        end = time.time()
        print("Image rendered in %s sec." % (end - start))

    # For viewport renders, this method gets called once at the start and
    # whenever the scene or 3D viewport changes. This method is where data
    # should be read from Blender in the same thread. Typically a render
    # thread will be started to do the work while keeping Blender responsive.
    def view_update(self, context, depsgraph):
        print("Fasta render view_update")
        region = context.region
        view3d = context.space_data
        scene = depsgraph.scene

        # Get viewport dimensions
        dimensions = region.width, region.height

        if not self.scene_data:
            # First time initialization
            self.scene_data = []
            first_time = True

            # Loop over all datablocks used in the scene.
            for datablock in depsgraph.ids:
                pass
        else:
            first_time = False

            # Test which datablocks changed
            for update in depsgraph.updates:
                print("Datablock updated: ", update.id.name)

            # Test if any material was added, removed or changed.
            if depsgraph.id_type_updated('MATERIAL'):
                print("Materials updated")

        # Loop over all object instances in the scene.
        if first_time or depsgraph.id_type_updated('OBJECT'):
            for instance in depsgraph.object_instances:
                pass

    # For viewport renders, this method is called whenever Blender redraws
    # the 3D viewport. The renderer is expected to quickly draw the render
    # with OpenGL, and not perform other expensive work.
    # Blender will draw overlays for selection and editing on top of the
    # rendered image automatically.
    def view_draw(self, context, depsgraph):
        print("Fasta render view_draw")
        region = context.region
        scene = depsgraph.scene

        # Get viewport dimensions
        dimensions = region.width, region.height

        # Init/resize ipr renderer
        self.ipr_renderer.init(region.width, region.height)
        self.ipr_renderer.start()

        # Bind shader that converts from scene linear to display space,
        bgl.glEnable(bgl.GL_BLEND)
        bgl.glBlendFunc(bgl.GL_ONE, bgl.GL_ONE_MINUS_SRC_ALPHA)
        self.bind_display_space_shader(scene)

        if not self.draw_data or self.draw_data.dimensions != dimensions:
            self.draw_data = CustomDrawData(dimensions, self.ipr_renderer)

        self.draw_data.draw()

        self.unbind_display_space_shader()
        bgl.glDisable(bgl.GL_BLEND)


class CustomDrawData:
    def __init__(self, dimensions, ipr_renderer):
        # Generate dummy float image buffer
        self.dimensions = dimensions
        width, height = dimensions

        #pixels = [0.1, 0.2, 0.1, 1.0] * width * height
        #pixels = bgl.Buffer(bgl.GL_FLOAT, width * height * 4, pixels)
        pixels = bgl.Buffer(bgl.GL_FLOAT, width * height * 4, ipr_renderer.image_data.astype("float32").ravel())

        # Generate texture
        self.texture = bgl.Buffer(bgl.GL_INT, 1)
        bgl.glGenTextures(1, self.texture)
        bgl.glActiveTexture(bgl.GL_TEXTURE0)
        bgl.glBindTexture(bgl.GL_TEXTURE_2D, self.texture[0])

        bgl.glTexImage2D(bgl.GL_TEXTURE_2D, 0, bgl.GL_RGBA16F, width, height, 0, bgl.GL_RGBA, bgl.GL_FLOAT, pixels)

        bgl.glTexParameteri(bgl.GL_TEXTURE_2D, bgl.GL_TEXTURE_MIN_FILTER, bgl.GL_LINEAR)
        bgl.glTexParameteri(bgl.GL_TEXTURE_2D, bgl.GL_TEXTURE_MAG_FILTER, bgl.GL_LINEAR)
        bgl.glBindTexture(bgl.GL_TEXTURE_2D, 0)

        # Bind shader that converts from scene linear to display space,
        # use the scene's color management settings.
        shader_program = bgl.Buffer(bgl.GL_INT, 1)
        bgl.glGetIntegerv(bgl.GL_CURRENT_PROGRAM, shader_program)

        # Generate vertex array
        self.vertex_array = bgl.Buffer(bgl.GL_INT, 1)
        bgl.glGenVertexArrays(1, self.vertex_array)
        bgl.glBindVertexArray(self.vertex_array[0])

        texturecoord_location = bgl.glGetAttribLocation(shader_program[0], "texCoord")
        position_location = bgl.glGetAttribLocation(shader_program[0], "pos")

        bgl.glEnableVertexAttribArray(texturecoord_location)
        bgl.glEnableVertexAttribArray(position_location)

        # Generate geometry buffers for drawing textured quad
        position = [0.0, 0.0, width, 0.0, width, height, 0.0, height]
        position = bgl.Buffer(bgl.GL_FLOAT, len(position), position)
        texcoord = [0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0]
        texcoord = bgl.Buffer(bgl.GL_FLOAT, len(texcoord), texcoord)

        self.vertex_buffer = bgl.Buffer(bgl.GL_INT, 2)

        bgl.glGenBuffers(2, self.vertex_buffer)
        bgl.glBindBuffer(bgl.GL_ARRAY_BUFFER, self.vertex_buffer[0])
        bgl.glBufferData(bgl.GL_ARRAY_BUFFER, 32, position, bgl.GL_STATIC_DRAW)
        bgl.glVertexAttribPointer(position_location, 2, bgl.GL_FLOAT, bgl.GL_FALSE, 0, None)

        bgl.glBindBuffer(bgl.GL_ARRAY_BUFFER, self.vertex_buffer[1])
        bgl.glBufferData(bgl.GL_ARRAY_BUFFER, 32, texcoord, bgl.GL_STATIC_DRAW)
        bgl.glVertexAttribPointer(texturecoord_location, 2, bgl.GL_FLOAT, bgl.GL_FALSE, 0, None)

        bgl.glBindBuffer(bgl.GL_ARRAY_BUFFER, 0)
        bgl.glBindVertexArray(0)

        print("Fasta CustomDrawData init done!")

    def __del__(self):
        bgl.glDeleteBuffers(2, self.vertex_buffer)
        bgl.glDeleteVertexArrays(1, self.vertex_array)
        bgl.glBindTexture(bgl.GL_TEXTURE_2D, 0)
        bgl.glDeleteTextures(1, self.texture)

        print("Fasta CustomDrawData destroyed!")

    def draw(self):
        bgl.glActiveTexture(bgl.GL_TEXTURE0)

        bgl.glBindTexture(bgl.GL_TEXTURE_2D, self.texture[0])

        bgl.glBindVertexArray(self.vertex_array[0])
        bgl.glDrawArrays(bgl.GL_TRIANGLE_FAN, 0, 4)
        bgl.glBindVertexArray(0)
        bgl.glBindTexture(bgl.GL_TEXTURE_2D, 0)
