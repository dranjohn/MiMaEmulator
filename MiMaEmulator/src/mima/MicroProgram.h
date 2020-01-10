#pragma once

#include <cstdint>
#include <memory>
#include <algorithm>
#include <functional>

#include <fmt/format.h>

#include "util/CharStream.h"
#include "StatusBit.h"

#include "debug/Log.h"


namespace MiMa {
	typedef std::unordered_map<std::string, size_t> StatusBitList;

	// ----------------------------------------------------------------------------------------
	// One code line (with or without condition) in a microprogram running on a minimal machine
	// ----------------------------------------------------------------------------------------

	class UnconditionalMicroProgramCode;

	class ConditionalMicroProgramCode {
	public:
		virtual UnconditionalMicroProgramCode& get(const size_t& condition) = 0;
		virtual UnconditionalMicroProgramCode& get(const StatusBitList& statusBits) = 0;
	};



	// ------------------------------------------
	// An unconditional line of microprogram code
	// ------------------------------------------

	class UnconditionalMicroProgramCode : public ConditionalMicroProgramCode {
		friend struct fmt::formatter <MiMa::UnconditionalMicroProgramCode>;

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

		//unconditional microprogram codes return themselves under any given condition
		inline UnconditionalMicroProgramCode& get(const size_t&) override { return *this; }
		inline UnconditionalMicroProgramCode& get(const StatusBitList&) override { return *this; }
	};

	typedef void(UnconditionalMicroProgramCode::* MicroProgramCodeSetFunction)();
	typedef void(UnconditionalMicroProgramCode::* MicroProgramCodeSet8BitFunction)(const uint8_t&);



	// --------------------------------------------------------------
	// A conditional line of microprogram code with two possibilities
	// --------------------------------------------------------------

	class BinaryConditionalMicroProgramCode : public ConditionalMicroProgramCode {
		friend struct fmt::formatter<MiMa::BinaryConditionalMicroProgramCode>;

	private:
		static const uint32_t JUMP_MASK = 0xFF;

		const std::string conditionName;

		UnconditionalMicroProgramCode trueCode;
		UnconditionalMicroProgramCode falseCode;

	public:
		BinaryConditionalMicroProgramCode(const std::string& conditionName);

		inline UnconditionalMicroProgramCode& get(const size_t& condition) override { return (condition) ? trueCode : falseCode; }
		inline UnconditionalMicroProgramCode& get(const StatusBitList& statusBits) override {
			StatusBitList::const_iterator condition = statusBits.find(conditionName);
			return (condition != statusBits.end() && condition->second) ? trueCode : falseCode;
		}

		/*
		//provide get functions by using internal uint32_t representation of the microprogram code
		inline uint8_t getNextInstructionDecoderState() const override { return (uint8_t)(bitsCopy() & StatusBit::FOLLOWING_ADDRESS); }
		inline bool isWritingToMemory() const override { return bitsCopy() & StatusBit::STORAGE_WRITING; }
		inline bool isReadingFromMemory() const override { return bitsCopy() & StatusBit::STORAGE_READING; }
		inline uint8_t getALUCode() const override { return (uint8_t)((bitsCopy() & StatusBit::ALU_C) >> 12); }
		inline bool isStorageAddressRegisterReading() const override { return bitsCopy() & StatusBit::SAR_READING; }
		inline bool isStorageDataRegisterReading() const override { return bitsCopy() & StatusBit::SDR_READING; }
		inline bool isStorageDataRegisterWriting() const override { return bitsCopy() & StatusBit::SDR_WRITING; }
		inline bool isInstructionRegisterReading() const override { return bitsCopy() & StatusBit::IR_READING; }
		inline bool isInstructionRegisterWriting() const override { return bitsCopy() & StatusBit::IR_WRITING; }
		inline bool isInstructionAddressRegisterReading() const override { return bitsCopy() & StatusBit::IAR_READING; }
		inline bool isInstructionAddressRegisterWriting() const override { return bitsCopy() & StatusBit::IAR_WRITING; }
		inline bool isConstantOneWriting() const override { return bitsCopy() & StatusBit::ONE; }
		inline bool isALUResultWriting() const override { return bitsCopy() & StatusBit::ALU_RESULT; }
		inline bool isLeftALUOperandReading() const override { return bitsCopy() & StatusBit::ALU_LEFT_OPERAND; }
		inline bool isRightALUOperandReading() const override { return bitsCopy() & StatusBit::ALU_RIGHT_OPERAND; }
		inline bool isAccumulatorRegisterReading() const override { return bitsCopy() & StatusBit::ACCUMULATOR_READING; }
		inline bool isAccumulatorRegisterWriting() const override { return bitsCopy() & StatusBit::ACCUMULATOR_WRITING; }

		//uint8_t setters
		inline void setJump(const uint8_t& jumpDestination, bool ifCondition) { bitsRef(ifCondition) &= ~JUMP_MASK; bitsRef(ifCondition) |= jumpDestination; }
		inline void setALUCode(const uint8_t& code, bool ifCondition) { bitsRef(ifCondition) &= ~StatusBit::ALU_C; bitsRef(ifCondition) |= ((code & 0x7) << 12); }

		//set/unset storage acces bit
		inline void enableMemoryWrite(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::STORAGE_WRITING; }
		inline void disableMemoryWrite(bool ifCondition) { bitsRef(ifCondition) &= ~StatusBit::STORAGE_WRITING; }
		inline void enableMemoryRead(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::STORAGE_READING; }
		inline void disableMemoryRead(bool ifCondition) { bitsRef(ifCondition) &= ~StatusBit::STORAGE_READING; }

		//set registers read/write bit
		inline void setStorageAddressRegisterReading(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::SAR_READING; }
		inline void setStorageDataRegisterReading(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::SDR_READING; }
		inline void setStorageDataRegisterWriting(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::SDR_WRITING; }
		inline void setInstructionRegisterReading(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::IR_READING; }
		inline void setInstructionRegisterWriting(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::IR_WRITING; }
		inline void setInstructionAddressRegisterReading(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::IAR_READING; }
		inline void setInstructionAddressRegisterWriting(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::IAR_WRITING; }
		inline void setConstantOneWriting(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::ONE; }
		inline void setALUResultWriting(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::ALU_RESULT; }
		inline void setLeftALUOperandReading(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::ALU_LEFT_OPERAND; }
		inline void setRightALUOperandReading(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::ALU_RIGHT_OPERAND; }
		inline void setAccumulatorRegisterReading(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::ACCUMULATOR_READING; }
		inline void setAccumulatorRegisterWriting(bool ifCondition) { bitsRef(ifCondition) |= StatusBit::ACCUMULATOR_WRITING; }
		*/
	};
	


	// ---------------------------------------------------------------------------------------------------
	// A conditional line of microprogram code with at least three possibilities, represented using a list
	// ---------------------------------------------------------------------------------------------------

	class MicroProgramCodeNode {
		friend class MicroProgramCodeList;
		friend struct fmt::formatter<MiMa::MicroProgramCodeList>;

	private:
		MicroProgramCodeNode* next;

		size_t upperConditionLimit;
		UnconditionalMicroProgramCode code;

		MicroProgramCodeNode(const size_t& upperConditionLimit, MicroProgramCodeNode* next = nullptr, UnconditionalMicroProgramCode code = UnconditionalMicroProgramCode());
	};

	class MicroProgramCodeList : public ConditionalMicroProgramCode {
		friend struct fmt::formatter<MiMa::MicroProgramCodeList>;
	private:
		MicroProgramCodeNode* head;

		const std::string conditionName;
		const size_t conditionMax;

	public:
		MicroProgramCodeList(const std::string& conditionName = "op_code", const size_t& conditionMax = 0xFF);
		~MicroProgramCodeList();

		//apply function
		void apply(const std::function<void(UnconditionalMicroProgramCode&)>& func, const size_t& lowerLimit, size_t upperLimit);

		inline void apply(const MicroProgramCodeSetFunction& func, const size_t& lowerLimit, size_t upperLimit) {
			apply([func](UnconditionalMicroProgramCode& microProgramCode) { std::invoke(func, microProgramCode); }, lowerLimit, upperLimit);
		}

		inline void apply(const MicroProgramCodeSet8BitFunction& func, const uint8_t& value, const size_t& lowerLimit, const size_t& upperLimit) {
			apply([func, value](UnconditionalMicroProgramCode& microProgramCode) { std::invoke(std::bind(func, std::placeholders::_1, value), microProgramCode); }, lowerLimit, upperLimit);
		}


		UnconditionalMicroProgramCode& get(const size_t& condition) override;
		inline UnconditionalMicroProgramCode& get(const StatusBitList& statusBits) override {
			StatusBitList::const_iterator condition = statusBits.find(conditionName);
			return (condition != statusBits.end()) ? get(condition->second) : get(0);
		}
	};



	// ------------------------------------------------
	// A microprogram that can run on a minimal machine
	// 
	// Consists out of a memory containing read-only
	// lines of microprogram code.
	// ------------------------------------------------

	class MicroProgram {
	private:
		//minimal machine microprogram read-only memory
		std::shared_ptr<MicroProgramCodeList[]> memory;
	public:
		MicroProgram(const std::shared_ptr<MicroProgramCodeList[]>& memory) : memory(memory) {}

		inline const UnconditionalMicroProgramCode getMicroCode(const uint8_t& memoryAddress, const uint8_t& opCode) const { return memory[memoryAddress].get(opCode); }
	};
}

//microprogramm code fmt formatting definition
template<>
struct fmt::formatter<MiMa::UnconditionalMicroProgramCode> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::UnconditionalMicroProgramCode& code, FormatContext& ctx) { return fmt::format_to(ctx.out(), "0x{:07X}", code.bits); }
};

template<>
struct fmt::formatter<MiMa::BinaryConditionalMicroProgramCode> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::BinaryConditionalMicroProgramCode& code, FormatContext& ctx) { return fmt::format_to(ctx.out(), "true: 0x{:07X} / false: 0x{:07X}", code.trueBits, code.falseBits); }
};

template<>
struct fmt::formatter<MiMa::MicroProgramCodeList> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::MicroProgramCodeList& codeList, FormatContext& ctx) {
		if (codeList.head->upperConditionLimit == codeList.conditionMax) {
			return fmt::format_to(ctx.out(), "{}", codeList.head->code);
		}


		std::string listOutput = "OpCode conditional:\n{}";
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
