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
	char name[0x40];
} AnimLocation_t;

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
	
	printf("File Header\n");
	printf("Table: 0x%x\n", header.tableOfs);
	printf("Name: 0x%x\n", header.nameOfs);
	printf("Data: 0x%x\n", header.dataOfs);
	printf("Count: 0x%x\n", header.animCount);
	printf("Length: 0x%x\n", header.byteLength);

	// Allocate the animation list to the number of animations
	animList = calloc(header.animCount, sizeof(AnimLocation_t));

	// First read the offset to the data for each animation
	fseek(fp, header.tableOfs, SEEK_SET);
	for (int i = 0; i < header.animCount; i++) {
		fread(&animList[i].animOfs, sizeof(uint32_t), 1, fp);
		animList[i].animOfs += header.dataOfs;
		fseek(fp, sizeof(uint32_t), SEEK_CUR);
	}

	// Read the name for each animation
	for (int i = 0; i < header.animCount - 1; i++) {
		fseek(fp, header.nameOfs, SEEK_SET);
		fread(&namePtr, sizeof(uint32_t), 1, fp);
		header.nameOfs+=4;
		
		fseek(fp, namePtr, SEEK_SET);
		int ch;
		int j = 0;
		do {
			ch = fgetc(fp);
			animList[i].name[j++] = ch;
		} while (ch != 0);

	}

	// Read an animation and convert it to JSON
	parseAnimation(fp, animList[242].animOfs);

	// Free memory and close Motion File
	free(animList);
	fclose(fp);

}

void parseAnimation(FILE *fp, uint32_t animOfs) {

	fseek(fp, animOfs, SEEK_SET);
	

}
