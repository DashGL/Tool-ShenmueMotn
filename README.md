# Tool-ShenmueMotn

This repository contains tools for parsing motn files from Shenmue with Nodejs and C.
Implementing the animations is out of scope from this repository, which describes 
the format and how to read the values from the files. 

An implementation will be needed to confirm the interpretation of the format
described within this document. 

## C

```
$ git clone https://github.com/DashGL/Tool-ShenmueMotn.git
$ cd Tool-ShenmueMotn/c
$ gcc main.c -lm
$ ./a.out
```

## Nodejs

```
$ git clone https://github.com/DashGL/Tool-ShenmueMotn.git
$ cd Tool-ShenmueMotn/c
$ node main.js
```

# List of Bones

- Provide a list of bones

# Motion (.motn) Format

This section covers technical information about the .motn format used in Shenmue.
Animations in Shenmue are encoded with Inverse Kinematics(IK), which uses a target to 
define where a limb of a character should be. The game engine runs through several
calculations to find the rotation needed for the limb to get as close to the target
as possivle per-frame.

The game effectively defines the animations a list of F-curves for position
and rotation per bone, per axis. Position values are used to define where targets
are to be located for key frames in the animation. Rotation values provide
pre-rotation values for a limb, to get a desired angle for the limb in the
animation.

The binary is broken down into "blocks" in order to get to the values per keyframe.
This document will provide the steps for reading each of the blocks needed
to parse the values for the animation. 

## File Structure

The .motn file contains all of the animations in the game for Shenmue. The way
this works is that all of the characters share the same skeleton, and everything 
from cat walking to kicks and punches are included in this one large file. This
file is persisted in the game's memory.

```c
typedef struct {
	uint32_t tableOfs;
	uint32_t nameOfs;
	uint32_t dataOfs;
	uint32_t animCount;
	uint32_t byteLength;
} MotnFileHeader_t;
```

The first 0x14 bytes that make up the header are shown by the struct above. The first value to bring attention to is the `animCount`. This contains the number of animations in the file. The `dataOfs` provides a pointer to the start of the section of the file where the animation values can be parsed. The `tableOfs` contains a list of pointers relative to the start of the `dataOfs`. The number of pointers being provided by the `animCount`. 

![motn-file](https://user-images.githubusercontent.com/25621780/230980203-b36b5789-9537-4692-8167-55138042d41f.png)

Likewise the `nameOfs` provides an offset to where the animation names are defined. The start of the names section contains a list of points to the actual string values. All of the values except for the last one will be zero terminated. The last animation string name is terminated by the start of the animation section. The figure above shows an outline of the structure of the file.

## Header

```c
typedef struct {
	uint16_t animLength;
	uint16_t block1Ofs;
	uint16_t block2Ofs;
	uint16_t block3Ofs;
	uint16_t block4Ofs;
	uint16_t block5Ofs;
} MotnAnimHeader_t;
```

Each animation in the animation section has a small header with a length of 0x12 bytes. The first value is the length of the animation in frames. And the rest of the five values are the relative offsets to the different blocks from the start of the animation definition. With the exception of block 1, which is actually a fixed values of 0x12. 

![anim-offsets](https://user-images.githubusercontent.com/25621780/230983693-b034a5ac-b6b6-4f60-9304-6a5635260d3b.png)

The figure above shows this relationship visually. The file header contains the offset to the start of the animation section. The animOfs table, provides the relative start to a specific animation from the start of the animation section. And each animation provides offsets to the blocks inside the animation from the start of the animation definition. 

## Block 1

- Bone Id + axis

## Block 2

- Number of key frames

## Block 3

- Specific Key frames

## Block 4

- Easing Values

## Block 5

- Float values for easing
- FLoat value for key frame

# License

Copyright 2017, 2023 kion@dashgl.com MIT License
