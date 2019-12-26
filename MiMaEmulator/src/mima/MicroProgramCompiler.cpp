#include "MicroProgramCompiler.h"

#include "StatusBit.h"

namespace MiMa {
	// ---------------
	// Utility methods
	// ---------------

	//Utility: constants for compiling
	constexpr uint8_t JUMP_MASK = 0xFF;
	constexpr uint32_t uint32_t_0 = 0;


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
	constexpr char char_LABEL = ':';
	constexpr char char_JUMP = '#';
	constexpr char char_MOVE = '>';
	constexpr char char_SET = '=';
	constexpr char char_BREAK = ';';
	constexpr char char_COMPILER_DIRECTIVE = '!';

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

	//Utility: predefined binary operators in compilation
	BinaryOperator MOVE_OPERATOR = [](const std::string& leftOperand, const std::string& rightOperand) {
		return getWriteBit(leftOperand) | getReadBit(rightOperand);
	};
	BinaryOperator SET_OPERATOR = [](const std::string& leftOperand, const std::string& rightOperand) {
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
			uint32_t finalValue = (uint32_t)std::stoi(rightOperand);

			finalValue &= 0xF;
			finalValue = finalValue << 28;

			return finalValue;
		}

		if (leftOperand == "ALU") {
			return getALUCode(rightOperand) << 12;
		}

		return uint32_t_0;
	};



	// -----------------------
	// Scope utility functions
	// -----------------------

	void MicroProgramCompiler::Scope::addLabel(std::string label) {
		MIMA_LOG_TRACE("Found label '{}' at address 0x{:02X}", label, compiler.firstFree);

		//add the label to the list of found labels
		MIMA_ASSERT_ERROR(compiler.labels.find(label) == compiler.labels.end(), "Found duplicate of label {}, using second label found from now on", label);
		compiler.labels.insert({ label, compiler.firstFree });

		//call all listeners waiting for this label to be added
		std::multimap<std::string, std::function<bool(const uint8_t&)>>::iterator labelAddListener = compiler.labelAddListeners.find(label);

		if (labelAddListener == compiler.labelAddListeners.end()) {
			return;
		}

		while (labelAddListener->first == label) {
			if ((labelAddListener->second)(compiler.firstFree)) {
				labelAddListener = compiler.labelAddListeners.erase(labelAddListener);
			}
			else {
				++labelAddListener;
			}

			MIMA_LOG_TRACE("Called listener for label at 0x{:02X}", compiler.firstFree);
		}
	}

	void MicroProgramCompiler::Scope::addJump(std::string label, bool& fixedJump, uint32_t& currentCode, bool overrideFixed) {
		//confirm that this can jump instruction can be set
		if (fixedJump && !overrideFixed) {
			MIMA_LOG_WARN("Attempted to override fixed jump at position 0x{:02X}", compiler.firstFree);
			return;
		}
		MIMA_ASSERT_TRACE(!fixedJump, "Overriding fixed jump at 0x{:02X}", compiler.firstFree);

		//find label location (ignoring the jump marker)
		std::map<std::string, uint8_t>::iterator labelLocation = compiler.labels.find(label);

		if (labelLocation != compiler.labels.end()) { //label found
			MIMA_LOG_TRACE("Found microprogram jump instruction from 0x{:02X} to 0x{:02X}", compiler.firstFree, labelLocation->second);
			currentCode &= ~JUMP_MASK;
			currentCode |= labelLocation->second;
		}
		else { //label not found, add this to unresolved references
			MIMA_LOG_TRACE("Found unresolved microprogram jump from 0x{:02X}", compiler.firstFree);

			uint8_t firstFreeCopy = compiler.firstFree;
			std::shared_ptr<uint32_t[]>& memoryReference = compiler.memory;
			std::function<bool(const uint8_t&)> x = [firstFreeCopy, memoryReference](const uint8_t& labelAddress) {
				memoryReference[firstFreeCopy] &= ~JUMP_MASK;
				memoryReference[firstFreeCopy] |= labelAddress;
				
				return true;
			};
		}

		//mark this instruction as having a fixed jump location
		fixedJump = true;
	}


	void MicroProgramCompiler::Scope::endOfLine(bool& fixedJump, uint32_t& currentCode) {
		if (!fixedJump) {
			MIMA_LOG_TRACE("Setting automatic jump to next address 0x{:02X} for microprogram instruction 0x{:08X}", compiler.firstFree + 1, currentCode);
			currentCode |= (compiler.firstFree + 1);
		}

		compiler.memory[compiler.firstFree] = currentCode;
		MIMA_LOG_TRACE("Compiled microcode 0x{:08X}, stored at 0x{:02X}", currentCode, compiler.firstFree);

		fixedJump = false;
		compiler.firstFree++;
		MIMA_ASSERT_WARN(compiler.firstFree != 0, "Memory position overflow in compilation, continuing to compile at 0x00.");
		currentCode = 0;
	}



	// -----------------------------------------
	// Global scope of the microprogram compiler
	// -----------------------------------------
	
	MicroProgramCompiler::DefaultScope::DefaultScope(MicroProgramCompiler& compiler) :
		Scope(compiler),
		operatorBuffer(std::string(""), [](const std::string&, const std::string&) { return uint32_t_0; })
	{}


	bool MicroProgramCompiler::DefaultScope::isControl(const char& control) {
		return control == char_LABEL
			|| control == char_MOVE
			|| control == char_SET
			|| control == char_BREAK;
	}

	void MicroProgramCompiler::DefaultScope::addToken(const char& control, char* token) {
		switch (control) {
		case char_LABEL:
			Scope::addLabel(token);
			break;

		case char_MOVE:
			//prepare for second operand of move instruction in next addToken() call
			operatorBuffer.buffer(token, MOVE_OPERATOR);
			break;

		case char_SET:
			//prepare for second operand of set instruction in next addToken() call
			operatorBuffer.buffer(token, SET_OPERATOR);
			break;

		case char_BREAK:
			//if the token is empty, the break operator stands for end-of-line
			if (*token == 0) {
				Scope::endOfLine(fixedJump, currentCode);
				break;
			}

			//if the token starts with the jump symbol, treat it as a jump instruction
			if (*token == char_JUMP) {
				if (operatorBuffer.isBufferOccupied()) {
					MIMA_LOG_INFO("Discarding binary operator followed by jump instruction to {}", token);
					operatorBuffer.discardBuffer();
				}

				Scope::addJump(token + 1, fixedJump, currentCode);
				break;
			}

			//if the token is the halt instruction, add the halt code
			if (token == "HALT") {
				if (operatorBuffer.isBufferOccupied()) {
					MIMA_LOG_INFO("Discarding binary operator followed by halt instruction");
					operatorBuffer.discardBuffer();
				}

				currentCode |= HALT_CODE;
				break;
			}

			MIMA_ASSERT_WARN(operatorBuffer.isBufferOccupied(), "Found token '{}' with no preceeding binary operator", token);
			if (operatorBuffer.isBufferOccupied()) {
				//execute binary operation
				currentCode |= operatorBuffer.apply(token);
			}
			break;
		default:
			MIMA_LOG_ERROR("Found unknown control symbol '{}'", control);
			break;
		}
	}


	void MicroProgramCompiler::DefaultScope::cleanUpScope() {
		MIMA_LOG_TRACE("Cleaning up default compiler scope");
	}

	void MicroProgramCompiler::DefaultScope::finish(char* finish) {
		MIMA_ASSERT_ERROR(!operatorBuffer.isBufferOccupied(), "Binary operator remaining after finishing compilation with remaining operand");
		MIMA_ASSERT_ERROR(currentCode == 0, "Failed to write last line in compilation 0x({:08X}) into dedicated memory position 0x{:02X}", currentCode, compiler.firstFree);
	}



	// ---------------------
	// Microprogram compiler
	// ---------------------

	MicroProgramCompiler::MicroProgramCompiler(std::shared_ptr<uint32_t[]>& memory, uint8_t& startingPoint, std::map<std::string, uint8_t>& labels, std::multimap<std::string, std::function<bool(const uint8_t&)>>& labelAddListeners) :
		memory(memory),
		firstFree(startingPoint),
		labels(labels),
		labelAddListeners(labelAddListeners)
	{
		currentScope.reset(new DefaultScope(*this));
		MIMA_LOG_INFO("Initialized microcode builder, starting compilation at 0x{:02X}", startingPoint);
	};


	bool MicroProgramCompiler::isControl(const char& control) {
		//check if a char is a control character usable for addToken()
		return control == char_COMPILER_DIRECTIVE
			|| currentScope->isControl(control);
	}

	void MicroProgramCompiler::addToken(const char& control, char* token) {
		if (control == char_COMPILER_DIRECTIVE) {
			MIMA_LOG_ERROR("Discarding unknwon compiler directive '{}'", token);
			return;
		}

		currentScope->addToken(control, token);
	}


	void MicroProgramCompiler::finish(char* remaining) {
		MIMA_ASSERT_WARN(*remaining == 0, "Found '{}' after finishing compilation", remaining);

		MIMA_LOG_INFO("Finished microprogram compilation at 0x{:02X}", firstFree);
	}
}
