#pragma once

#include <string>
#include <memory>

#include "util/MinType.h"
#include "util/Bitfield.h"
#include "util/Tree.h"

#include "mimaprogram/MiMaMemory.h"
#include "microprogram/MicroProgram.h"

#include "debug/Log.h"
#include "debug/LogFormat.h"

namespace MiMa {
	// --------------------------------------
	// Emulatable minimal machine abstraction
	//
	// Contains the common components of a
	// minimal machine, and lets the user
	// exchange some of these, for example
	// the instruction decoder program or the
	// initial main memory.
	// --------------------------------------

	class MinimalMachine {
		friend struct fmt::formatter<MiMa::MinimalMachine>;

	public:
		//define data sizes for the MiMa
		static const size_t DATA_SIZE = 24;
		static const size_t ADDRESS_SIZE = 20;
		static const size_t OP_CODE_SIZE = 8;

		typedef typename MinType<DATA_SIZE>::type Data;
		typedef typename MinType<ADDRESS_SIZE>::type Address;

	private:
		//keeps track of the current main memory access
		struct MemoryState {
			uint32_t address : 20;
			uint32_t access : 1;
			uint32_t accessDuration : 11;
		};

	private:
		//cnstant one register
		static const Data ONE = 1;

		//accumulator for storing calculation results
		union {
			Data value : DATA_SIZE;
			BitField<DATA_SIZE - 1, 1> negative;
		} accumulator;

		//registers for instruction reading
		Address instructionAddressRegister : ADDRESS_SIZE;
		union {
			Data value : DATA_SIZE;
			BitField<0, ADDRESS_SIZE> address;
			BitField<DATA_SIZE - OP_CODE_SIZE, OP_CODE_SIZE> opCode;
		} instructionRegister;

		//ALU registers
		Data X : DATA_SIZE;
		Data Y : DATA_SIZE;
		Data Z : DATA_SIZE;

		//registers for reading/writing from/to the memory
		Address storageAddressRegister : ADDRESS_SIZE;
		Data storageDataRegister : DATA_SIZE;

		//exchangable MiMa components
		std::shared_ptr<const MicroProgram> instructionDecoder;
		std::shared_ptr<MiMaMemory> memory;

		//MiMa state
		bool running;
		uint8_t instructionDecoderState;
		MemoryState memoryState;
	public:
		MinimalMachine(const std::shared_ptr<const MicroProgram>& instructionDecoder, const std::shared_ptr<MiMaMemory>& memory);
		~MinimalMachine() { MIMA_LOG_INFO("Destructed MiMa"); }

		//emulate minimal machine
		void emulateClockCycle();
		void emulateInstructionCycle();
		void emulateLifeTime();
	};
}



//minimal machine fmt formatting definition
template<>
struct fmt::formatter<MiMa::MinimalMachine> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::MinimalMachine& mima, FormatContext& ctx) {
		Tree<std::string> hierarchy("MinimalMachine state");
		DataNode<std::string>& root = hierarchy.getRoot();

		root.addChild(fmt::format("running: {}", mima.running));

		DataNode<std::string>& instructionDecoderRoot = root.addChild("Instruction decoder state");
		instructionDecoderRoot.addChild(fmt::format("next microprogram instruction address: 0x{:02X}", mima.instructionDecoderState));
		instructionDecoderRoot.addChild(fmt::format("next microprogram instruction code: {}", mima.instructionDecoder->getMicroCode(mima.instructionDecoderState, MiMa::StatusBitMap())));

		DataNode<std::string>& registersRoot = root.addChild("Register states");
		registersRoot.addChild(fmt::format("IAR: 0x{:05X}", mima.instructionAddressRegister));
		registersRoot.addChild(fmt::format("IR: 0x{:06X}", mima.instructionRegister.value));
		registersRoot.addChild(fmt::format("X: 0x{:06X}", mima.X));
		registersRoot.addChild(fmt::format("Y: 0x{:06X}", mima.Y));
		registersRoot.addChild(fmt::format("Z: 0x{:06X}", mima.Z));
		registersRoot.addChild(fmt::format("Accumulator: 0x{:06X}", mima.accumulator.value));
		registersRoot.addChild(fmt::format("SAR: 0x{:05X}", mima.storageAddressRegister));
		registersRoot.addChild(fmt::format("SDR: 0x{:06X}", mima.storageDataRegister));

		return fmt::format_to(ctx.out(), MiMa::formatHierarchy(hierarchy));
	}
};
