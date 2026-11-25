# Builds the lab scene in the open level, waits for shaders and the idle poses,
# renders one frame through LabCamera, then quits. No level is saved: the
# scene exists for the shot. The editor must run with a real renderer:
#
#   UnrealEditor-Cmd <project> -RenderOffscreen -unattended -NoSound -ExecCmds="py <this>"
#
# One row of evidence, left to right:
#   the two characters that crossed the GLB -> FBX bridge, their idle playing;
#   a raw character straight from the Synty Unreal-format pack (no conversion) —
#   the very model the bridged fighter was built from;
#   behind them the bridged wall piece, under them the bridged rug;
#   at the right a raw prop from the Unreal pack.
import unreal

# unreal.Rotator's POSITIONAL order is (roll, pitch, yaw). Every rotation below
# uses keywords for that reason; a positional one here pointed the camera at the sky.

BUILD_AT = 6.0      # the editor is up and the asset registry has the imports
SHOT_AT = 30.0      # shaders compiled, poses evaluated
QUIT_AT = 40.0

assets = unreal.EditorAssetLibrary
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
state = {"t": 0.0, "built": False, "shot": False, "done": False}


def place(cls, path, x, y, z=0.0, yaw=0.0, scale=1.0):
    actor = eas.spawn_actor_from_class(cls, unreal.Vector(x, y, z), unreal.Rotator(pitch=0.0, yaw=yaw, roll=0.0))
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    asset = assets.load_asset(path)
    if asset is None:
        unreal.log_warning("LAB missing asset %s" % path)
    elif cls is unreal.StaticMeshActor:
        actor.static_mesh_component.set_static_mesh(asset)
    else:
        actor.skeletal_mesh_component.set_skeletal_mesh_asset(asset)
    unreal.log("LAB placed %-28s as %s" % (actor.get_name(), path))
    return actor


def play_idle(actor, anim_path):
    component = actor.skeletal_mesh_component
    anim = assets.load_asset(anim_path)
    component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    component.set_animation(anim)
    component.play(True)
    component.set_update_animation_in_editor(True)
    unreal.log("LAB %s plays %s" % (actor.get_name(), anim.get_name()))


def build():
    place(unreal.StaticMeshActor, "/Engine/BasicShapes/Plane", 0.0, 60.0, 0.0, 0.0, 40.0)
    sun = eas.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 500.0), unreal.Rotator(pitch=-48.0, yaw=32.0, roll=0.0))
    sun.light_component.set_intensity(6.0)
    sky_sphere = assets.load_asset("/Engine/EngineSky/BP_Sky_Sphere")
    if sky_sphere:
        eas.spawn_actor_from_object(sky_sphere, unreal.Vector(0.0, 0.0, 0.0))
    skylight = eas.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 300.0))
    skylight.light_component.set_intensity(1.0)

    place(unreal.StaticMeshActor, "/Game/WebBridge/Props/Crypt_Rug_01", 0.0, 60.0, 0.2)
    place(unreal.StaticMeshActor, "/Game/WebBridge/Env/Crypt_Wall_Base_01", 0.0, 260.0, 0.0, 180.0)
    fighter = place(unreal.SkeletalMeshActor, "/Game/WebBridge/Characters/fighter", -170.0, 60.0, yaw=180.0)
    soldier = place(unreal.SkeletalMeshActor, "/Game/WebBridge/Characters/skeleton-soldier-01", 0.0, 60.0, yaw=180.0)
    play_idle(fighter, "/Game/WebBridge/Characters/fighterRoot_Idle_Relaxed")
    play_idle(soldier, "/Game/WebBridge/Characters/skeleton-soldier-01Root_Idle_Relaxed")
    state["idles"] = [(fighter, "/Game/WebBridge/Characters/fighterRoot_Idle_Relaxed"),
                      (soldier, "/Game/WebBridge/Characters/skeleton-soldier-01Root_Idle_Relaxed")]
    place(unreal.SkeletalMeshActor, "/Game/PolygonFantasyKingdom/Meshes/Chr/SK_Chr_Peasant_Male_01", 170.0, 60.0, yaw=180.0)
    place(unreal.StaticMeshActor, "/Game/PolygonFantasyKingdom/Meshes/Props/SM_Prop_Barrel_01", 340.0, 30.0)

    camera = eas.spawn_actor_from_class(unreal.CameraActor, unreal.Vector(80.0, -620.0, 230.0), unreal.Rotator(pitch=-9.0, yaw=90.0, roll=0.0))
    camera.set_actor_label("LabCamera")
    camera.camera_component.set_field_of_view(42.0)
    unreal.log("LAB built: %d actors in %s" % (len(eas.get_all_level_actors()), unreal.EditorLevelLibrary.get_editor_world().get_name()))


def tick(dt):
    state["t"] += dt
    if not state["built"] and state["t"] >= BUILD_AT:
        state["built"] = True
        try:
            build()
        except Exception as error:
            unreal.log_error("LAB build failed: %s" % error)
    elif state["built"] and not state["shot"] and state["t"] >= SHOT_AT:
        state["shot"] = True
        # Re-assert each idle and report what the component says it is doing.
        for actor, anim_path in state.get("idles", []):
            component = actor.skeletal_mesh_component
            report = "LAB %-24s before: playing=%s" % (actor.get_actor_label(), component.is_playing())
            component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
            component.set_animation(assets.load_asset(anim_path))
            component.set_position(0.8, False)
            component.play(True)
            component.set_update_animation_in_editor(True)
            unreal.log(report + "  mode=%s mesh=%s" % (component.get_editor_property("animation_mode"),
                                                      component.skeletal_mesh_asset.get_name() if component.skeletal_mesh_asset else None))
        camera = None
        for actor in eas.get_all_level_actors():
            if actor.get_actor_label() == "LabCamera":
                camera = actor
        unreal.log("LAB shot at t=%.1f from %s" % (state["t"], camera.get_name() if camera else "the viewport"))
        # One request only: a second would replace the first before it is written.
        unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "AssetLab", camera)
    elif state["shot"] and not state["done"] and state["t"] >= QUIT_AT:
        state["done"] = True
        unreal.log("LAB quitting")
        unreal.SystemLibrary.quit_editor()


unreal.register_slate_post_tick_callback(tick)
