# Imports the bridged FBX files into the project, headless:
#
#   UnrealEditor-Cmd <project> -ExecutePythonScript=<this file> -nullrhi -unattended -NoSound
#
# Each asset is imported with the editor's FBX factory, textures and materials
# included; the two characters as skeletal meshes with every take in the file.
# Prints one IMPORT line per task so the caller can read what was created.
import os
import unreal

ROOT = "/home/kirk/UnrealEngine/Projects/RpgUeProof/AssetLab/fbx"

JOBS = [
    # name, destination folder, skeletal
    ("Crypt_Rug_01", "/Game/WebBridge/Props", False),
    ("Crypt_Wall_Base_01", "/Game/WebBridge/Env", False),
    ("fighter", "/Game/WebBridge/Characters", True),
    ("skeleton-soldier-01", "/Game/WebBridge/Characters", True),
]


def import_fbx(name, destination, skeletal):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(ROOT, name + ".fbx"))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)

    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("mesh_type_to_import",
                                unreal.FBXImportType.FBXIT_SKELETAL_MESH if skeletal else unreal.FBXImportType.FBXIT_STATIC_MESH)
    options.set_editor_property("import_animations", skeletal)
    options.set_editor_property("create_physics_asset", True)
    options.set_editor_property("automated_import_should_detect_type", False)
    task.set_editor_property("options", options)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.get_editor_property("imported_object_paths"))


for job_name, job_destination, job_skeletal in JOBS:
    paths = import_fbx(job_name, job_destination, job_skeletal)
    print("IMPORT", job_name, "skeletal" if job_skeletal else "static", len(paths), "objects")
    for path in paths:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        kind = asset.get_class().get_name() if asset else "FAILED TO LOAD"
        extra = ""
        if isinstance(asset, unreal.AnimSequence):
            extra = " num_frames=%d length=%.2fs" % (asset.get_editor_property("number_of_sampled_frames"),
                                                     asset.get_editor_property("sequence_length"))
        if isinstance(asset, unreal.SkeletalMesh):
            extra = " bones=%d" % len(asset.get_editor_property("ref_skeleton").get_editor_property("ref_bone_info")) if False else ""
        print("IMPORT   ", path.split(".")[-1], kind, extra)
print("IMPORT DONE")
