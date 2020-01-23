#pragma once

//debugging utility
#include "debug/Log.h"


template<typename Operand, typename Result>
class BinaryOperatorBuffer {
public:
	typedef Result(*Operator)(const Operand&, const Operand&);

private:
	const Operator defaultOperator;
	const Operand defaultLeftOperand;

	Operator bufferedOperator;
	Operand bufferedLeftOperand;
	bool bufferOccupied = false;

public:
	BinaryOperatorBuffer(const Operand& defaultLeftOperand, const Operator& defaultOperator) :
		defaultOperator(defaultOperator),
		bufferedOperator(defaultOperator),
		bufferedLeftOperand(defaultLeftOperand)
	{}


	inline bool isBufferOccupied() const { return bufferOccupied; }
	inline void discardBuffer() { bufferOccupied = false; };

	void buffer(const Operand& leftOperand, const Operator& binaryOperator) {
		bufferedLeftOperand = leftOperand;
		bufferedOperator = binaryOperator;

		bufferOccupied = true;
	}


	Result apply(const Operand& rightOperand) {
		if (!bufferOccupied) {
			MIMA_LOG_WARN("No operator buffered, using default operator and operand to apply to {}", rightOperand);
			return defaultOperator(defaultLeftOperand, rightOperand);
		}

		bufferOccupied = false;
		return bufferedOperator(bufferedLeftOperand, rightOperand);
	}

	Result applyConst(const Operand& rightOperand) const {
		if (!bufferOccupied) {
			MIMA_LOG_WARN("No operator buffered, using default operator and operand to apply to {}", rightOperand);
			return defaultOperator(defaultLeftOperand, rightOperand);
		}

		return bufferedOperator(bufferedLeftOperand, rightOperand);
	}
};
