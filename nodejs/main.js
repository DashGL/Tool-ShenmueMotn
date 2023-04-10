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

"use strict";

const { readFileSync } = require('fs')

const MEM = {}

const decodeFloat16 = (binary) => {

	const exponent = (binary & 0x7C00) >> 10;
	const fraction = binary & 0x03FF;

	return (binary >> 15 ? -1 : 1) * (
		exponent ?
		(
			exponent === 0x1F ?
			fraction ? NaN : Infinity :
			Math.pow(2, exponent - 15) * (1 + fraction / 0x400)
		) : 6.103515625e-5 * (fraction / 0x400)
	);

}

const parseAnimation = (animIndex) => {
	
	const anim = MEM.seq_list[animIndex];
	MEM.anim = anim;
	MEM.ofs = anim.dataOfs;

	// Read the header

	const motn = [];
	const length = MEM.view.getUint16(MEM.ofs + 0x00, true);
	const header = {
		block1Ofs : 0x0c,
		block2Ofs : MEM.view.getUint16(MEM.ofs + 0x04, true),
		block3Ofs : MEM.view.getUint16(MEM.ofs + 0x06, true),
		block4Ofs : MEM.view.getUint16(MEM.ofs + 0x08, true),
		block5Ofs : MEM.view.getUint16(MEM.ofs + 0x0a, true)
	}
		
	const block2EntryHalfSize = length & 0xFFFF8000 ? false : true;
	const block3EntryHalfSize = (length & 0x7FFF) <= 0xFF ? true : false;

	// Read Block 1
		
	const pos_mask = 0x1c0;
	const rot_mask = 0x38;

	let instruction;
	let axis_count = 0;
	let index = 0;
	MEM.ofs = anim.dataOfs + header.block1Ofs;

	console.log('Block 1 ofs: 0x%0s', MEM.ofs.toString(16));
		
	const b1 = []

	instruction = MEM.view.getUint16(MEM.ofs, true);
	while((index < 127) && instruction !== 0) {
		let motn_id = instruction >> 9;
		
		console.log(motn_id);

		if(instruction & pos_mask) {
			const node = {
				motn_id : motn_id,
				type : 'pos',
				axis : []
			}

			if(instruction & 0x100) {
				node.axis.push('x');
				axis_count++;
			}

			if(instruction & 0x80) {
				node.axis.push('y');
				axis_count++;
			}

			if(instruction & 0x40) {
				node.axis.push('z');
				axis_count++;
			}
				
			b1.push( `${motn_id} pos ${node.axis.join('')}` );
			motn.push(node);
		} 

		if(instruction & rot_mask) { 
			const node = {
				motn_id : motn_id,
				type : 'rot',
				axis : []
			}

			if(instruction & 0x20) {
				node.axis.push('x');
				axis_count++;
			}

			if(instruction & 0x10) {
				node.axis.push('y');
				axis_count++;
			}

			if(instruction & 0x08) {
				node.axis.push('z');
				axis_count++;
			}

			b1.push( `${motn_id} rot ${node.axis.join('')}` );
			motn.push(node);

		}
			
		MEM.ofs += 2;
		index++;
		instruction = MEM.view.getUint16(MEM.ofs, true);
		
	}

	console.log(b1.join('\n'));

	// Read Block 2

	let block2Size = header.block3Ofs - header.block2Ofs;
	// If block 2 is half size, these should be the same

	console.log('--- Block 2 ---');
	const b2 = [];

	MEM.ofs = anim.dataOfs + header.block2Ofs;

	motn.forEach(node => {
			
		node.axis.forEach( d => {
			console.log("Block 2 Offset 0x%s", MEM.ofs.toString(16));
			let size;
			if(block2EntryHalfSize) {
				size = MEM.view.getUint8(MEM.ofs);
				MEM.ofs++;
			} else {
				size = MEM.view.getUint16(MEM.ofs, true);
				MEM.ofs += 2;
			}
			b2.push(size);
			console.log(node.type, size);
			node[d] = new Array(size);
		});

	});

	console.log(b2.join('\n'));

	// Read Block 3
		
	let zero_count = 0;
	let total_frames = 0;

	let start_ofs = anim.dataOfs + header.block3Ofs;
	let end_ofs = anim.dataOfs + header.block4Ofs;

	for(let i = start_ofs; i < end_ofs; i++) {
		let byte = MEM.view.getUint8(i);
		if(i !== 0) {
			continue;
		}
		zero_count++;
	}

	const b3 = [];
	MEM.ofs = anim.dataOfs + header.block3Ofs;
	motn.forEach(node => {
			
		node.axis.forEach( d => {
				
			const line = [0];

			for(let i = 0; i < node[d].length; i++) {
					
				let no;
				if(block3EntryHalfSize) {
					no = MEM.view.getUint8(MEM.ofs);
					MEM.ofs++;
				} else {
					no = MEM.view.getUint16(MEM.ofs, true);
					MEM.ofs += 2;
				}

				node[d][i] = { frame : no };
				line.push(no);

			}
				
			node[d].unshift({frame:0});
			node[d].push({frame:length - 1});
			line.push(length - 1);
			total_frames += node[d].length;
			
			b3.push(line.join(','));
		});

	});

	console.log(b3.join('\n'));

	let block4Size = header.block5Ofs - header.block4Ofs;

	// Read Block 4

	MEM.ofs = anim.dataOfs + header.block4Ofs;
		
	const b4 = []
	motn.forEach(node => {
			
		const { motn_id, axis, type } = node;

		node.axis.forEach( d => {
				
			let index = 0;

			do {

				let keyframeBlockType = MEM.view.getUint8(MEM.ofs);
				MEM.ofs++;

				if(keyframeBlockType & 0x80) {
					node[d][index].high = true;
					b4.push(`${motn_id}, ${d}, ${type}, ${index}, h`);
				}

				if(keyframeBlockType & 0x40) {
					node[d][index].low = true;
					b4.push(`${motn_id}, ${d}, ${type}, ${index}, l`);
				}
				index++;

				if(keyframeBlockType & 0x20) {
					node[d][index].high = true;
					b4.push(`${motn_id}, ${d}, ${type}, ${index}, h`);
				}

				if(keyframeBlockType & 0x10) {
					node[d][index].low = true;
					b4.push(`${motn_id}, ${d}, ${type}, ${index}, l`);
				}
				index++;

				if(keyframeBlockType & 0x08) {
					node[d][index].high = true;
					b4.push(`${motn_id}, ${d}, ${type}, ${index}, h`);
				}

				if(keyframeBlockType & 0x04) {
					node[d][index].low = true;
					b4.push(`${motn_id}, ${d}, ${type}, ${index}, l`);
				}
				index++;

				if(keyframeBlockType & 0x02) {
					node[d][index].high = true;
					b4.push(`${motn_id}, ${d}, ${type}, ${index}, h`);
				}

				if(keyframeBlockType & 0x01) {
					node[d][index].low = true;
					b4.push(`${motn_id}, ${d}, ${type}, ${index}, l`);
				}
				index++;

			} while(index < node[d].length);

		});

	});
		
	console.log(b4.join('\n'));

	// Read Block 5

	MEM.ofs = anim.dataOfs + header.block5Ofs;
	motn.forEach(node => {
			
		node.axis.forEach( d => {
				
			node[d].forEach(frame => {
					
				if(frame.high && frame.low) {
					let a = MEM.view.getUint16(MEM.ofs, true);
					a = decodeFloat16(a);
					MEM.ofs += 2;

					let b = MEM.view.getUint16(MEM.ofs, true);
					b = decodeFloat16(b);
					MEM.ofs += 2;

					let c = MEM.view.getUint16(MEM.ofs, true);
					c = decodeFloat16(c);
					MEM.ofs += 2;

					frame.easing = [a, b];
					if(node.type === 'rot') {
						c *= 65536;
						c = c % (Math.PI * 2);
					}
					frame.value = c;

				} else if(frame.high) {
					let a = MEM.view.getUint16(MEM.ofs, true);
					a = decodeFloat16(a);
					MEM.ofs += 2;

					let b = MEM.view.getUint16(MEM.ofs, true);
					b = decodeFloat16(b);
					MEM.ofs += 2;

					frame.easing = [a];
						
					if(node.type === 'rot') {
						b *= 65536;
						b = b % (Math.PI * 2);
					}
					frame.value = b;

				} else if(frame.low) {
					let a = MEM.view.getUint16(MEM.ofs, true);
					a = decodeFloat16(a);
					MEM.ofs += 2;

					if(node.type === 'rot') {
						a *= 65536;
						a = a % (Math.PI * 2);
					}
						
					frame.value = a;
				}
					
				delete frame.high;
				delete frame.low;

			});

		});

	});
	
	const compat = [];
	motn.forEach( m => {

		const { motn_id, axis, type } = m;
			
		axis.forEach( a => {

			const frames = m[a].map( f=> {
					
				const { frame, easing, value } = f;

				return {
					frame,
					easing: easing || [],
					value: value || 0
					}
			});

			compat.push({
				boneId:motn_id,
				type,
				axis:a,
				frames
			})
		});

	});
		
	const str = JSON.stringify(compat, null, 2);

}


const readMotion = () => {

	const header = {
		sequence_table_ofs : MEM.view.getUint32(0x00, true),
		sequence_names_ofs : MEM.view.getUint32(0x04, true),
		sequence_data_ofs : MEM.view.getUint32(0x08, true),
		sequence_count : MEM.view.getUint32(0x0c, true),
		filelength : MEM.view.getUint32(0x10, true)
	}

	const SEQ_LIST = new Array(header.sequence_count-1);

	let ofs_a = header.sequence_table_ofs;
	let ofs_b = header.sequence_names_ofs;

	for(let i = 0; i < SEQ_LIST.length; i++) {

		SEQ_LIST[i] = {
			dataOfs : MEM.view.getUint32(ofs_a + 0x00, true),
			ptr_b : MEM.view.getUint32(ofs_a + 0x04, true),
			name : ''
		}
		ofs_a += 8;

		let name_ptr = MEM.view.getUint32(ofs_b, true);
		ofs_b += 4;

		let ch;
		while( (ch = MEM.view.getUint8(name_ptr++)) !== 0) {
			 SEQ_LIST[i].name += String.fromCharCode(ch);
		}
		let num = (i).toString();
		while(num.length < 3) {
			num = "0" + num;
		}
		SEQ_LIST[i].dataOfs += header.sequence_data_ofs;
	}

	MEM.seq_list = SEQ_LIST;

}

// Read Motn File, Create Data View

MEM.buffer = readFileSync('../data/motion.bin');
const uint8arr = new Uint8Array(MEM.buffer.byteLength);
MEM.buffer.copy(uint8arr, 0, 0, MEM.buffer.byteLength);
MEM.view = new DataView(uint8arr.buffer);

// Read the list of animations and their offsets in the file

readMotion();
parseAnimation(242);
