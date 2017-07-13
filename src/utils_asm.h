#ifndef ZONTWELG_GMSVRCON2_UTILS_ASM_H
#define ZONTWELG_GMSVRCON2_UTILS_ASM_H

#define OP_RETN						0xC2
#define OP_FS						0x64	// FS segment override prefix
#define OP_BOUND					0x62	// Check Array Index Against Bounds
#define OP_JLE						0x7E
#define OP_LOCK						0xF0
#define OP_JMP						0xE9
#define OP_JMP_SIZE					5
#define OP_NOP						0x90
#define OP_NOP_SIZE					1
#define OP_PREFIX					0xFF
#define OP_TWO_BYTES_INSTR			0xFF
#define OP_OPERAND_SIZE				0x66
#define OP_CLEAR_CARRY_FLAG			0xF8
#define OP_CLEAR_DIRECTION_FLAG		0xFC
#define OP_JMP_SEG					0x25
#define OP_JMP_BYTE					0xEB
#define OP_JMP_BYTE_SIZE			2

int AsmCopyBytes(unsigned char *func, unsigned char *dest, int required_len);

#endif
