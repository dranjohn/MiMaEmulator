#pragma once

#include <cctype>
#include <cstdint>
#include <memory>
#include <algorithm>
#include <functional>
#include <unordered_map>

#include <fmt/format.h>

#include "util/CharStream.h"
#include "StatusBit.h"
#include "mima/mimaprogram/MiMaMemory.h"

#include "debug/Log.h"


namespace MiMa {
	typedef std::unordered_map<std::string, size_t> StatusBitMap;


	class MicroProgramCode {
		friend struct fmt::formatter<MiMa::MicroProgramCode>;

	private:
		static const uint32_t JUMP_MASK = 0xFF;

		uint32_t bits = 0;

	public:
		//override get functions by using internal uint32_t representation of the microprogram code
		inline uint8_t getNextInstructionDecoderState() const { return (uint8_t)(bits & StatusBit:: FOLLOWING_ADDRESS); }
		inline bool isWritingToMemory() const { return bits & StatusBit:: STORAGE_WRITING; }
		inline bool isReadingFromMemory() const { return bits & StatusBit:: STORAGE_READING; }
		inline uint8_t getALUCode() const { return (uint8_t)((bits & StatusBit:: ALU_C) >> 12); }
		inline bool isStorageAddressRegisterReading() const { return bits & StatusBit:: SAR_READING; }
		inline bool isStorageDataRegisterReading() const { return bits & StatusBit:: SDR_READING; }
		inline bool isStorageDataRegisterWriting() const { return bits & StatusBit:: SDR_WRITING; }
		inline bool isInstructionRegisterReading() const { return bits & StatusBit:: IR_READING; }
		inline bool isInstructionRegisterWriting() const { return bits & StatusBit:: IR_WRITING; }
		inline bool isInstructionAddressRegisterReading() const { return bits & StatusBit:: IAR_READING; }
		inline bool isInstructionAddressRegisterWriting() const { return bits & StatusBit:: IAR_WRITING; }
		inline bool isConstantOneWriting() const { return bits & StatusBit:: ONE; }
		inline bool isALUResultWriting() const { return bits & StatusBit:: ALU_RESULT; }
		inline bool isLeftALUOperandReading() const { return bits & StatusBit:: ALU_LEFT_OPERAND; }
		inline bool isRightALUOperandReading() const { return bits & StatusBit:: ALU_RIGHT_OPERAND; }
		inline bool isAccumulatorRegisterReading() const { return bits & StatusBit:: ACCUMULATOR_READING; }
		inline bool isAccumulatorRegisterWriting() const { return bits & StatusBit:: ACCUMULATOR_WRITING; }

		//uint8_t setters
		inline void setJump(const uint8_t& jumpDestination) { bits &= ~JUMP_MASK; bits |= jumpDestination; }
		inline void setALUCode(const uint8_t& code) { bits &= ~StatusBit::ALU_C; bits |= ((code & 0x7) << 12); }

		//set/unset storage acces bit
		inline void enableMemoryWrite() { bits |= StatusBit::STORAGE_WRITING; }
		inline void disableMemoryWrite() { bits &= ~StatusBit::STORAGE_WRITING; }
		inline void enableMemoryRead() { bits |= StatusBit::STORAGE_READING; }
		inline void disableMemoryRead() { bits &= ~StatusBit::STORAGE_READING; }

		//set registers read/write bit
		inline void setStorageAddressRegisterReading() { bits |= StatusBit::SAR_READING; }
		inline void setStorageDataRegisterReading() { bits |= StatusBit::SDR_READING; }
		inline void setStorageDataRegisterWriting() { bits |= StatusBit::SDR_WRITING; }
		inline void setInstructionRegisterReading() { bits |= StatusBit::IR_READING; }
		inline void setInstructionRegisterWriting() { bits |= StatusBit::IR_WRITING; }
		inline void setInstructionAddressRegisterReading() { bits |= StatusBit::IAR_READING; }
		inline void setInstructionAddressRegisterWriting() { bits |= StatusBit::IAR_WRITING; }
		inline void setConstantOneWriting() { bits |= StatusBit::ONE; }
		inline void setALUResultWriting() { bits |= StatusBit::ALU_RESULT; }
		inline void setLeftALUOperandReading() { bits |= StatusBit::ALU_LEFT_OPERAND; }
		inline void setRightALUOperandReading() { bits |= StatusBit::ALU_RIGHT_OPERAND; }
		inline void setAccumulatorRegisterReading() { bits |= StatusBit::ACCUMULATOR_READING; }
		inline void setAccumulatorRegisterWriting() { bits |= StatusBit::ACCUMULATOR_WRITING; }

		//does nothing
		inline void pass() {}
	};

	typedef void(MicroProgramCode::* MicroProgramCodeSetFunction)();
	typedef void(MicroProgramCode::* MicroProgramCodeSet8BitFunction)(const uint8_t&);
	


	// ---------------------------------------------------------------------------------------------------
	// A conditional line of microprogram code with at least three possibilities, represented using a list
	// ---------------------------------------------------------------------------------------------------

	class MicroProgramCodeNode {
		friend class MicroProgramCodeList;
		friend struct fmt::formatter<MiMa::MicroProgramCodeList>;

	private:
		MicroProgramCodeNode* next;

		size_t upperConditionLimit;
		MicroProgramCode code;

		MicroProgramCodeNode(const size_t& upperConditionLimit, MicroProgramCodeNode* next = nullptr, MicroProgramCode code = MicroProgramCode());
	public:
		~MicroProgramCodeNode();
	};

	class MicroProgramCodeList {
		friend struct fmt::formatter<MiMa::MicroProgramCodeList>;
	private:
		MicroProgramCodeNode* head;

		std::string conditionName;
		size_t conditionMax;

	public:
		MicroProgramCodeList(const std::string& conditionName = "", const size_t& conditionMax = 0);
		~MicroProgramCodeList();

		void reset();
		void reset(const std::string& conditionName, const size_t& conditionMax);


		inline void apply(const std::function<void(MicroProgramCode&)>& func) { apply(func, 0, conditionMax); }
		void apply(const std::function<void(MicroProgramCode&)>& func, const size_t& lowerLimit, size_t upperLimit);

		inline void apply(const MicroProgramCodeSetFunction& func, const size_t& lowerLimit, size_t upperLimit) {
			apply([func](MicroProgramCode& microProgramCode) { std::invoke(func, microProgramCode); }, lowerLimit, upperLimit);
		}

		inline void apply(const MicroProgramCodeSet8BitFunction& func, const uint8_t& value) {
			apply([func, value](MicroProgramCode& microProgramCode) { std::invoke(std::bind(func, std::placeholders::_1, value), microProgramCode); }, 0, conditionMax);
		}
		inline void apply(const MicroProgramCodeSet8BitFunction& func, const uint8_t& value, const size_t& lowerLimit, const size_t& upperLimit) {
			apply([func, value](MicroProgramCode& microProgramCode) { std::invoke(std::bind(func, std::placeholders::_1, value), microProgramCode); }, lowerLimit, upperLimit);
		}


		MicroProgramCode get(const StatusBitMap& statusBits);
	};



	// ------------------------------------------------
	// A microprogram that can run on a minimal machine
	// 
	// Consists out of a memory containing read-only
	// lines of microprogram code.
	// ------------------------------------------------

	class MicroProgram {
		friend struct fmt::formatter<MiMa::MicroProgram>;

	private:
		//minimal machine microprogram read-only memory
		std::shared_ptr<MicroProgramCodeList[]> memory;
	public:
		MicroProgram(const std::shared_ptr<MicroProgramCodeList[]>& memory) : memory(memory) {}

		inline const MicroProgramCode getMicroCode(const uint8_t& memoryAddress, const StatusBitMap& statusBits) const { return memory[memoryAddress].get(statusBits); }
	};
}



//microprogramm code fmt formatting definition
template<>
struct fmt::formatter<MiMa::MicroProgramCode> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::MicroProgramCode& code, FormatContext& ctx) { return fmt::format_to(ctx.out(), "0x{:07X}", code.bits); }
};

template<>
struct fmt::formatter<MiMa::MicroProgramCodeList> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::MicroProgramCodeList& codeList, FormatContext& ctx) {
		if (codeList.conditionMax == 0) {
			return fmt::format_to(ctx.out(), "Unconditional microcode: {}", codeList.head->code);
		}

		//if the microprogram code list has only one element, print it in one line
		if (codeList.head->upperConditionLimit == codeList.conditionMax) {
			return fmt::format_to(ctx.out(), "Conditional microcode for {} up to max 0x{:X}: {}", codeList.conditionName, codeList.conditionMax, codeList.head->code);
		}


		//otherwise, format each node into a separate line
		std::string listOutput = fmt::format("Conditional microcode for {} up to max 0x{:X}:\n{{}}", codeList.conditionName, codeList.conditionMax);
		std::string nodeFormat = "up to 0x{:02X}: {}\n{{}}";

		MiMa::MicroProgramCodeNode* current = codeList.head;
		while (current != nullptr) {
			std::string node = fmt::format(nodeFormat, current->upperConditionLimit, current->code);
			listOutput = fmt::format(listOutput, node);

			current = current->next;
		}

		return fmt::format_to(ctx.out(), listOutput, "");
	}
};


template<>
struct fmt::formatter<MiMa::MicroProgram> {
private:
	size_t lowerLimit = 0;
	size_t upperLimit = 0x100;

	static const size_t char_0 = '0';

public:
	constexpr auto parse(format_parse_context& ctx) {
		auto it = ctx.begin();
		auto end = ctx.end();

		if (it == end && *it == '}') {
			return it;
		}

		lowerLimit = 0;
		while (*it != ',') {
			if (it == end || *it == '}') {
				throw format_error("Expected a ',' somewhere in the range declaration");
			}

			if (!isdigit(*it)) {
				throw format_error(fmt::format("Only digits [0-9] are allowed for range declarations, found {}", *it));
			}

			lowerLimit *= 10;
			lowerLimit += *it - char_0;

			++it;
		}
		++it;

		upperLimit = 0;
		while (it != end && *it != '}') {
			if (!isdigit(*it)) {
				throw format_error(fmt::format("Only digits [0-9] are allowed for range declarations, found {}", *it));
			}

			upperLimit *= 10;
			upperLimit += *it - char_0;

			++it;
		}

		return it;
	}

	template<typename FormatContext>
	auto format(const MiMa::MicroProgram& program, FormatContext& ctx) {
		if (lowerLimit >= upperLimit) {
			return fmt::format_to(ctx.out(), "");
		}

		if (lowerLimit + 1 == upperLimit) {
			return fmt::format_to(ctx.out(), "at 0x{:X}: {}", lowerLimit, program.memory[lowerLimit]);
		}

		std::string listOutput = "\n{}";
		std::string nodeFormat = "at 0x{:X}: {}\n{{}}";
		for (size_t i = lowerLimit; i < upperLimit; ++i) {
			std::string node = fmt::format(nodeFormat, i, program.memory[i]);
			listOutput = fmt::format(listOutput, node);
		}

		return fmt::format_to(ctx.out(), listOutput, "");
	}
};
