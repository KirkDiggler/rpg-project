# Lists what an import actually built under /Game/WebBridge: every asset with
# its class, each mesh's material slots, each material instance's parent, and
# each texture's size. Headless:
#
#   UnrealEditor-Cmd <project> -ExecutePythonScript=<this file> -nullrhi -unattended -NoSound
#
# Every optional detail is tried separately: a property this version does not
# expose must not hide the rest.
import unreal

library = unreal.EditorAssetLibrary


def detail(asset):
    try:
        if isinstance(asset, unreal.Texture2D):
            size = asset.get_editor_property("imported_size")
            return "%dx%d imported" % (size.x, size.y)
    except Exception as error:
        return "(texture size unreadable: %s)" % error
    try:
        if isinstance(asset, unreal.MaterialInstanceConstant):
            parent = asset.get_editor_property("parent")
            parameters = [str(p) for p in asset.get_editor_property("texture_parameters")]
            return "parent=%s texture_parameters=%s" % (parent.get_name() if parent else None, parameters)
    except Exception as error:
        return "(instance unreadable: %s)" % error
    try:
        if isinstance(asset, unreal.Material):
            return "(a material) base_color=%s" % asset.get_editor_property("base_color")
    except Exception:
        return "(a material)"
    try:
        if isinstance(asset, unreal.SkeletalMesh):
            return "slots=%s" % [str(m.material_interface) for m in asset.get_editor_property("materials")]
    except Exception:
        return "(skeletal)"
    try:
        if isinstance(asset, unreal.StaticMesh):
            return "slots=%s" % [str(m.material_interface) for m in asset.get_editor_property("static_materials")]
    except Exception:
        return "(static)"
    try:
        if isinstance(asset, unreal.AnimSequence):
            return "frames=%d length=%.3fs" % (asset.get_editor_property("number_of_sampled_frames"),
                                               asset.get_editor_property("sequence_length"))
    except Exception:
        return "(animation)"
    return ""


paths = library.list_assets("/Game/WebBridge", recursive=True, include_folder=False)
unreal.log("CENSUS %d assets" % len(paths))
for path in sorted(paths):
    asset = library.load_asset(path)
    name = path.replace("/Game/WebBridge/", "").split(".")[0]
    if asset is None:
        unreal.log("CENSUS %-46s FAILED TO LOAD" % name)
        continue
    unreal.log("CENSUS %-46s %-26s %s" % (name, asset.get_class().get_name(), detail(asset)))
