# RTS units: setup

Five classes that give you an RTS camera, click/box selection, and units that walk where you right-click.

## How the pieces fit

- **ARTSGameMode** picks the classes below. Nothing else needs configuring in code.
- **ARTSCameraPawn** is what "the player" is: a root that slides across the map, a spring arm (tilt + zoom distance) and a camera. It has no input of its own.
- **ARTSPlayerController** reads the mouse and keyboard. It pans/zooms the camera pawn, keeps the list of selected units, and gives them move orders.
- **ARTSHUD** only draws the green drag box, asking the controller where it is.
- **ARTSUnit** is a Character (capsule + CharacterMovement) possessed by a plain AIController. A move order is `AIController->MoveToLocation`, which pathfinds on the NavMesh. That is why the map needs a NavMesh.

Controls: left click selects, left drag box-selects, Shift adds, right click moves, WASD/arrows or screen edge pans, wheel zooms.

## 1. Drop in the code

1. Close the Unreal Editor.
2. Copy the `Source/RTSGame/RTS` folder into your project so you get `G:/Dev/Unreal Engine/RTSGame/Source/RTSGame/RTS/*.h/.cpp`.
3. Open `Source/RTSGame/RTSGame.Build.cs` and make sure the dependency list contains `"AIModule"` and `"NavigationSystem"` (the Top-Down template usually already has both). For example:
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] {
       "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
       "AIModule", "NavigationSystem"
   });
   ```
4. Right-click `RTSGame.uproject` > **Generate Visual Studio project files**.
5. Open `RTSGame.sln`, set configuration **Development Editor / Win64**, build, then launch the editor (F5 or double-click the .uproject).

New classes need a full build with the editor closed; Live Coding won't pick them up reliably.

If your module isn't called `RTSGame`, replace `RTSGAME_API` in the five headers with `YOURMODULE_API`.

## 2. Make a map

1. **File > New Level > Basic** and save it (e.g. `Content/Maps/L_RTSTest`).
2. Scale the floor up (e.g. scale 20, 20, 1) so there is room to move.
3. **Place Actors > Volumes > Nav Mesh Bounds Volume**, drag it in and scale it to cover the whole floor (and a bit above/below it). Press **P** in the viewport: walkable ground turns green. No green, no movement.
4. Make sure there is a **Player Start** (the Basic level has one). The camera spawns there.
5. In the Content Browser, enable **Settings > Show C++ Classes**, open `C++ Classes/RTSGame/RTS`, and drag **RTSUnit** into the level 4 to 6 times. They appear as grey cylinders.
6. **Window > World Settings > GameMode Override = RTSGameMode.** (Or set it project-wide in Project Settings > Maps & Modes, and set this level as the Editor Startup Map / Game Default Map.)
7. Press Play. Click the viewport once so it has focus, then select and right-click.

## Optional: a real character mesh

Right-click `RTSUnit` > **Create Blueprint class based on RTSUnit** (e.g. `BP_Unit`). In it, select **Mesh**, set Skeletal Mesh to the template's `SKM_Quinn_Simple` (or `SKM_Manny_Simple`), Anim Class to `ABP_Unarmed`, move it down to Z = -96 and rotate yaw -90. Then select **PlaceholderBody** and untick Visible. Place `BP_Unit` instead of `RTSUnit`.

## Troubleshooting

- **Units don't move:** no NavMesh under them (press P), or the destination is off the NavMesh.
- **Clicking does nothing:** GameMode Override isn't set, so you're still using the template's controller.
- **Click selects nothing but box select works:** something else blocks the Pawn channel in front of the unit, or the unit's capsule collision was changed.
- **Camera drifts on its own:** mouse is resting on a screen edge; set `bEdgeScroll` off on a Blueprint of the controller if that's annoying in the editor.
