#include "MicroProgram.h"

#include "MinimalMachine.h"

#include <map>
#include <vector>
#include <stack>
#include <string>

namespace MiMa {
	//Utility: checking for white spaces
	constexpr char* WHITESPACES = " \t\n";
	bool isWhiteSpace(const char& input) {
		char* whitespace = WHITESPACES;
		while (*whitespace != NULL) {
			if (input == *whitespace) {
				return true;
			}

			whitespace++;
		}

		return false;
	}

	//Utility: checking for letters (matching regex [a-zA-Z])
	constexpr char char_a = 0x41;
	constexpr char char_z = 0x5A;
	constexpr char char_A = 0x61;
	constexpr char char_Z = 0x7A;
	bool isAlphabetic(const char& input) {
		return (char_a <= input && input <= char_z) || (char_A <= input && input <= char_Z);
	}

	//Utility: checking for numbers (matching regex [0-9])
	constexpr char char_0 = 0x30;
	constexpr char char_1 = char_0 + 1;
	bool isNumber(const char& input) {
		return (char_0 <= input && input <= char_0 + 10);
	}
	bool isBinary(const char& input) {
		return input == char_0 || input == char_1;
	}

	//Utility: control symbols
	constexpr char LABEL = ':';
	constexpr char JUMP = '#';
	constexpr char MOVE = '>';
	constexpr char SET = '=';
	constexpr char BREAK = ';';

	//Utility: ALU codes
	uint32_t getALUCode(const std::string& input) {
		if (input == "ADD")
			return 1;
		if (input == "RROT")
			return 2;
		if (input == "AND")
			return 3;
		if (input == "OR")
			return 4;
		if (input == "XOR")
			return 5;
		if (input == "NOT")
			return 6;
		if (input == "EQL")
			return 7;

		return 0;
	}


	uint32_t getWriteBit(const std::string& identifier) {
		if (identifier == "SDR")
			return StatusBit::SDR_WRITING;
		if (identifier == "IR")
			return StatusBit::IR_WRITING;
		if (identifier == "IAR")
			return StatusBit::IAR_WRITING;
		if (identifier == "ONE")
			return StatusBit::ONE;
		if (identifier == "Z")
			return StatusBit::ALU_RESULT;
		if (identifier == "ACCU")
			return StatusBit::ACCUMULATOR_WRITING;

		return 0;
	}

	uint32_t getReadBit(const std::string& identifier) {
		if (identifier == "SAR")
			return StatusBit::SAR_READING;
		if (identifier == "SDR")
			return StatusBit::SDR_READING;
		if (identifier == "IR")
			return StatusBit::IR_READING;
		if (identifier == "IAR")
			return StatusBit::IAR_READING;
		if (identifier == "X")
			return StatusBit::ALU_LEFT_OPERAND;
		if (identifier == "Y")
			return StatusBit::ALU_RIGHT_OPERAND;
		if (identifier == "ACCU")
			return StatusBit::ACCUMULATOR_READING;

		return 0;
	}

	constexpr uint8_t JUMP_MASK = 0xFF;
	constexpr uint32_t uint32_t_0 = 0;

	uint32_t(*MOVE_OPERATOR)(std::string, std::string) = [](std::string leftOperand, std::string rightOperand) {
		return getWriteBit(leftOperand) | getReadBit(rightOperand);
	};
	uint32_t(*SET_OPERATOR)(std::string, std::string) = [](std::string leftOperand, std::string rightOperand) {
		if (leftOperand == "R") {
			if (rightOperand == "1") {
				return static_cast<typename std::underlying_type<StatusBit>::type>(StatusBit::STORAGE_READING);
			}
			else {
				return uint32_t_0;
			}
		}

		if (leftOperand == "W") {
			if (rightOperand == "1") {
				return static_cast<typename std::underlying_type<StatusBit>::type>(StatusBit::STORAGE_WRITING);
			}
			else {
				return uint32_t_0;
			}
		}

		if (leftOperand == "D") {
			char* value = rightOperand.data();
			uint32_t finalValue = 0;

			while (*value != NULL) {
				char digit = *value;
				value++;

				if (!isNumber(digit)) {
					return uint32_t_0;
				}

				finalValue *= 10;
				finalValue += (digit - char_0);
			}

			finalValue &= 0xF;
			finalValue = finalValue << 28;

			return finalValue;
		}

		if (leftOperand == "ALU") {
			return getALUCode(rightOperand) << 12;
		}

		return uint32_t_0;
	};

	class MicroCodeBuilder {
	private:
		//a pointer to the memory to manipulate
		uint32_t* memory;

		bool fixedJump = false;
		uint32_t currentCode = 0;
		uint8_t firstFree;
		
		//label tracking
		std::map<std::string, uint8_t> labels;
		std::multimap<std::string, uint8_t> unresolvedLabels;

		//shunting yard algorithm variables for binary operator input
		std::stack<std::string> operands;
		std::stack<uint32_t(*)(std::string, std::string)> operators;
	public:
		MicroCodeBuilder(uint32_t* memory, uint8_t startingPoint) : memory(memory), firstFree(startingPoint) {};


		bool isControl(char control) {
			return control == LABEL
				|| control == MOVE
				|| control == SET
				|| control == BREAK;
		}

		void addToken(char control, char* token) {
			switch (control) {
			case LABEL:
				{
					std::string label(token);
					
					labels.insert({ label, firstFree });
					std::multimap<std::string, uint8_t>::iterator unresolvedLabelLocations = unresolvedLabels.find(label);

					if (unresolvedLabelLocations != unresolvedLabels.end()) {
						while (unresolvedLabelLocations->first == label) {
							memory[unresolvedLabelLocations->second] &= ~JUMP_MASK;
							memory[unresolvedLabelLocations->second] |= firstFree;

							unresolvedLabelLocations++;
						}
					}
				}
				break;

			case MOVE:
				operands.push(std::string(token));
				operators.push(MOVE_OPERATOR);
				break;

			case SET:
				operands.push(std::string(token));
				operators.push(SET_OPERATOR);
				break;

			case BREAK:
				//if the token is empty, the break operator stands for end-of-line
				if (*token == 0) {
					if (!fixedJump) {
						currentCode |= (firstFree + 1);
					}

					memory[firstFree] = currentCode;

					fixedJump = false;
					firstFree++;
					currentCode = 0;

					break;
				}

				//if the token starts with the jump symbol, treat it as a jump instruction
				if (*token == JUMP) {
					std::string label(token + 1);

					std::map<std::string, uint8_t>::iterator labelLocation = labels.find(label);

					if (labelLocation != labels.end()) {
						currentCode &= ~JUMP_MASK;
						currentCode |= labelLocation->second;
					}
					else {
						unresolvedLabels.insert({ token, firstFree });
					}

					fixedJump = true;
					break;
				}

				//TODO: fix faulty shunting yard for more than one operation
				operands.push(std::string(token));
				while (!operators.empty()) {
					uint32_t(*tokenOperator)(std::string, std::string) = operators.top();
					operators.pop();

					if (operands.size() < 2) {
						//TODO: throw exception
						return;
					}

					std::string rightOperand = operands.top().c_str();
					operands.pop();
					std::string leftOperand = operands.top().c_str();
					operands.pop();

					currentCode |= tokenOperator(leftOperand, rightOperand);
				}

				//TODO: if operands isn't empty, there are mismatched operands
				break;
			}
		}


		void finish(char* remaining) {
			//TODO: ensure clean state
			//TODO: ensure nothing remaining
		}
	};


	void readInput(std::uint32_t* memory, char* microProgramCode, uint8_t startingPoint) {
		//initialize token buffer
		std::vector<char> tokenBuffer;
		
		//initialize micro code builder
		MicroCodeBuilder codeBuilder(memory, startingPoint);

		//check for control token
		while (*microProgramCode != NULL) {
			char input = *microProgramCode;
			microProgramCode++;

			if (isWhiteSpace(input)) {
				continue;
			}

			//control character terminate tokens
			if (codeBuilder.isControl(input)) {
				tokenBuffer.push_back(0);
				codeBuilder.addToken(input, tokenBuffer.data());

				tokenBuffer.clear();
				continue;
			}

			//token not terminated, add current character to the token
			tokenBuffer.push_back(input);
		}

		//add final token
		tokenBuffer.push_back(0);
		codeBuilder.finish(tokenBuffer.data());
	}
}
