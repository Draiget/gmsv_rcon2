#include "utils_asm.h"

void asm_copy_op_code_or_iterate(unsigned char*& func, unsigned char*& dest) {
	if (dest) {
		*dest++ = *func++;
	} else {
		func++;
	}
}

int asm_copy_bytes(unsigned char* func, unsigned char* dest, int required_len) {
	auto byte_count = 0;
	while (byte_count < required_len && *func != 0xCC) {
		// prefixes F0h, F2h, F3h, 66h, 67h, D8h-DFh, 2Eh, 36h, 3Eh, 26h, 64h and 65h
		auto operand_size = 4;
		auto fpu = 0; // Floating point unit
		auto two_byte = false;

		while (*func == OP_LOCK ||
			*func == 0xF2 || // REPNZ, REPNE, REP
			*func == 0xF3 || // REPZ, REPE, REP
			(*func & OP_CLEAR_DIRECTION_FLAG) == OP_FS ||
			(*func & OP_CLEAR_CARRY_FLAG) == 0xD8 || // FADD, FMUL, FCOMP, FSUB, FSUBR, FDIV, FDIVR
			(*func & OP_JLE) == OP_BOUND) 
		{
			if (*func == OP_OPERAND_SIZE) {
				operand_size = 2;
			}
			// FADD, FMUL, FCOMP, FSUB, FSUBR, FDIV, FDIVR
			else if ((*func & OP_CLEAR_CARRY_FLAG) == 0xD8) {
				fpu = *func;

				asm_copy_op_code_or_iterate(func, dest);
				byte_count++;

				break;
			}

			asm_copy_op_code_or_iterate(func, dest);
			byte_count++;
		}

		if (*func == OP_TWO_BYTES_INSTR) {
			two_byte = true;

			asm_copy_op_code_or_iterate(func, dest);
			byte_count++;
		}

		const auto opCode = *func++;
		if (dest) {
			*dest++ = opCode;
		}

		byte_count++;
		unsigned char modRm = 0xFF;

		if (fpu) {
			// ROL, ROR, RCL, RCR, SHL, SAL, SHR, SAR
			if ((opCode & 0xC0) != 0xC0) {
				modRm = opCode;
			}
		} else if (!two_byte) {
			if ((opCode & 0xC4) == 0x00 ||
				(opCode & 0xF4) == 0x60 &&
				((opCode & 0x0A) == 0x02 ||
					(opCode & 0x09) == 0x09) ||
				(opCode & 0xF0) == 0x80 ||
				(opCode & 0xF8) == 0xC0 &&
				(opCode & 0x0E) != 0x02 ||
				(opCode & 0xFC) == 0xD0 ||
				(opCode & 0xF6) == 0xF6) {
				modRm = *func++;

				if (dest) {
					*dest++ = modRm;
				}

				byte_count++;
			}
		} else {
			if ((opCode & 0xF0) == 0x00 &&
				(opCode & 0x0F) >= 0x04 &&
				(opCode & 0x0D) != 0x0D ||
				(opCode & 0xF0) == 0x30 ||
				opCode == 0x77 ||
				(opCode & 0xF0) == 0x80 ||
				(opCode & 0xF0) == 0xA0 &&
				(opCode & 0x07) <= 0x02 ||
				(opCode & 0xF8) == 0xC8) {
				// No mod R/M byte
			} else {
				modRm = *func++;

				if (dest) {
					*dest++ = modRm;
				}

				byte_count++;
			}
		}

		if ((modRm & 0x07) == 0x04 &&
			(modRm & 0xC0) != 0xC0) {
			asm_copy_op_code_or_iterate(func, dest);
			byte_count++;
		}

		if ((modRm & 0xC5) == 0x05) {
			if (dest) {
				*reinterpret_cast<unsigned int*>(dest) = *reinterpret_cast<unsigned int*>(func);
				dest += 4;
			}

			func += 4;
			byte_count += 4;
		}

		if ((modRm & 0xC0) == 0x40) {
			asm_copy_op_code_or_iterate(func, dest);

			byte_count++;
		}

		if ((modRm & 0xC0) == 0x80) {
			if (dest) {
				*reinterpret_cast<unsigned int*>(dest) = *reinterpret_cast<unsigned int*>(func);
				dest += 4;
			}

			func += 4;
			byte_count += 4;
		}

		if (fpu) {
			// Can't have immediate operand
		} else if (!two_byte) {
			if ((opCode & 0xC7) == 0x04 ||
				(opCode & 0xFE) == 0x6A ||	// PUSH/POP/IMUL
				(opCode & 0xF0) == 0x70 ||	// Jcc
				opCode == 0x80 ||
				opCode == 0x83 ||
				(opCode & 0xFD) == 0xA0 ||	// MOV
				opCode == 0xA8 ||	// TEST
				(opCode & 0xF8) == 0xB0 ||	// MOV
				(opCode & 0xFE) == 0xC0 ||	// RCL
				opCode == 0xC6 ||	// MOV
				opCode == 0xCD ||	// INT
				(opCode & 0xFE) == 0xD4 ||	// AAD/AAM
				(opCode & 0xF8) == 0xE0 ||	// LOOP/JCXZ
				opCode == 0xEB ||
				opCode == 0xF6 &&
				(modRm & 0x30) == 0x00) // TEST
			{
				asm_copy_op_code_or_iterate(func, dest);

				byte_count++;
			}
			else if ((opCode & 0xF7) == OP_RETN) {
				if (dest) {
					*reinterpret_cast<unsigned short*>(dest) = *reinterpret_cast<unsigned short*>(func);
					dest += 2;
				}

				func += 2;
				byte_count += 2;
			}
			else if ((opCode & 0xFC) == 0x80 ||
				(opCode & 0xC7) == 0x05 ||
				(opCode & 0xF8) == 0xB8 ||
				(opCode & 0xFE) == 0xE8 ||	  // CALL/Jcc
				(opCode & 0xFE) == 0x68 ||
				(opCode & 0xFC) == 0xA0 ||
				(opCode & 0xEE) == 0xA8 ||
				opCode == 0xC7 ||
				opCode == 0xF7 &&
				(modRm & 0x30) == 0x00) {
				if (dest) {
					if ((opCode & 0xFE) == 0xE8) {
						if (operand_size == 4) {
							*reinterpret_cast<long*>(dest) = func + *reinterpret_cast<long*>(func) - dest;
						}
						else {
							*reinterpret_cast<short*>(dest) = func + *reinterpret_cast<short*>(func) - dest;
						}
					}
					else {
						if (operand_size == 4) {
							*reinterpret_cast<unsigned long*>(dest) = *reinterpret_cast<unsigned long*>(func);
						}
						else {
							*reinterpret_cast<unsigned short*>(dest) = *reinterpret_cast<unsigned short*>(func);
						}
					}

					dest += operand_size;
				}

				func += operand_size;
				byte_count += operand_size;

			}
		}
		else {
			if (opCode == 0xBA ||	// BT
				opCode == 0x0F ||	// 3DNow!
				(opCode & 0xFC) == 0x70 ||	// PSLLW
				(opCode & 0xF7) == 0xA4 ||	// SHLD
				opCode == 0xC2 ||
				opCode == 0xC4 ||
				opCode == 0xC5 ||
				opCode == 0xC6) {
				asm_copy_op_code_or_iterate(func, dest);
			}
			else if ((opCode & 0xF0) == 0x80) // Jcc -i
			{
				if (dest) {
					if (operand_size == 4) {
						*reinterpret_cast<unsigned long*>(dest) = *reinterpret_cast<unsigned long*>(func);
					}
					else {
						*reinterpret_cast<unsigned short*>(dest) = *reinterpret_cast<unsigned short*>(func);
					}

					dest += operand_size;
				}

				func += operand_size;
				byte_count += operand_size;
			}
		}
	}

	return byte_count;
}

void asm_inject_jmp(void* src, void* dest) {
	const auto c_src = static_cast<unsigned char*>(src);
	const auto c_dest = static_cast<unsigned char*>(dest);

	*c_src = OP_JMP;
	*reinterpret_cast<long*>(c_src + 1) = static_cast<long>(c_dest - (c_src + OP_JMP_SIZE));
}

void asm_fill_nop(void* src, unsigned int len) {
	auto src2 = static_cast<unsigned char*>(src);
	while (len) {
		*src2++ = OP_NOP;
		--len;
	}
}

void* asm_eval_jmp(void* src) {
	auto address = static_cast<unsigned char*>(src);
	if (!address) {
		return nullptr;
	}

	if (address[0] == OP_PREFIX && address[1] == OP_JMP_SEG) {
		address += 2;
		address = *reinterpret_cast<unsigned char**>(address);
		return *reinterpret_cast<void**>(address);
	}

	if (address[0] == OP_JMP_BYTE) {
		address = &address[OP_JMP_BYTE_SIZE] + *reinterpret_cast<char*>(&address[1]);

		if (address[0] == OP_JMP) {
			address = address + *reinterpret_cast<int*>(&address[1]);
		}

		return address;
	}

	return address;
}
