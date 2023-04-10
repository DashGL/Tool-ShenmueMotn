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

## Name Table

The .motn file contains all of the animations in the game for Shenmue. The way
this works is that all of the characters share the same skeleton. And all of
the animations are loaded into one motion file that gets loaed into memory
and used through out all of the gameplay. 

There are more than 300 animations, and everything from cat walking to 
kicks and punches are included in this one large file.

```
typedef struct {
	uint32_t tableOfs;
	uint32_t nameOfs;
	uint32_t dataOfs;
	uint32_t animCount;
	uint32_t byteLength;
} MotnFileHeader_t;
```


## Header

- Length of each block

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
