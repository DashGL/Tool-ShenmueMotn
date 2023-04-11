/*
	This file is part of Shenmue Motion
	Copyright 2023 kion@dashgl.com Benjamin Collins

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	uint32_t tableOfs;
	uint32_t nameOfs;
	uint32_t dataOfs;
	uint32_t animCount;
	uint32_t byteLength;
} MotnFileHeader_t;

typedef struct {
	uint32_t animOfs;
	uint32_t namePtr;
	char name[0x40];
} AnimLocation_t;

typedef struct {
	uint16_t frameCount;
	uint16_t block1Ofs;
	uint16_t block2Ofs;
	uint16_t block3Ofs;
	uint16_t block4Ofs;
	uint16_t block5Ofs;
} MotnAnimHeader_t;


void parseAnimation(FILE *fp, uint32_t animOfs);
float decodeFloat16(uint16_t binary);

int main() {

	// Define Variables

	FILE *fp;
	uint32_t namePtr;
	AnimLocation_t* animList;
	MotnFileHeader_t header;

	// Option Motion file

	fp = fopen("../data/motion.bin","r");

	// Read the file header
	fread(&header, sizeof(MotnFileHeader_t), 1, fp);
	header.animCount-=1;

	printf("File Header\n");
	printf("Table: 0x%x\n", header.tableOfs);
	printf("Name: 0x%x\n", header.nameOfs);
	printf("Data: 0x%x\n", header.dataOfs);
	printf("Count: 0x%x\n", header.animCount);
	printf("Length: 0x%x\n", header.byteLength);

	// Allocate the animation list to the number of animations
	animList = malloc(header.animCount * sizeof(AnimLocation_t));

	// First read the offset to the data for each animation
	fseek(fp, header.tableOfs, SEEK_SET);
	for (int i = 0; i < header.animCount; i++) {
		fread(&animList[i].animOfs, sizeof(uint32_t), 1, fp);
		animList[i].animOfs += header.dataOfs;
		fseek(fp, sizeof(uint32_t), SEEK_CUR);
	}

	// Read the name for each animation
	fseek(fp, header.nameOfs, SEEK_SET);
	for (int i = 0; i < header.animCount; i++) {
		fread(&animList[i].namePtr, sizeof(uint32_t), 1, fp);
	}
	
	for (int i = 0; i < header.animCount; i++) {
		fseek(fp, animList[i].namePtr, SEEK_SET);
		fread(&animList[i].name, 0x40, sizeof(char), fp);
	}

	// Read an animation and convert it to JSON
	printf("%s\n", animList[242].name);
	parseAnimation(fp, animList[242].animOfs);

	// Free memory and close Motion File
	fclose(fp);
	
	free(animList);

}

void parseAnimation(FILE *fp, uint32_t animOfs) {

	MotnAnimHeader_t header;

	fseek(fp, animOfs, SEEK_SET);
	fread(&header, sizeof(MotnAnimHeader_t), 1, fp);
	header.block1Ofs = 0x0c;

	printf("File Header\n");
	printf("Frames: 0x%x\n", header.frameCount);
	printf("Block 1 Ofs: 0x%x\n", header.block1Ofs);
	printf("Block 2 Ofs: 0x%x\n", header.block2Ofs);
	printf("Block 3 Ofs: 0x%x\n", header.block3Ofs);
	printf("Block 4 Ofs: 0x%x\n", header.block4Ofs);
	printf("Block 5 Ofs: 0x%x\n", header.block5Ofs);
	
	// Block 1

	uint16_t block1Len = header.block2Ofs - header.block1Ofs;
	uint32_t block1Ofs = animOfs + header.block1Ofs;
	fseek(fp, block1Ofs, SEEK_SET);

	printf("Block 1 Length: 0x%x\n", block1Len);
	printf("Block 1 Offset: 0x%x\n", block1Ofs);

	for(int i = 0; i < block1Len; i+= 2) {

	}

}
