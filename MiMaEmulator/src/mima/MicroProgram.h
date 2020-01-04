#pragma once

#include <cstdint>
#include <memory>

#include <fmt/format.h>

#include "util/CharStream.h"
#include "StatusBit.h"

namespace MiMa {
	class MicroProgramCode {
		friend struct fmt::formatter<MiMa::MicroProgramCode>;

	private:
		static const uint32_t JUMP_MASK = 0xFF;

		uint32_t bits = 0;

	public:
		inline uint8_t getNextInstructionDecoderState() const { return (uint8_t)(bits & FOLLOWING_ADDRESS); }
		inline bool isWritingToMemory() const { return bits & STORAGE_WRITING; }
		inline bool isReadingFromMemory() const { return bits & STORAGE_READING; }
		inline uint8_t getALUCode() const { return (uint8_t)((bits & ALU_C) >> 12); }
		inline bool isStorageAddressRegisterReading() const { return bits & SAR_READING; }
		inline bool isStorageDataRegisterReading() const { return bits & SDR_READING; }
		inline bool isStorageDataRegisterWriting() const { return bits & SDR_WRITING; }
		inline bool isInstructionRegisterReading() const { return bits & IR_READING; }
		inline bool isInstructionRegisterWriting() const { return bits & IR_WRITING; }
		inline bool isInstructionAddressRegisterReading() const { return bits & IAR_READING; }
		inline bool isInstructionAddressRegisterWriting() const { return bits & IAR_WRITING; }
		inline bool isConstantOneWriting() const { return bits & ONE; }
		inline bool isALUResultWriting() const { return bits & ALU_RESULT; }
		inline bool isLeftALUOperandReading() const { return bits & ALU_LEFT_OPERAND; }
		inline bool isRightALUOperandReading() const { return bits & ALU_RIGHT_OPERAND; }
		inline bool isAccumulatorRegisterReading() const { return bits & ACCUMULATOR_READING; }
		inline bool isAccumulatorRegisterWriting() const { return bits & ACCUMULATOR_WRITING; }
		inline uint8_t getDecodingBits() const { return (uint8_t)((bits & DECODING) >> 28); }

		inline void setJump(const uint8_t& jumpDestination) { bits &= ~JUMP_MASK; bits |= jumpDestination; }
		inline void addBits(const uint32_t& addedBits) { bits |= addedBits; }

		inline void enableMemoryWrite() { bits |= STORAGE_WRITING; }
		inline void disableMemoryWrite() { bits &= ~STORAGE_WRITING; }
		inline void enableMemoryRead() { bits |= STORAGE_READING; }
		inline void disableMemoryRead() { bits &= ~STORAGE_READING; }

		inline void setDecode(const uint8_t& code) { bits &= ~DECODING; bits |= ((code & 0xF) << 28); }
		inline void setALUCode(const uint8_t& code) { bits &= ~ALU_C; bits |= ((code & 0x7) << 12); }

		inline void setStorageAddressRegisterReading() { bits |= SAR_READING; }
		inline void setStorageDataRegisterReading() { bits |= SDR_READING; }
		inline void setStorageDataRegisterWriting() { bits |= SDR_WRITING; }
		inline void setInstructionRegisterReading() { bits |= IR_READING; }
		inline void setInstructionRegisterWriting() { bits |= IR_WRITING; }
		inline void setInstructionAddressRegisterReading() { bits |= IAR_READING; }
		inline void setInstructionAddressRegisterWriting() { bits |= IAR_WRITING; }
		inline void setConstantOneWriting() { bits |= ONE; }
		inline void setALUResultWriting() { bits |= ALU_RESULT; }
		inline void setLeftALUOperandReading() { bits |= ALU_LEFT_OPERAND; }
		inline void setRightALUOperandReading() { bits |= ALU_RIGHT_OPERAND; }
		inline void setAccumulatorRegisterReading() { bits |= ACCUMULATOR_READING; }
		inline void setAccumulatorRegisterWriting() { bits |= ACCUMULATOR_WRITING; }

		inline void pass() {} //does nothing
	};
	typedef void(MicroProgramCode::* MicroProgramCodeSetFunction)();

	class MicroProgram {
	private:
		//minimal machine microprogram read-only memory
		std::shared_ptr<MicroProgramCode[]> memory;
	public:
		MicroProgram(const std::shared_ptr<MicroProgramCode[]>& memory) : memory(memory) {}

		inline const MicroProgramCode getMicroCode(const uint8_t& memoryAddress, const uint8_t& opCode) const { return memory[memoryAddress]; }
	};
}

//microprogramm code fmt formatting definition
template<>
struct fmt::formatter<MiMa::MicroProgramCode> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::MicroProgramCode& code, FormatContext& ctx) { return fmt::format_to(ctx.out(), "0x{:07X}", code.bits); }
};
