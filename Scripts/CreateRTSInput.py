"""
Creates the Enhanced Input assets the RTS player controller expects, in /Game/RTS/Input:
  IA_Select (bool), IA_AddToSelection (bool), IA_Command (bool),
  IA_Pan (Axis2D), IA_Zoom (Axis1D), IA_RotateHold (bool), IA_Rotate (Axis2D),
  and the mapping context IMC_RTS.

Safe to re-run: existing assets are reused and the context's mappings are rewritten.

Run headless with the editor closed:
  UnrealEditor-Cmd.exe "<path>/RTSGame.uproject" -run=pythonscript -script="<path>/Scripts/CreateRTSInput.py" -unattended -nosplash -nop4
or from the editor: Tools > Execute Python Script.
"""
import unreal

FOLDER = "/Game/RTS/Input"
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary


def get_or_create(name, cls, factory):
    path = f"{FOLDER}/{name}"
    if eal.does_asset_exist(path):
        return eal.load_asset(path)
    return asset_tools.create_asset(name, FOLDER, cls, factory)


def make_action(name, value_type):
    action = get_or_create(name, unreal.InputAction, unreal.InputAction_Factory())
    action.set_editor_property("value_type", value_type)
    eal.save_loaded_asset(action)
    return action


def key(name):
    k = unreal.Key()
    k.set_editor_property("key_name", name)
    return k


VT = unreal.InputActionValueType
select = make_action("IA_Select", VT.BOOLEAN)
add_to_selection = make_action("IA_AddToSelection", VT.BOOLEAN)
command = make_action("IA_Command", VT.BOOLEAN)
pan = make_action("IA_Pan", VT.AXIS2D)
zoom = make_action("IA_Zoom", VT.AXIS1D)
rotate_hold = make_action("IA_RotateHold", VT.BOOLEAN)
rotate = make_action("IA_Rotate", VT.AXIS2D)

imc = get_or_create("IMC_RTS", unreal.InputMappingContext, unreal.InputMappingContext_Factory())


def negate():
    return unreal.new_object(unreal.InputModifierNegate, outer=imc)


def swizzle():
    # Default order YXZ: moves a key's 1D value from X into Y.
    return unreal.new_object(unreal.InputModifierSwizzleAxis, outer=imc)


def mapping(action, key_name, modifiers=()):
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property("action", action)
    m.set_editor_property("key", key(key_name))
    m.set_editor_property("modifiers", list(modifiers))
    return m


# IA_Pan: X = forward (up the screen), Y = right.
# A key press is 1.0 on X, so forward needs nothing, back is negated,
# right is swizzled into Y, left is swizzled and negated.
mappings = [
    mapping(select, "LeftMouseButton"),
    mapping(add_to_selection, "LeftShift"),
    mapping(add_to_selection, "RightShift"),
    mapping(command, "RightMouseButton"),
    mapping(zoom, "MouseWheelAxis"),
    mapping(rotate_hold, "MiddleMouseButton"),
    # Mouse2D is the mouse's X/Y movement each frame. Add a Negate modifier here to invert the rotation.
    mapping(rotate, "Mouse2D"),
]
# WASD was removed at Alski's request; arrow keys and screen-edge scrolling still pan.
for fwd, back, right, left in (("Up", "Down", "Right", "Left"),):
    mappings += [
        mapping(pan, fwd),
        mapping(pan, back, [negate()]),
        mapping(pan, right, [swizzle()]),
        mapping(pan, left, [swizzle(), negate()]),
    ]

# UE 5.8 keeps the list inside the "default_key_mappings" struct ("mappings" is deprecated);
# older engines only have "mappings".
try:
    data = imc.get_editor_property("default_key_mappings")
    data.set_editor_property("mappings", mappings)
    imc.set_editor_property("default_key_mappings", data)
except Exception:
    imc.set_editor_property("mappings", mappings)

eal.save_loaded_asset(imc)
unreal.log(f"RTS input assets ready in {FOLDER}: {len(mappings)} key mappings in IMC_RTS")
