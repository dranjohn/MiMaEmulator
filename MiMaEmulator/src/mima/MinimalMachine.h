#pragma once

#include <string>

#include "util/MinType.h"
#include "util/Bitfield.h"

#define BIT(n) (1 << n)

namespace MiMa {
	static const enum StatusBit : uint32_t {
		FOLLOWING_ADDRESS = 0xFF,
		RESERVED = 0x300,
		STORAGE_WRITING = BIT(10),
		STORAGE_READING = BIT(11),
		ALU_C = 0x7000,
		SAR_READING = BIT(15),
		SDR_WRITING = BIT(16),
		SDR_READING = BIT(17),
		IR_WRITING = BIT(18),
		IR_READING = BIT(19),
		IAR_WRITING = BIT(20),
		IAR_READING = BIT(21),
		ONE = BIT(22),
		ALU_RESULT = BIT(23),
		ALU_RIGHT_OPERAND = BIT(24),
		ALU_LEFT_OPERAND = BIT(25),
		ACCUMULATOR_WRITING = BIT(26),
		ACCUMULATOR_READING = BIT(27),
		DECODING = 0xF0000000
	};


	struct MemoryCell {
		uint32_t data : 24;
		uint32_t debug : 8;
	};

	struct MemoryState {
		uint32_t address : 20;
		uint32_t access : 1;
		uint32_t accessDuration : 11;
	};


	class MinimalMachine {
	public:
		//Define data sizes for the MiMa.
		static const size_t DATA_SIZE = 24;
		static const size_t ADDRESS_SIZE = 20;
		static const size_t OP_CODE_SIZE = 8;
		static const size_t MEMORY_CAPACITY = 0xFFFFF;

		typedef typename MinType<DATA_SIZE>::type Data;
		typedef typename MinType<ADDRESS_SIZE>::type Address;

	private:
		//Constant one register:
		static const Data ONE = 1;

		//Accumulator for storing calculation results:
		union {
			Data value : DATA_SIZE;
			BitField<DATA_SIZE - 1, 1> negative;
		} accumulator;

		//Registers for instruction reading:
		Address instructionAddressRegister : ADDRESS_SIZE;
		union {
			Data value : DATA_SIZE;
			BitField<0, ADDRESS_SIZE> address;
			BitField<DATA_SIZE - OP_CODE_SIZE, OP_CODE_SIZE> opCode;
		} instructionRegister;

		//ALU registers:
		Data X : DATA_SIZE;
		Data Y : DATA_SIZE;
		Data Z : DATA_SIZE;

		//Registers for reading/writing from/to the memory:
		Address storageAddressRegister : ADDRESS_SIZE;
		Data storageDataRegister : DATA_SIZE;

		//Exchangable MiMa components:
		uint32_t* instructionDecoder;
		uint32_t(*decodingFunction)(const uint8_t&, const uint8_t&, const uint8_t&); //arguments: decoding value, opCode, ptr to instructionDecoderState
		MemoryCell* memory;

		//MiMa state:
		bool running;
		uint8_t instructionDecoderState;
		MemoryState memoryState;
	public:
		MinimalMachine(uint32_t* instructionDecoder, uint32_t(*decodingFunction)(const uint8_t&, const uint8_t&, const uint8_t&), MemoryCell memory[MEMORY_CAPACITY]);

		void emulateClockCycle();
		void emulateInstructionCycle();
		void emulateLifeTime();

		void printState() const;
	};
}
