#include "utils_asm.h"

void AsmCopyOpCodeOrIterate(unsigned char *&func, unsigned char *&dest) {
	if (dest) {
		*dest++ = *func++;
	} else {
		func++;
	}
}

int AsmCopyBytes(unsigned char *func, unsigned char *dest, int required_len)
{
	auto bytecount = 0;
	while (bytecount < required_len && *func != 0xCC) {
		// prefixes F0h, F2h, F3h, 66h, 67h, D8h-DFh, 2Eh, 36h, 3Eh, 26h, 64h and 65h
		auto operandSize = 4;
		auto fpu = 0; // Floating point unit
		auto twoByte = false;

		while (*func == OP_LOCK ||
			*func == 0xF2 || // REPNZ, REPNE, REP
			*func == 0xF3 || // REPZ, REPE, REP
			(*func & OP_CLEAR_DIRECTION_FLAG) == OP_FS ||
			(*func & OP_CLEAR_CARRY_FLAG) == 0xD8 || // FADD, FMUL, FCOMP, FSUB, FSUBR, FDIV, FDIVR
			(*func & OP_JLE) == OP_BOUND)
		{
			if (*func == OP_OPERAND_SIZE) {
				operandSize = 2;
			}
			// FADD, FMUL, FCOMP, FSUB, FSUBR, FDIV, FDIVR
			else if ((*func & OP_CLEAR_CARRY_FLAG) == 0xD8) {
				fpu = *func;

				AsmCopyOpCodeOrIterate(func, dest);
				bytecount++;

				break;
			}

			AsmCopyOpCodeOrIterate(func, dest);
			bytecount++;
		}

		if (*func == OP_TWO_BYTES_INSTR) {
			twoByte = true;

			AsmCopyOpCodeOrIterate(func, dest);
			bytecount++;
		}

		const auto opcode = *func++;
		if (dest) {
			*dest++ = opcode;
		}

		bytecount++;
		unsigned char modRm = 0xFF;

		if (fpu) {
			// ROL, ROR, RCL, RCR, SHL, SAL, SHR, SAR
			if ((opcode & 0xC0) != 0xC0) {
				modRm = opcode;
			}
		} else if (!twoByte) {
			if ((opcode & 0xC4) == 0x00 ||
				(opcode & 0xF4) == 0x60 &&
				((opcode & 0x0A) == 0x02 ||
				(opcode & 0x09) == 0x09) ||
				(opcode & 0xF0) == 0x80 ||
				(opcode & 0xF8) == 0xC0 &&
				(opcode & 0x0E) != 0x02 ||
				(opcode & 0xFC) == 0xD0 ||
				(opcode & 0xF6) == 0xF6)
			{
				modRm = *func++;

				if (dest) {
					*dest++ = modRm;
				}

				bytecount++;
			}
		} else {
			if ((opcode & 0xF0) == 0x00 &&
				(opcode & 0x0F) >= 0x04 &&
				(opcode & 0x0D) != 0x0D ||
				(opcode & 0xF0) == 0x30 ||
						 opcode == 0x77 ||
				(opcode & 0xF0) == 0x80 ||
				(opcode & 0xF0) == 0xA0 &&
				(opcode & 0x07) <= 0x02 ||
				(opcode & 0xF8) == 0xC8)
			{
				// No mod R/M byte
			} else {
				modRm = *func++;

				if (dest) {
					*dest++ = modRm;
				}

				bytecount++;
			}
		}

		if ((modRm & 0x07) == 0x04 &&
			(modRm & 0xC0) != 0xC0)
		{
			AsmCopyOpCodeOrIterate(func, dest);
			bytecount++;
		}

		if ((modRm & 0xC5) == 0x05) {
			if (dest) {
				*reinterpret_cast<unsigned int *>(dest) = *reinterpret_cast<unsigned int *>(func);
				dest += 4;
			}

			func += 4;
			bytecount += 4;
		}

		if ((modRm & 0xC0) == 0x40) {
			AsmCopyOpCodeOrIterate(func, dest);

			bytecount++;
		}

		if ((modRm & 0xC0) == 0x80) {
			if (dest) {
				*reinterpret_cast<unsigned int *>(dest) = *reinterpret_cast<unsigned int *>(func);
				dest += 4;
			}

			func += 4;
			bytecount += 4;
		}

		if (fpu) {
			// Can't have immediate operand
		} else if (!twoByte) {
			if ((opcode & 0xC7) == 0x04 ||
				(opcode & 0xFE) == 0x6A ||	// PUSH/POP/IMUL
				(opcode & 0xF0) == 0x70 ||	// Jcc
						 opcode == 0x80 ||
						 opcode == 0x83 ||
				(opcode & 0xFD) == 0xA0 ||	// MOV
						 opcode == 0xA8 ||	// TEST
				(opcode & 0xF8) == 0xB0 ||	// MOV
				(opcode & 0xFE) == 0xC0 ||	// RCL
						 opcode == 0xC6 ||	// MOV
						 opcode == 0xCD ||	// INT
				(opcode & 0xFE) == 0xD4 ||	// AAD/AAM
				(opcode & 0xF8) == 0xE0 ||	// LOOP/JCXZ
						 opcode == 0xEB ||
						 opcode == 0xF6 &&
				 (modRm & 0x30) == 0x00) // TEST
			{
				AsmCopyOpCodeOrIterate(func, dest);

				bytecount++;
			} 
			else if ((opcode & 0xF7) == OP_RETN)
			{
				if (dest) {
					*reinterpret_cast<unsigned short *>(dest) = *reinterpret_cast<unsigned short *>(func);
					dest += 2;
				}

				func += 2;
				bytecount += 2;
			} else if ((opcode & 0xFC) == 0x80 ||
				(opcode & 0xC7) == 0x05 ||
				(opcode & 0xF8) == 0xB8 ||
				(opcode & 0xFE) == 0xE8 ||	  // CALL/Jcc
				(opcode & 0xFE) == 0x68 ||
				(opcode & 0xFC) == 0xA0 ||
				(opcode & 0xEE) == 0xA8 ||
						 opcode == 0xC7 ||
						 opcode == 0xF7 &&
				(modRm & 0x30) == 0x00)
			{
				if (dest) {
					if ((opcode & 0xFE) == 0xE8) {
						if (operandSize == 4) {
							*reinterpret_cast<long *>(dest) = func + *reinterpret_cast<long *>(func) - dest;
						} else {
							*reinterpret_cast<short *>(dest) = func + *reinterpret_cast<short *>(func) - dest;
						}
					} else {
						if (operandSize == 4) {
							*reinterpret_cast<unsigned long *>(dest) = *reinterpret_cast<unsigned long *>(func);
						} else {
							*reinterpret_cast<unsigned short *>(dest) = *reinterpret_cast<unsigned short *>(func);
						}
					}

					dest += operandSize;
				}

				func += operandSize;
				bytecount += operandSize;

			}
		} else {
			if (opcode == 0xBA ||	// BT
				opcode == 0x0F ||	// 3DNow!
	   (opcode & 0xFC) == 0x70 ||	// PSLLW
	   (opcode & 0xF7) == 0xA4 ||	// SHLD
				opcode == 0xC2 ||
				opcode == 0xC4 ||
				opcode == 0xC5 ||
				opcode == 0xC6)
			{
				AsmCopyOpCodeOrIterate(func, dest);
			} 
			else if ((opcode & 0xF0) == 0x80) // Jcc -i
			{
				if (dest) {
					if (operandSize == 4) {
						*reinterpret_cast<unsigned long *>(dest) = *reinterpret_cast<unsigned long *>(func);
					} else {
						*reinterpret_cast<unsigned short *>(dest) = *reinterpret_cast<unsigned short *>(func);
					}

					dest += operandSize;
				}

				func += operandSize;
				bytecount += operandSize;
			}
		}
	}

	return bytecount;
}

void AsmInjectJmp(void *src, void *dest)
{
	const auto csrc = static_cast<unsigned char *>(src);
	const auto cdest = static_cast<unsigned char *>(dest);

	*csrc = OP_JMP;
	*reinterpret_cast<long *>(csrc + 1) = static_cast<long>(cdest - (csrc + OP_JMP_SIZE));
}

void AsmFillNop(void *src, unsigned int len)
{
	auto src2 = static_cast<unsigned char *>(src);
	while (len) {
		*src2++ = OP_NOP;
		--len;
	}
}

void *AsmEvalJmp(void *src)
{
	auto addr = static_cast<unsigned char *>(src);

	if (!addr) {
		return nullptr;
	}

	if (addr[0] == OP_PREFIX && addr[1] == OP_JMP_SEG) {
		addr += 2;
		addr = *reinterpret_cast<unsigned char **>(addr);
		return *reinterpret_cast<void **>(addr);
	}

	if (addr[0] == OP_JMP_BYTE) {
		addr = &addr[OP_JMP_BYTE_SIZE] + *reinterpret_cast<char *>(&addr[1]);

		if (addr[0] == OP_JMP) {
			addr = addr + *reinterpret_cast<int *>(&addr[1]);
		}

		return addr;
	}

	return addr;
}