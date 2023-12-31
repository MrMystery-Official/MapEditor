
# Map Editor

A  map editor for The Legend of Zelda: Tears of the Kingdom
## Installing
First of all, download the latest version of the editor from the Releases tab. Extract the archive and run the executable. You will need a RomFS dump as well as a folder containing all .bfres (not .bfres.mc) models. To decompress the models, use this tool: https://gamebanana.com/tools/13236. The tool will produce two directories called "mc_output0" and "mc_output1". Copy all files from "mc_output1" into "mc_output0", and then pass the path to "mc_output0" to the editor when it asks for it.
## Hash Transformers
Currently, there is no way to create entirely new hashes for our actors. If you still want to use actors that utilize physics or AI, you must transform existing hashes. The button for that is located in the Tools tab of the editor. There are two options:

- Map model merged actor: Use this when you are editing a small dungeon that is not a blessing shrine. This method does not work in MainField.
- Local actor: Use this when you are editing a MainField section. After the transformer has run, the editor will close. Simply reopen it, and on the left side, where the label "Available hashes" is located, you can see how many hashes are remaining.

If you want to enable physics for an actor and you have already transformed hashes, simply click on the actor and press the "Enable physics" button. When duplicating an actor with physics enabled, it automatically assigns a new hash to the clone.
## Mod Merging
Let's say you have a mod that adds custom weapons to the game and you want to place them in the MainField using the editor. To do this, you need to go into WorkingDir/Save and paste the mods RomFS overwrites there. Again, the editor cannot read .bfres.mc files. You must decompress them using Switch Toolbox into WorkingDir/EditorModels.
## Collisions
The editor supports basic collisions. To add a box collision, add an actor called "MapEditor_Collision_Cube". You can then place it wherever you like and set whether it is climbable or not in the Transform section of the actor. If you want to use complex phive-shape collisions, use the "MapEditor_Collision_File" actor. There you can set the collision file. To check if an actor has such a shape, press the "Add collision" button in the Actor tab. Currently, polytopes are not implemented, but they will be soon!
## SmallDungeon StartPos
If you want to move the WarpIn actor inside of a small dungeon, it has no collision and the starting position will not be aligned with the position of the WarpIn actor. To set the correct entry position, click the "StarPos to WarpIn" button on the left. Don't forget to click on the "Add collision" button to add collision to the WarpIn actor.
