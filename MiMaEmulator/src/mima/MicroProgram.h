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
		//inline void addBits(const uint32_t& addedBits) { bits |= addedBits; }

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
	typedef void(MicroProgramCode::* MicroProgramCodeSet8BitFunction)(const uint8_t&);
	

	template<size_t opCodeMax>
	class MicroProgramCodeNode;

	template<size_t opCodeMax>
	class MicroProgramCodeList;

	template<size_t opCodeMax>
	struct fmt::formatter<MiMa::MicroProgramCodeList<opCodeMax>>;

	template<size_t opCodeMax>
	class MicroProgramCodeList {
		friend fmt::formatter<MiMa::MicroProgramCodeList<opCodeMax>>;
	private:
		MicroProgramCodeNode<opCodeMax>* head = new MicroProgramCodeNode<opCodeMax>();

	public:
		~MicroProgramCodeList() {
			MicroProgramCodeNode<opCodeMax>* current = head;
			MicroProgramCodeNode<opCodeMax>* next;

			while (current != nullptr) {
				next = current->next;
				delete current;

				current = next;
			}
		}


		void apply(const std::function<void(MicroProgramCode&)>& func, const size_t& lowerLimit = 0, size_t upperLimit = opCodeMax) {
			upperLimit = std::min(upperLimit, opCodeMax);

			MicroProgramCodeNode<opCodeMax>* prev = nullptr;
			MicroProgramCodeNode<opCodeMax>* current = head;

			//find the first node which is affected by the application of the microprogram code function,
			//i.e. the first node which has an upper limit equal or above the lower limit of the effect
			//=> the previous node has an upper limit equal or below the lower limit
			while (current->opCodeUpperLimit < lowerLimit) {
				prev = current;
				current = current->next;
			}

			//in case the previous node has an upper limit below the lower limit
			if (prev == nullptr) {
				if (lowerLimit != 0) {
					size_t limitingNodeLimit = lowerLimit - 1;

					head = new MicroProgramCodeNode<opCodeMax>(head, head->code, limitingNodeLimit);
					prev = head;
				}
			}
			else {
				if (prev->opCodeUpperLimit + 1 != lowerLimit) {
					size_t limitingNodeLimit = lowerLimit - 1;

					prev->next = new MicroProgramCodeNode<opCodeMax>(current, current->code, limitingNodeLimit);
					prev = prev->next;
				}
			}

			//apply function to all nodes with an upper limit under the given upper limit
			while (current->opCodeUpperLimit < upperLimit) {
				func(current->code);

				prev = current;
				current = current->next;
			}

			//if there is a node matching the given upper limit, apply the function to it as well and return
			if (current->opCodeUpperLimit == upperLimit) {
				func(current->code);
				return;
			}

			//if there is no node matching the given upper limit, create one and apply the function to it as well
			MicroProgramCodeNode<opCodeMax>* upperLimitNode = new MicroProgramCodeNode<opCodeMax>(current, current->code, upperLimit);

			if (prev == nullptr) {
				head = upperLimitNode;
			}
			else {
				prev->next = upperLimitNode;
			}
			func(upperLimitNode->code);
		}

		inline void apply(const MicroProgramCodeSetFunction& func, const size_t& lowerLimit = 0, size_t upperLimit = opCodeMax) {
			apply([func](MicroProgramCode& microProgramCode) { std::invoke(func, microProgramCode); }, lowerLimit, upperLimit);
		}

		inline void apply(const MicroProgramCodeSet8BitFunction& func, const uint8_t& value, const size_t& lowerLimit = 0, const size_t& upperLimit = opCodeMax) {
			apply([func, value](MicroProgramCode& microProgramCode) { std::invoke(std::bind(func, std::placeholders::_1, value), microProgramCode); }, lowerLimit, upperLimit);
		}


		MicroProgramCode get(size_t opCode) {
			opCode = std::min(opCode, opCodeMax);
			MicroProgramCodeNode<opCodeMax>* current = head;

			while (current->opCodeUpperLimit < opCode) {
				current = current->next;
			}

			return current->code;
		}
	};

	template<size_t opCodeMax>
	class MicroProgramCodeNode {
		friend class MicroProgramCodeList<opCodeMax>;

	private:
		MicroProgramCodeNode<opCodeMax>* next;

		size_t opCodeUpperLimit;
		MicroProgramCode code;

		MicroProgramCodeNode(MicroProgramCodeNode<opCodeMax>* next = nullptr, MicroProgramCode code = MicroProgramCode(), size_t opCodeUpperLimit = opCodeMax) : next(next), code(code), opCodeUpperLimit(opCodeUpperLimit) {}
	};


	class MicroProgram {
	private:
		//minimal machine microprogram read-only memory
		std::shared_ptr<MicroProgramCodeList<0x100>[]> memory;
	public:
		MicroProgram(const std::shared_ptr<MicroProgramCodeList<0x100>[]>& memory) : memory(memory) {}

		inline const MicroProgramCode getMicroCode(const uint8_t& memoryAddress, const uint8_t& opCode) const { return memory[memoryAddress].get(opCode); }
	};
}

//microprogramm code fmt formatting definition
template<>
struct fmt::formatter<MiMa::MicroProgramCode> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::MicroProgramCode& code, FormatContext& ctx) { return fmt::format_to(ctx.out(), "0x{:07X}", code.bits); }
};

template<size_t opCodeMax>
struct fmt::formatter<MiMa::MicroProgramCodeList<opCodeMax>> {
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }

	template<typename FormatContext>
	auto format(const MiMa::MicroProgramCodeList<opCodeMax>& codeList, FormatContext& ctx) { return fmt::format_to(ctx.out(), "<microprogram code list>"); } //TODO: add meaningful format
};
