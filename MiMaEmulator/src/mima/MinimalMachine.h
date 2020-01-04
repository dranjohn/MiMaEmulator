#pragma once

#include <string>
#include <memory>

#include "StatusBit.h"
#include "MicroProgram.h"
#include "util/MinType.h"
#include "util/Bitfield.h"
#include "debug/Log.h"

namespace MiMa {
	typedef MicroProgramCode(*InstructionDecodeFunction)(const uint8_t&, const uint8_t&); //arguments: decoding value, opCode


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
		MicroProgram instructionDecoder;
		InstructionDecodeFunction decodingFunction;
		MemoryCell* memory;

		//MiMa state:
		bool running;
		uint8_t instructionDecoderState;
		MemoryState memoryState;
	public:
		MinimalMachine(MicroProgram instructionDecoder, InstructionDecodeFunction decodingFunction, MemoryCell memory[MEMORY_CAPACITY]);
		~MinimalMachine() { MIMA_LOG_INFO("Destructed MiMa"); }

		//emulate minimal machine
		void emulateClockCycle();
		void emulateInstructionCycle();
		void emulateLifeTime();

		void printState() const;
	};
}
