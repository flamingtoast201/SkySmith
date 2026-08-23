import unreal

# Locate the active open Control Rig in your editor window
open_rigs = unreal.ControlRigBlueprint.get_currently_open_rig_blueprints()

if not open_rigs or len(open_rigs) == 0:
    raise Exception("Error: No Control Rig asset is open! Please double-click CR_Wyvern_Saddle_Companion_IK first.")

# Safely extract the single asset item out of the python list array
blueprint = open_rigs[0]

# Establish the core engine sub-controllers
controller = blueprint.get_controller()
hierarchy = blueprint.hierarchy
hierarchy_controller = blueprint.get_hierarchy_controller()

# Target your explicit DEF spine bone tree layout array
spine_bones = [
    "DEF-spine_004", "DEF-spine_005", "DEF-spine_006", "DEF-spine_007",
    "DEF-spine_008", "DEF-spine_009", "DEF-spine_010", "DEF-spine_011", 
    "DEF-spine_012"
]

print("Generating ring-aligned DEF Spine Control Rig network...")

# Load the exact script struct objects required by add_unit_node
get_transform_struct = unreal.find_object(None, "/Script/ControlRig.RigUnit_GetTransform")
set_transform_struct = unreal.find_object(None, "/Script/ControlRig.RigUnit_SetTransform")

# Suspend graph refreshes to prevent viewport lag while generating nodes
blueprint.suspend_notifications(True)

try:
    prev_set_node_name = None

    for idx, bone_name in enumerate(spine_bones):
        # Build structural key reference tokens for verification passes
        bone_key = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=bone_name)
        
        if not hierarchy.contains(bone_key):
            continue
            
        control_name = bone_name.replace("DEF-", "CTRL_")
        control_key = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=control_name)
        
        if not hierarchy.contains(control_key):
            # Fetch the base bone transform matrix
            bone_transform = hierarchy.get_global_transform(bone_key, initial=True)

            # --- RING ALIGNMENT MATRIX ---
            # Apply a 90-degree Roll offset to rotate the box faces sideways, 
            # turning them into vertical rings that encircle the spine line.
            rotation_offset = unreal.Rotator(90.0, 0.0, 0.0) 
            bone_transform.rotation = bone_transform.rotation * rotation_offset.quaternion()
            # -----------------------------

            desc = unreal.RigControlSettings()
            desc.control_type = unreal.RigControlType.EULER_TRANSFORM
            desc.shape_name = "Box"
            desc.shape_color = unreal.LinearColor(1.0, 0.5, 0.0, 1.0) # Signature asset orange outline
            
            parent_control_key = unreal.RigElementKey()
            if idx > 0:
                prev_control_name = spine_bones[idx-1].replace("DEF-", "CTRL_")
                parent_control_key = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=prev_control_name)
            
            default_value = unreal.RigControlValue()
            actual_control_key = hierarchy_controller.add_control(
                control_name, 
                parent_control_key, 
                desc,
                default_value,
                setup_undo=True
            )
            
            # Save the new offset coordinates back into the base initial pose system
            hierarchy.set_global_transform(actual_control_key, bone_transform, initial=True)
            hierarchy.set_global_transform(actual_control_key, bone_transform, initial=False)

        # Spawn node networks safely
        get_node = controller.add_unit_node(get_transform_struct, "Execute", unreal.Vector2D(100, idx * 240))
        get_node_name = get_node.get_node_path()
        controller.set_pin_default_value(f"{get_node_name}.Item", f"(Type=Control,Name={control_name})")
        
        set_node = controller.add_unit_node(set_transform_struct, "Execute", unreal.Vector2D(550, idx * 240))
        set_node_name = set_node.get_node_path()
        controller.set_pin_default_value(f"{set_node_name}.Item", f"(Type=Bone,Name={bone_name})")
        
        # Connect yellow transform data links
        controller.add_link(f"{get_node_name}.Transform", f"{set_node_name}.Transform")
        
        # Chain white sequence execution paths down the spine line track loop string
        if prev_set_node_name:
            controller.add_link(f"{prev_set_node_name}.ExecuteContext", f"{set_node_name}.ExecuteContext")
            
        prev_set_node_name = set_node_name

    print("Ring generation complete! The controls are now standing upright like rings.")

finally:
    blueprint.suspend_notifications(False)
