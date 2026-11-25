# Renames a Fantasy Kingdom pack skeleton's bones from Synty's OLDER rig
# vocabulary to the CURRENT one the animation packs use, so clips from the
# animation packs bind natively. The same mapping the web pipeline applies in
# Blender. One-time per skeleton; the pack ships no clips of its own.
#
#   UnrealEditor-Cmd <project> -ExecutePythonScript=<this file> -nullrhi -unattended -NoSound
import unreal

MESH = "/Game/PolygonFantasyKingdom/Meshes/Chr/SK_Chr_Soldier_Male_01"

MAPPING = {
    "Pelvis": "Hips", "spine_01": "Spine_01", "spine_02": "Spine_02", "spine_03": "Spine_03",
    "neck_01": "Neck", "head": "Head", "eyes": "Eyes", "eyebrows": "Eyebrows",
    "clavicle_l": "Clavicle_L", "clavicle_r": "Clavicle_R",
    "UpperArm_L": "Shoulder_L", "UpperArm_R": "Shoulder_R",
    "lowerarm_l": "Elbow_L", "lowerarm_r": "Elbow_R",
    # fingers: the current rig separates hands with a _1 suffix; which side is
    # which is the guess this run tests (left plain, right _1).
    "thumb_01_l": "Thumb_01", "thumb_02_l": "Thumb_02", "thumb_03_l": "Thumb_03",
    "thumb_01_r": "Thumb_01_1", "thumb_02_r": "Thumb_02_1", "thumb_03_r": "Thumb_03_1",
    "indexFinger_01_l": "IndexFinger_01", "indexFinger_02_l": "IndexFinger_02",
    "indexFinger_03_l": "IndexFinger_03", "indexFinger_04_l": "IndexFinger_04",
    "indexFinger_01_r": "IndexFinger_01_1", "indexFinger_02_r": "IndexFinger_02_1",
    "indexFinger_03_r": "IndexFinger_03_1", "indexFinger_04_r": "IndexFinger_04_1",
    "finger_01_l": "Finger_01", "finger_02_l": "Finger_02", "finger_03_l": "Finger_03", "finger_04_l": "Finger_04",
    "finger_01_r": "Finger_01_1", "finger_02_r": "Finger_02_1", "finger_03_r": "Finger_03_1", "finger_04_r": "Finger_04_1",
    "Thigh_L": "UpperLeg_L", "Thigh_R": "UpperLeg_R",
    "calf_l": "LowerLeg_L", "calf_r": "LowerLeg_R",
    "Foot_L": "Ankle_L", "Foot_R": "Ankle_R",
    "ball_l": "Ball_L", "ball_r": "Ball_R",
    "toes_l": "Toes_L", "toes_r": "Toes_R",
    # Root, Hand_L, Hand_R already match; the ik_* bones exist only on the old rig.
}

mesh = unreal.EditorAssetLibrary.load_asset(MESH)
if mesh is None:
    raise RuntimeError("no such mesh: " + MESH)
modifier = unreal.SkeletonModifier(name="synty_rig_rename", outer=mesh)
modifier.set_skeletal_mesh(mesh)
before = [str(b) for b in modifier.get_all_bone_names()]
unreal.log_error("RENAME before: %d bones" % len(before))
try:
    result = modifier.rename_bones(list(MAPPING.keys()), list(MAPPING.values()))
    unreal.log_error("RENAME rename_bones returned: %s" % result)
except Exception as error:
    unreal.log_error("RENAME rename_bones failed: %s" % str(error)[:300])
committed = modifier.commit_skeleton_to_skeletal_mesh()
after = [str(b) for b in modifier.get_all_bone_names()]
unreal.log_error("RENAME commit: %s; after: %d bones: %s" % (committed, len(after), after))
for path in (MESH, "/Game/PolygonFantasyKingdom/Meshes/Chr/SKEL_Chr_Soldier_Male_01"):
    unreal.log_error("RENAME saved %s: %s" % (path, unreal.EditorAssetLibrary.save_asset(path)))
