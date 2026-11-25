# GLB -> FBX for Unreal's importer, in headless Blender 5.0:
#
#   blender -b --factory-startup --python glb_to_fbx.py -- <in.glb> <out.fbx> [<out.fbx> ...]
#
# It calls the FBX exporter's own save() directly with every option spelled
# out. In this Blender build (5.0.1, Ubuntu package) the export operator
# registers only a path and two axes, so bpy.ops.export_scene.fbx fails on
# its own sixth option; the function underneath is whole.
#
# Defaults are the exporter's own, so the output matches a normal
# Blender FBX export. The four that differ, for Unreal:
#   - no leaf bones                      (Unreal shows them as extra bones)
#   - one FBX take per clip, no NLA strips (each becomes an AnimSequence)
#   - no key simplification              (the clips arrive as authored)
#   - textures as files beside the FBX    (Unreal's importer cannot read Blender's embedded ones)
# Nothing is rescaled: the GLBs are in the pipeline's normalized units
# (meters), which Unreal converts to centimetres.
import sys
import bpy
from mathutils import Matrix
from bpy_extras.io_utils import axis_conversion


class Reporter:
    """What save() needs from an operator: a place to report through."""

    def report(self, level, message):
        print("FBX", sorted(level), message, flush=True)


def convert(src, dst):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    # The clips are authored at 30 fps. Blender's factory rate is 24, at which a
    # 53-frame clip lands on a fractional frame and Unreal's importer refuses it
    # ("not compatible with import frame-rate"). Export at the authored rate.
    bpy.context.scene.render.fps = 30
    bpy.context.scene.render.fps_base = 1.0
    bpy.ops.import_scene.gltf(filepath=src)
    # The glTF importer gives the bones a display shape (an "Icosphere") in a
    # collection it names glTF_not_exported. Left in, it exports as a second
    # skinned mesh. Take the collection's advice.
    for collection in list(bpy.data.collections):
        if collection.name.startswith("glTF_not_exported"):
            for obj in list(collection.objects):
                bpy.data.objects.remove(obj, do_unlink=True)
            bpy.data.collections.remove(collection)
    # The GLB's images are packed inside the scene with no file behind them, and
    # Unreal's importer cannot read Blender's embedded ones. Write each image
    # out beside the FBX, as a file, and point the image at it, so the exporter
    # references a path the importer can open.
    import os
    out_dir = os.path.dirname(dst)
    for image in bpy.data.images:
        if image.size[0] == 0:
            continue
        image_path = os.path.join(out_dir, bpy.path.clean_name(image.name) + ".png")
        image.filepath_raw = image_path
        image.file_format = 'PNG'
        image.save()
        image.source = 'FILE'
        image.filepath = image_path
        image.reload()
        print("IMAGE", os.path.basename(image_path), tuple(image.size), flush=True)

    # Unreal's FBX importer capitalizes the first letter of every bone name of
    # a skeletal mesh (Pelvis, Spine_01) but keeps an animation track's name as
    # written (pelvis, spine_01). Same bone, different case, no track finds its
    # bone and the mesh never leaves its bind pose. Capitalize here, once, so
    # both sides are spelled the same before they meet.
    for armature in [o for o in bpy.data.objects if o.type == "ARMATURE"]:
        for bone in armature.data.bones:
            bone.name = bone.name[:1].upper() + bone.name[1:]
        print("BONES", armature.name, [b.name for b in armature.data.bones][:4], flush=True)
    print("SCENE", sorted("%s(%s)" % (o.name, o.type) for o in bpy.data.objects),
          "fps", bpy.context.scene.render.fps, flush=True)
    from io_scene_fbx import export_fbx_bin

    ret = export_fbx_bin.save(
        Reporter(),
        bpy.context,
        filepath=dst,
        use_selection=False, use_visible=False, use_active_collection=False,
        collection="", batch_mode='OFF', use_batch_own_dir=False,
        # the exporter's own defaults, as its operator passes them
        global_matrix=axis_conversion(to_forward='-Z', to_up='Y').to_4x4(),
        axis_forward='-Z', axis_up='Y',
        apply_unit_scale=True, global_scale=1.0, apply_scale_options='FBX_SCALE_NONE',
        bake_space_transform=False, armature_nodetype='NULL',
        object_types={'EMPTY', 'ARMATURE', 'MESH'},
        use_mesh_modifiers=True, use_mesh_modifiers_render=True,
        mesh_smooth_type='FACE', use_subsurf=False, use_armature_deform_only=False,
        bake_anim=True, bake_anim_use_all_bones=True, bake_anim_step=1.0,
        bake_anim_force_startend_keying=True,
        use_metadata=True, use_mesh_edges=True, use_tspace=True, use_triangles=False,
        use_custom_props=False, colors_type='SRGB', prioritize_active_color=False,
        # the four that differ, for Unreal
        add_leaf_bones=False,
        primary_bone_axis='Y', secondary_bone_axis='X',
        bake_anim_use_nla_strips=False, bake_anim_use_all_actions=True,
        bake_anim_simplify_factor=0.0,
        path_mode='COPY', embed_textures=False,
    )
    print("EXPORTED", dst, ret, flush=True)


args = sys.argv[sys.argv.index("--") + 1:]
src, dsts = args[0], args[1:]
for dst in dsts:
    convert(src, dst)
