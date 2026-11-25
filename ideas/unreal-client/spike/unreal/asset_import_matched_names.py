# Imports the bridged characters that have their bone names matched to
# Unreal's capitalization, into /Game/WebBridgeCap/Characters. Headless:
#
#   UnrealEditor-Cmd <project> -ExecutePythonScript=<this file> -nullrhi -unattended -NoSound
#
# Each becomes a skeletal mesh with a skeleton, its clips as animation
# sequences, its texture and a material instance. See glb_to_fbx.py for the
# export and its four choices.
import unreal

ROOT = "/home/kirk/UnrealEngine/Projects/RpgUeProof/AssetLab/fbx-cap"
DESTINATION = "/Game/WebBridgeCap/Characters"

options = unreal.FbxImportUI()
options.set_editor_property("import_mesh", True)
options.set_editor_property("import_materials", True)
options.set_editor_property("import_textures", True)
options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
options.set_editor_property("import_animations", True)
options.set_editor_property("create_physics_asset", True)
options.set_editor_property("automated_import_should_detect_type", False)

tasks = []
for name in ["fighter", "skeleton-soldier-01"]:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", ROOT + "/" + name + ".fbx")
    task.set_editor_property("destination_path", DESTINATION)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    task.set_editor_property("options", options)
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    unreal.log_error("IMPORTCAP %s" % list(task.get_editor_property("imported_object_paths")))
