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
#include <math.h>

// Struct Defitnitions

typedef struct {
	uint32_t dataOfs;
	char name[256];
} SeqList;

typedef struct {
	uint16_t no;
	bool high;
	bool low;
	float easing[2];
	float value;
} KeyFrame_t;

typedef struct {
	uint16_t bone_id;
	char type[4];
	char axis[4];
	bool isRot;
	uint16_t keyFrameCount;
	KeyFrame_t* keyFrames;
} Block_t;

// Const Definitions

#define POS_MASK 0x1c0
#define ROT_MASK 0x38

#define POS_X 0x100
#define POS_Y 0x80
#define POS_Z 0x40
#define ROT_X 0x20
#define ROT_Y 0x10
#define ROT_Z 0x08

SeqList *seq_list;
void readMotion(FILE *fp, uint32_t tableOfs, uint32_t nameOfs, uint32_t count);
void parseAnimation(FILE *fp, uint32_t animOfs);
float Float16ToFloat(uint16_t fltInt16);

int main() {
	
	// Open File
	FILE *fp;
	fp = fopen("motion.bin","rb");

	// Read Header from Motion file

	uint32_t tableOfs;
	uint32_t nameOfs;
	uint32_t dataOfs;
	uint32_t count;
	uint32_t filelength;

	fread(&tableOfs, sizeof(uint32_t), 1, fp);
	fread(&nameOfs, sizeof(uint32_t), 1, fp);
	fread(&dataOfs, sizeof(uint32_t), 1, fp);
	fread(&count, sizeof(uint32_t), 1, fp);
	fread(&filelength, sizeof(uint32_t), 1, fp);
	
	// Allocate Sequence List
	seq_list = calloc(count - 1, sizeof(SeqList));
	readMotion(fp, tableOfs, nameOfs, count);
	
	// printf("Read %s\n", seq_list[242].name);
	uint32_t animOfs = seq_list[242].dataOfs + dataOfs;
	parseAnimation(fp, animOfs);

	fclose(fp);
	free(seq_list);

}

void readMotion(FILE *fp, uint32_t tableOfs, uint32_t nameOfs, uint32_t count) {

	uint32_t name_ptr;
	fseek(fp, tableOfs, SEEK_SET);
	for (int i = 0; i < count - 1; i++) {
		SeqList *seq = &seq_list[i];
		fread(&seq->dataOfs, sizeof(uint32_t), 1, fp);
		fseek(fp, sizeof(uint32_t), SEEK_CUR);
	}
	
	for (int i = 0; i < count - 1; i++) {
		SeqList *seq = &seq_list[i];
		fseek(fp, nameOfs, SEEK_SET);
		nameOfs += 4;
		fread(&name_ptr, sizeof(uint32_t), 1, fp);
		
		fseek(fp, name_ptr, SEEK_SET);
		int ch;
		int j = 0;
		do {
			ch = fgetc(fp);
			if (ch != 0) {
				seq->name[j++] = ch;
			}
		} while (ch != 0);

		char num[5];
		snprintf(num, 5, "%04d", i);
	}

}

void parseAnimation(FILE *fp, uint32_t animOfs) {

	// Read the header
	
	uint8_t size8;
	uint16_t length, instruction, size16;
	uint16_t block1Ofs, block2Ofs, block3Ofs, block4Ofs, block5Ofs;

	fseek(fp, animOfs, SEEK_SET);
	fread(&length, sizeof(uint16_t), 1, fp);
	fread(&block1Ofs, sizeof(uint16_t), 1, fp);
	fread(&block2Ofs, sizeof(uint16_t), 1, fp);
	fread(&block3Ofs, sizeof(uint16_t), 1, fp);
	fread(&block4Ofs, sizeof(uint16_t), 1, fp);
	fread(&block5Ofs, sizeof(uint16_t), 1, fp);

	// Block 1

	Block_t blocks[255];
	fseek(fp, animOfs + 0x0c, SEEK_SET);
	// printf("--- Block 1 ---\n");

	uint16_t blocks_len = 0;
	do {
		
		fread(&instruction, sizeof(uint16_t), 1, fp);
		if(instruction == 0) {
			break;
		}

		uint16_t bone_id = instruction >> 9;

		if(instruction & POS_X) {
			blocks[blocks_len].bone_id = bone_id;
			blocks[blocks_len].isRot = false;
			strcpy(blocks[blocks_len].type, "pos\0");
			strcpy(blocks[blocks_len].axis, "x\0");
			blocks_len++;
		}

		if(instruction & POS_Y) {
			blocks[blocks_len].bone_id = bone_id;
			blocks[blocks_len].isRot = false;
			strcpy(blocks[blocks_len].type, "pos\0");
			strcpy(blocks[blocks_len].axis, "y\0");
			blocks_len++;
		}

		if(instruction & POS_Z) {
			blocks[blocks_len].bone_id = bone_id;
			blocks[blocks_len].isRot = false;
			strcpy(blocks[blocks_len].type, "pos\0");
			strcpy(blocks[blocks_len].axis, "z\0");
			blocks_len++;
		}

		if(instruction & ROT_X) {
			blocks[blocks_len].bone_id = bone_id;
			blocks[blocks_len].isRot = true;
			strcpy(blocks[blocks_len].type, "rot\0");
			strcpy(blocks[blocks_len].axis, "x\0");
			blocks_len++;
		}

		if(instruction & ROT_Y) {
			blocks[blocks_len].bone_id = bone_id;
			blocks[blocks_len].isRot = true;
			strcpy(blocks[blocks_len].type, "rot\0");
			strcpy(blocks[blocks_len].axis, "y\0");
			blocks_len++;
		}

		if(instruction & ROT_Z) {
			blocks[blocks_len].bone_id = bone_id;
			blocks[blocks_len].isRot = true;
			strcpy(blocks[blocks_len].type, "rot\0");
			strcpy(blocks[blocks_len].axis, "z\0");
			blocks_len++;
		}

	} while(instruction != 0);
	

	// Block 2
	

	fseek(fp, animOfs + block2Ofs, SEEK_SET);
	bool block2EntryHalfSize = length & 0xFFFF8000 ? false : true;
	// printf("--- Block 2 ---\n");

	for(int i = 0; i < blocks_len; i++) {
		
		if(block2EntryHalfSize) {
			fread(&size8, sizeof(uint8_t), 1, fp);
			blocks[i].keyFrameCount = (uint16_t)size8;
		} else {
			fread(&size16, sizeof(uint16_t), 1, fp);
			blocks[i].keyFrameCount = size16;
		}

		// First and last frames are implied
		blocks[i].keyFrameCount += 2;
		uint32_t keyFrameSize = blocks[i].keyFrameCount * sizeof(KeyFrame_t);
		blocks[i].keyFrames = malloc(keyFrameSize);

		// printf("%d\n", blocks[i].size);

	}

	// Block 3

	// printf("--- Block 3 ---\n");

	fseek(fp, animOfs + block3Ofs, SEEK_SET);
	bool block3EntryHalfSize = (length & 0x7FFF) <= 0xFF ? true : false;
	
	for(int i = 0; i < blocks_len; i++) {
		
		for(int k = 0; k < blocks[i].keyFrameCount - 1; k++) {
			
			if(k == 0) {
				blocks[i].keyFrames[0].no = 0;
				// printf("0,");
				continue;
			}

			if(block3EntryHalfSize) {
				fread(&size8, sizeof(uint8_t), 1, fp);
				blocks[i].keyFrames[k].no = size8;
			} else {
				fread(&size16, sizeof(uint16_t), 1, fp);
				blocks[i].keyFrames[k].no = size16;
			}
		
			// printf("%d,", blocks[i].keyFrames[k + 1]);

		}
		
		blocks[i].keyFrames[blocks[i].keyFrameCount - 1].no = length - 1;
		// printf("%d\n", length - 1);

	}

	// Block 4
	
	// printf("--- Block 4 ---\n");
	fseek(fp, animOfs + block4Ofs, SEEK_SET);
	
	FILE *b4;
	b4 = fopen ("test/block4.txt", "w");

	for(int i = 0; i < blocks_len; i++) {
		
		uint32_t index = 0;

		do {
			fread(&size8, sizeof(uint8_t), 1, fp);

			blocks[i].keyFrames[index].high = size8 & 0x80 ? true : false;
			blocks[i].keyFrames[index].low = size8 & 0x40 ? true : false;
		
			if(blocks[i].keyFrames[index].high) {
				fprintf(b4, "%d, %s, %s, %d, h\n", 
					blocks[i].bone_id, 
					blocks[i].axis,
					blocks[i].type,
					index
				);
			}

			if(blocks[i].keyFrames[index].low) {
				fprintf(b4, "%d, %s, %s, %d, l\n", 
					blocks[i].bone_id, 
					blocks[i].axis,
					blocks[i].type,
					index
				);
			}
			index++;

			blocks[i].keyFrames[index].high = size8 & 0x20 ? true : false;
			blocks[i].keyFrames[index].low = size8 & 0x10 ? true : false;
			if(blocks[i].keyFrames[index].high) {
				fprintf(b4, "%d, %s, %s, %d, h\n", 
					blocks[i].bone_id, 
					blocks[i].axis,
					blocks[i].type,
					index
				);
			}

			if(blocks[i].keyFrames[index].low) {
				fprintf(b4, "%d, %s, %s, %d, l\n", 
					blocks[i].bone_id, 
					blocks[i].axis,
					blocks[i].type,
					index
				);
			}
			index++;

			blocks[i].keyFrames[index].high = size8 & 0x08 ? true : false;
			blocks[i].keyFrames[index].low = size8 & 0x04 ? true : false;
			if(blocks[i].keyFrames[index].high) {
				fprintf(b4, "%d, %s, %s, %d, h\n", 
					blocks[i].bone_id, 
					blocks[i].axis,
					blocks[i].type,
					index
				);
			}

			if(blocks[i].keyFrames[index].low) {
				fprintf(b4, "%d, %s, %s, %d, l\n", 
					blocks[i].bone_id, 
					blocks[i].axis,
					blocks[i].type,
					index
				);
			}
			index++;

			blocks[i].keyFrames[index].high = size8 & 0x02 ? true : false;
			blocks[i].keyFrames[index].low = size8 & 0x01 ? true : false;
			if(blocks[i].keyFrames[index].high) {
				fprintf(b4, "%d, %s, %s, %d, h\n", 
					blocks[i].bone_id, 
					blocks[i].axis,
					blocks[i].type,
					index
				);
			}

			if(blocks[i].keyFrames[index].low) {
				fprintf(b4, "%d, %s, %s, %d, l\n", 
					blocks[i].bone_id, 
					blocks[i].axis,
					blocks[i].type,
					index
				);
			}
			index++;

		} while(index < blocks[i].keyFrameCount);
		
	}

	fclose(b4);

	// Block 5
	
	// printf("--- Block 5 ---\n");
	fseek(fp, animOfs + block5Ofs, SEEK_SET);

	for(int i = 0; i < blocks_len; i++) {
		
		for(int k = 0; k < blocks[i].keyFrameCount; k++) {

			blocks[i].keyFrames[k].easing[0] = 0.0f;
			blocks[i].keyFrames[k].easing[1] = 0.0f;

			if(blocks[i].keyFrames[k].high && blocks[i].keyFrames[k].low) {
				fread(&size16, sizeof(uint16_t), 1, fp);
				blocks[i].keyFrames[k].easing[0] = Float16ToFloat(size16);

				fread(&size16, sizeof(uint16_t), 1, fp);
				blocks[i].keyFrames[k].easing[1] = Float16ToFloat(size16);

				fread(&size16, sizeof(uint16_t), 1, fp);
				blocks[i].keyFrames[k].value = Float16ToFloat(size16);
			
				if(blocks[i].isRot) {
					float c = blocks[i].keyFrames[k].value;
					c = c * 65536.0f;
					blocks[i].keyFrames[k].value = fmodf(c, 3.1415926f*2.0f);
				}

			} else if(blocks[i].keyFrames[k].high) {
				fread(&size16, sizeof(uint16_t), 1, fp);
				blocks[i].keyFrames[k].easing[0] = Float16ToFloat(size16);

				fread(&size16, sizeof(uint16_t), 1, fp);
				blocks[i].keyFrames[k].value = Float16ToFloat(size16);
			
				if(blocks[i].isRot) {
					float c = blocks[i].keyFrames[k].value;
					c = c * 65536.0f;
					blocks[i].keyFrames[k].value = fmodf(c, 3.1415926f*2.0f);
				}

			} else if(blocks[i].keyFrames[k].low) {
				fread(&size16, sizeof(uint16_t), 1, fp);
				blocks[i].keyFrames[k].easing[1] = Float16ToFloat(size16);

				fread(&size16, sizeof(uint16_t), 1, fp);
				blocks[i].keyFrames[k].value = Float16ToFloat(size16);
			
				if(blocks[i].isRot) {
					float c = blocks[i].keyFrames[k].value;
					c = c * 65536.0f;
					blocks[i].keyFrames[k].value = fmodf(c, 3.1415926f*2.0f);
				}

			}

		}
		
	}

	// Print out values
	
	FILE *out;
	out = fopen ("test/out.json", "w");

	fprintf(out, "[\n");

	for(int i = 0; i < blocks_len; i++) {
		
		fprintf(out,"  {\n");
		
		fprintf(out,"    \"boneId\": %d,\n",blocks[i].bone_id);
		fprintf(out,"    \"type\": \"%s\",\n",blocks[i].type);
		fprintf(out,"    \"axis\": \"%s\",\n",blocks[i].axis);
		fprintf(out,"    \"frames\": [\n");
		
		for(int k = 0; k < blocks[i].keyFrameCount; k++) {
			fprintf(out,"      {\n");
			fprintf(out,"        \"frame\": %d,\n", blocks[i].keyFrames[k].no);

			bool high = blocks[i].keyFrames[k].high;
			bool low = blocks[i].keyFrames[k].low;
			float a = blocks[i].keyFrames[k].easing[0];
			float b = blocks[i].keyFrames[k].easing[1];

			if(high && low) {
				fprintf(out,"        \"easing\": [\n", a, b);
				fprintf(out,"          %f,\n", a);
				fprintf(out,"          %f\n", b);
				fprintf(out,"        ],\n");
			} else if(high) {
				fprintf(out,"        \"easing\": [\n", a, b);
				fprintf(out,"          %f\n", a);
				fprintf(out,"        ],\n");
			} else if(low) {
				fprintf(out,"        \"easing\": [\n", a, b);
				fprintf(out,"          %f\n", b);
				fprintf(out,"        ],\n");
			} else {
				fprintf(out,"        \"easing\": [],\n");
			}

			fprintf(out,"        \"value\":%f\n", blocks[i].keyFrames[k].value);
			
			if(k < blocks[i].keyFrameCount - 1) {
				fprintf(out,"      },\n");
			} else {
				fprintf(out,"      }\n");
			}
		}

		fprintf(out,"    ]\n");
		if(i < blocks_len - 1) {
			fprintf(out,"  },\n");
		} else {
			fprintf(out,"  }\n");
		}

	}


	fprintf(out,"]\n");
	fclose(out);

	// Clean up Memory

	for(int i = 0; i < blocks_len; i++) {
		free(blocks[i].keyFrames);
	}

}

float Float16ToFloat(uint16_t fltInt16) {

	int fltInt32 =  ((fltInt16 & 0x8000) << 16);
	fltInt32 |= ((fltInt16 & 0x7fff) << 13) + 0x38000000;

	float fRet;
	memcpy( &fRet, &fltInt32, sizeof( float ) );
	return fRet;

}
