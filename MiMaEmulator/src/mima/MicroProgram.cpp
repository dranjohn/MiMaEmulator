#include "MicroProgram.h"

#include "StatusBit.h"
#include "debug/Log.h"

#include <vector>
#include <fstream>
#include <limits>
#include <cctype>

namespace MiMa {
	//Utility: checking for letters (matching regex [a-zA-Z]*)
	bool isalphastring(const char* inputString) {
		while (*inputString != 0) {
			if (!isalpha(*inputString)) {
				return false;
			}
		}

		return true;
	}

	//Utility: checking for numbers (matching regex [0-9])
	constexpr char char_0 = 0x30;
	constexpr char char_1 = char_0 + 1;
	bool isbinary(const char& input) {
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


	//Utility: convert register write identifiers to corresponding control bits
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

	//Utility: convert register read identifiers to corresponding control bits
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

	//Utility: constants for compiling
	constexpr uint8_t JUMP_MASK = 0xFF;
	constexpr uint32_t uint32_t_0 = 0;

	//Utility: binary operators understandable by the micro program compiler
	typedef uint32_t(*BinaryOperator)(std::string, std::string);

	BinaryOperator MOVE_OPERATOR = [](std::string leftOperand, std::string rightOperand) {
		return getWriteBit(leftOperand) | getReadBit(rightOperand);
	};
	BinaryOperator SET_OPERATOR = [](std::string leftOperand, std::string rightOperand) {
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

				if (!isdigit(digit)) {
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
		std::shared_ptr<uint32_t[]> memory;
		uint8_t& firstFree;

		//current line of code encoding
		bool fixedJump = false;
		uint32_t currentCode = 0;
		
		//label tracking
		std::map<std::string, uint8_t>& labels;
		std::multimap<std::string, uint8_t>& unresolvedLabels;

		//binary operator buffers
		std::string bufferedOperand;
		BinaryOperator bufferedOperator;
		bool operatorBufferOccupied = false;
	public:
		MicroCodeBuilder(std::shared_ptr<uint32_t[]>& memory, uint8_t& startingPoint, std::map<std::string, uint8_t>& labels, std::multimap<std::string, uint8_t>& unresolvedLabels) :
			memory(memory),
			firstFree(startingPoint),
			labels(labels),
			unresolvedLabels(unresolvedLabels),
			bufferedOperator([](std::string, std::string) { return uint32_t_0; })
		{
			MIMA_LOG_INFO("Initializing microcode builder, starting compilation at 0x{:02X}", startingPoint);
		};


		static bool isControl(const char& control) {
			//check if a char is a control character usable for addToken()
			return control == LABEL
				|| control == MOVE
				|| control == SET
				|| control == BREAK;
		}

		void addToken(const char& control, char* token) {
			switch (control) {
			case LABEL:
				{
					MIMA_LOG_TRACE("Found label '{}' at address 0x{:02X}", token, firstFree);
					//add the label to the list of found labels
					std::string label(token);
					labels.insert({ label, firstFree });

					//resolve any unresolved jumps to this label
					std::multimap<std::string, uint8_t>::iterator unresolvedLabelLocations = unresolvedLabels.find(label);

					if (unresolvedLabelLocations != unresolvedLabels.end()) {
						while (unresolvedLabelLocations->first == label) {
							memory[unresolvedLabelLocations->second] &= ~JUMP_MASK;
							memory[unresolvedLabelLocations->second] |= firstFree;

							unresolvedLabelLocations++;

							MIMA_LOG_TRACE("Resolved jump from 0x{:02X} to 0x{:02X}", unresolvedLabelLocations->second, firstFree);
						}
					}
				}
				break;

			case MOVE:
				//prepare for second operand of move instruction in next addToken() call
				bufferedOperand = std::string(token);
				bufferedOperator = MOVE_OPERATOR;
				operatorBufferOccupied = true;
				break;

			case SET:
				//prepare for second operand of set instruction in next addToken() call
				bufferedOperand = std::string(token);
				bufferedOperator = SET_OPERATOR;
				operatorBufferOccupied = true;
				break;

			case BREAK:
				//if the token is empty, the break operator stands for end-of-line
				if (*token == 0) {
					if (!fixedJump) {
						MIMA_LOG_TRACE("Setting automatic jump to next address 0x{:02X} for microprogram instruction 0x{:08X}", firstFree + 1, currentCode);
						currentCode |= (firstFree + 1);
					}

					memory[firstFree] = currentCode;
					MIMA_LOG_TRACE("Compiled microcode 0x{:08X}, stored at 0x{:02X}", currentCode, firstFree);

					fixedJump = false;
					firstFree++;
					MIMA_ASSERT_WARN(firstFree != 0, "Memory position overflow in compilation, continuing to compile at 0x00.");
					currentCode = 0;

					break;
				}

				//if the token starts with the jump symbol, treat it as a jump instruction
				if (*token == JUMP) {
					if (fixedJump) {
						MIMA_LOG_WARN("Found multiple jump instruction in one line of microprogram code, ignoring jump instruction to '{}'", token);
						break;
					}

					//find label location (ignoring the jump marker)
					std::string label(token + 1);
					std::map<std::string, uint8_t>::iterator labelLocation = labels.find(label);

					if (labelLocation != labels.end()) { //label found
						MIMA_LOG_TRACE("Found microprogram jump instruction from 0x{:02X} to 0x{:02X}", firstFree, labelLocation->second);
						currentCode &= ~JUMP_MASK;
						currentCode |= labelLocation->second;
					}
					else { //label not found, add this to unresolved references
						MIMA_LOG_TRACE("Found unresolved microprogram jump from 0x{:02X}", firstFree);
						unresolvedLabels.insert({ token, firstFree });
					}

					//mark this instruction as having a fixed jump location
					fixedJump = true;
					break;
				}
				
				MIMA_ASSERT_WARN(operatorBufferOccupied, "Found token '{}' with no preceeding binary operator", token);
				if (operatorBufferOccupied) {
					//execute binary operation
					std::string operand(token);
					currentCode |= bufferedOperator(bufferedOperand, operand);

					bufferedOperand = std::string();
					operatorBufferOccupied = false;
				}
				break;
			default:
				MIMA_LOG_ERROR("Found unknown control symbol '{}'", control);
				break;
			}
		}


		void finish(char* remaining) {
			MIMA_ASSERT_ERROR(!operatorBufferOccupied, "Binary operator remaining after finishing compilation with operand '{}'", bufferedOperand);
			MIMA_ASSERT_ERROR(currentCode == 0, "Failed to write last line in compilation 0x({:08X}) into dedicated memory position 0x{:02X}", currentCode, firstFree);

			MIMA_ASSERT_WARN(*remaining == 0, "Found '{}' after finishing compilation", remaining);

			MIMA_LOG_INFO("Finished microprogram compilation at 0x{:02X}", firstFree);
		}
	};



	MicroProgram::MicroProgram(const uint8_t& startingPoint) : memory(new uint32_t[256]) , firstFree(startingPoint) {
		MIMA_LOG_INFO("Created microprogram starting at 0x{:02X}", startingPoint);
		MIMA_LOG_WARN("Using the microprogram compiler is not thread-safe!");
	}


	//Interface: read input from a pointer to a char array containing the code for the program.
	void MicroProgram::compile(char* microProgramCode) {
		MIMA_LOG_INFO("Compiling microprogram from given code string");

		BufferedCharStream microProgramCodeStream(microProgramCode);
		compile(microProgramCodeStream);
	}

	//Interface: read input from an inputstream providing the code for the program.
	void MicroProgram::compile(std::istream& microProgramCode) {
		MIMA_LOG_INFO("Starting microprogram compilation");
		
		InputCharStream microProgramCodeStream(microProgramCode);
		compile(microProgramCodeStream);
	}


	//Interface: read input from a file containing the code for the program.
	void MicroProgram::compileFile(const char* fileName) {
		MIMA_LOG_INFO("Compiling microprogram from file '{}'", fileName);

		std::ifstream fileInput;
		fileInput.open(fileName);

		compile(fileInput);

		fileInput.close();
	}


	void MicroProgram::compile(CharStream& microProgramCodeStream) {
		//initialize token buffer
		std::vector<char> tokenBuffer;

		//initialize micro code builder
		MicroCodeBuilder codeBuilder(memory, firstFree, labels, unresolvedLabels);
		char input;

		//check for control token
		while (input = microProgramCodeStream.get()) {
			if (isspace(input)) {
				continue;
			}

			//control character terminate tokens
			if (MicroCodeBuilder::isControl(input)) {
				tokenBuffer.push_back(0);

				MIMA_LOG_TRACE("Found microprogram token: '{}'", tokenBuffer.data());
				MIMA_LOG_TRACE("Found microprogram control character: '{}'", input);

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


	std::shared_ptr<uint32_t[]> MicroProgram::getMemory() {
		MIMA_ASSERT_WARN(unresolvedLabels.empty(), "Microprogram memory with unresolved labels has been requested");
		return memory;
	}


	void MicroProgram::printLabels() const {
		for (auto const& [label, loc] : labels) {
			MIMA_LOG_INFO("Found label: {} at 0x{:02X}", label, loc);
		}
	}

	void MicroProgram::printCompileStart() const {
		MIMA_LOG_INFO("Compilation start is at 0x{:02X}", firstFree);
	}
}
