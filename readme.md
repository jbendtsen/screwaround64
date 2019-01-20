# Screwaround64
**_An Interactive Assembler For N64_**

Screwaround64 is a Windows program that assembles/disassembles your code as you type it.

There are two text editing windows inside the main window. The one on the left is for assembly, and the one on the right is for binary code.
Each time you move to a different row, the row you just left gets assembled/disassembled, depending on the window you were in.

At this time, this project is more of a prototype. Upcoming features are:

* Transfer binary code directly to a running emulator
	- While a game is playing, the code can be placed in an arbitrary location inside the emulated N64 RAM
* Error handling
	- Any time an instruction fails to assemble/disassemble, an informative message will be added to the bottom of the window
	- If there are any errors, the code won't be able to be transferred/exported
* Multiple tabs
	- Each tab will have its own N64 RAM offset, so that when the code is exported each tab will be distributed to a different location in RAM
* Project management
	- Assembly configuration, symbols, comments, metadata, tabs, etc.
* Export as
	- GameShark code, bytecode w/ various formatting options
	- Embed export in a file or create a new file