import unreal
L = unreal.EditorAssetLibrary
pack = L.load_asset("/Game/PolygonFantasyKingdom/Meshes/Chr/SKEL_Chr_Soldier_Male_01")
pack_bones = [str(b) for b in unreal.SkeletonModifier("m", None).get_all_bone_names()] if False else None
for asset in sorted(L.list_assets("/Game/NativeAnim", recursive=False, include_folder=False)):
    seq = L.load_asset(asset)
    kind = seq.get_class().get_name() if seq else "FAILED TO LOAD"
    line = "NATIVECENSUS %-40s %s" % (asset.split(".")[-1], kind)
    try:
        skel = seq.get_editor_property("skeleton")
        line += " skeleton=%s (the pack's soldier: %s)" % (skel.get_name() if skel else None, "yes" if skel and skel.get_name() == "SKEL_Chr_Soldier_Male_01" else "NO")
        line += " frames=%d length=%.2fs" % (seq.get_editor_property("number_of_sampled_frames"), seq.get_editor_property("sequence_length"))
        tracks = [str(t) for t in unreal.AnimationLibrary.get_animation_track_names(seq)]
        line += " tracks=%d first=%s" % (len(tracks), tracks[:4])
    except Exception as e:
        line += " (%s)" % str(e)[:100]
    unreal.log_error(line)
