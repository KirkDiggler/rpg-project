# Imports animation FBX from a Synty animation pack NATIVELY (the POLYGON-rig
# clips: A_POLY_/A_ names, not A_MOD_, which are for the newer modular rig) onto a stock
# character's skeleton from the Unreal-format pack: no Blender, no bridge.
#
#   UnrealEditor-Cmd <project> -ExecutePythonScript=<this file> -nullrhi -unattended -NoSound
#
# Clips only (import_mesh off); the skeleton is the pack's own.
import os, unreal

ROOT = "/home/kirk/UnrealEngine/Projects/RpgUeProof/AssetLab/native-anim"
SKELETON = "/Game/PolygonFantasyKingdom/Meshes/Chr/SKEL_Chr_Soldier_Male_01"
DESTINATION = "/Game/NativeAnim"

skeleton = unreal.EditorAssetLibrary.load_asset(SKELETON)
if skeleton is None:
    raise RuntimeError("the pack's soldier skeleton did not load: " + SKELETON)

tasks = []
for name in ["A_POLY_IDL_Base_Masc", "A_Walk_F_Masc", "A_Attack_HeavyStab01_Sword"]:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(ROOT, name + ".fbx"))
    task.set_editor_property("destination_path", DESTINATION)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", False)
    options.set_editor_property("import_animations", True)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    options.set_editor_property("skeleton", skeleton)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("create_physics_asset", False)
    options.set_editor_property("automated_import_should_detect_type", False)
    task.set_editor_property("options", options)
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    unreal.log_error("NATIVEANIM %s" % list(task.get_editor_property("imported_object_paths")))
