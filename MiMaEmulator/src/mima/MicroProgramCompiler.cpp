#include "MicroProgramCompiler.h"

#include "StatusBit.h"

namespace MiMa {
	// ---------------
	// Utility methods
	// ---------------

	//Utility: constants for compiling
	constexpr uint8_t JUMP_MASK = 0xFF;
	constexpr uint32_t uint32_t_0 = 0;
	constexpr uint8_t HALT_RESERVED = 0xFF;


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
	static const std::function<void(MicroProgramCode&)> x = [](MicroProgramCode&) {};

	BinaryOperator MOVE_OPERATOR = [](const std::string& leftOperand, const std::string& rightOperand) {
		MicroProgramCodeModifier bitSetter = [leftOperand, rightOperand](MicroProgramCode& microProgramCode) {
			microProgramCode.addBits(getWriteBit(leftOperand) | getReadBit(rightOperand));
		};
		return bitSetter;
	};
	BinaryOperator SET_OPERATOR = [](const std::string& leftOperand, const std::string& rightOperand) {
		uint32_t bits = uint32_t_0;

		if (leftOperand == "R") {
			if (rightOperand == "1") {
				bits = static_cast<typename std::underlying_type<StatusBit>::type>(StatusBit::STORAGE_READING);
			}
		}

		if (leftOperand == "W") {
			if (rightOperand == "1") {
				bits = static_cast<typename std::underlying_type<StatusBit>::type>(StatusBit::STORAGE_WRITING);
			}
		}

		if (leftOperand == "D") {
			bits = (uint32_t)std::stoi(rightOperand);

			bits &= 0xF;
			bits = bits << 28;
		}

		if (leftOperand == "ALU") {
			bits = getALUCode(rightOperand) << 12;
		}

		MicroProgramCodeModifier bitSetter = [bits](MicroProgramCode& microProgramCode) {
			microProgramCode.addBits(bits);
		};
		return bitSetter;
	};



	// ------------------------------
	// Compile mode utility functions
	// ------------------------------

	void MicroProgramCompiler::CompileMode::addLabel(std::string label) {
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

	void MicroProgramCompiler::CompileMode::addJump(std::string label, bool& fixedJump, MicroProgramCode& currentCode, bool overrideFixed) {
		//confirm that this can jump instruction can be set
		if (fixedJump && !overrideFixed) {
			MIMA_LOG_WARN("Attempted to override fixed jump at position 0x{:02X}", compiler.firstFree);
			return;
		}
		MIMA_ASSERT_TRACE(!fixedJump, "Overriding fixed jump at 0x{:02X}", compiler.firstFree);

		//find label location (ignoring the jump marker)
		std::unordered_map<std::string, uint8_t>::iterator labelLocation = compiler.labels.find(label);

		if (labelLocation != compiler.labels.end()) { //label found
			MIMA_LOG_TRACE("Found microprogram jump instruction from 0x{:02X} to 0x{:02X}", compiler.firstFree, labelLocation->second);
			currentCode.setJump(labelLocation->second);
		}
		else { //label not found, add this to unresolved references
			MIMA_LOG_TRACE("Found unresolved microprogram jump from 0x{:02X}", compiler.firstFree);

			uint8_t firstFreeCopy = compiler.firstFree;
			std::shared_ptr<MicroProgramCode[]>& memoryReference = compiler.memory;
			std::function<bool(const uint8_t&)> x = [firstFreeCopy, memoryReference](const uint8_t& labelAddress) {
				memoryReference[firstFreeCopy].setJump(labelAddress);
				
				return true;
			};
		}

		//mark this instruction as having a fixed jump location
		fixedJump = true;
	}


	void MicroProgramCompiler::CompileMode::endOfLine(bool& fixedJump, MicroProgramCode& currentCode) {
		if (!fixedJump) {
			MIMA_LOG_TRACE("Setting automatic jump to next address 0x{:02X} for microprogram instruction {}", compiler.firstFree + 1, currentCode);
			currentCode.setJump(compiler.firstFree + 1);
		}

		MIMA_LOG_TRACE("Compiled at 0x{:02X}: microcode {}", compiler.firstFree, currentCode);

		fixedJump = false;
		compiler.firstFree++;
		MIMA_ASSERT_WARN(compiler.firstFree != 0, "Memory position overflow in compilation, continuing to compile at 0x00.");

		compiler.lineStart = true;
	}



	// ----------------------------------------------------------
	// Default (global) compile mode of the microprogram compiler
	// ----------------------------------------------------------
	
	MicroProgramCompiler::DefaultCompileMode::DefaultCompileMode(MicroProgramCompiler& compiler) :
		CompileMode(compiler),
		operatorBuffer(std::string(""), [](const std::string&, const std::string&) { return x; })
	{}


	bool MicroProgramCompiler::DefaultCompileMode::isControl(const char& control) {
		return control == char_LABEL
			|| control == char_MOVE
			|| control == char_SET
			|| control == char_BREAK;
	}

	void MicroProgramCompiler::DefaultCompileMode::addToken(const char& control, char* token) {
		switch (control) {
		case char_LABEL:
			CompileMode::addLabel(token);
			break;

		case char_MOVE:
			//prepare for second operand of move instruction in next addToken() call
			operatorBuffer.buffer(token, MOVE_OPERATOR);
			compiler.lineStart = false;
			break;

		case char_SET:
			//prepare for second operand of set instruction in next addToken() call
			operatorBuffer.buffer(token, SET_OPERATOR);
			compiler.lineStart = false;
			break;

		case char_BREAK:
			//if the token is empty, the break operator stands for end-of-line
			if (*token == 0) {
				CompileMode::endOfLine(fixedJump, compiler.memory[compiler.firstFree]);
				break;
			}

			//if the token starts with the jump symbol, treat it as a jump instruction
			if (*token == char_JUMP) {
				if (operatorBuffer.isBufferOccupied()) {
					MIMA_LOG_INFO("Discarding binary operator followed by jump instruction to {}", token);
					operatorBuffer.discardBuffer();
				}

				CompileMode::addJump(token + 1, fixedJump, compiler.memory[compiler.firstFree]);
				compiler.lineStart = false;
				break;
			}

			MIMA_ASSERT_WARN(operatorBuffer.isBufferOccupied(), "Found token '{}' with no preceeding binary operator", token);
			if (operatorBuffer.isBufferOccupied()) {
				//execute binary operation
				operatorBuffer.apply(token)(compiler.memory[compiler.firstFree]);
			}
			break;
		default:
			MIMA_LOG_ERROR("Found unknown control symbol '{}'", control);
			break;
		}
	}


	void MicroProgramCompiler::DefaultCompileMode::closeCompileMode() {
		MIMA_LOG_TRACE("Closing default compiler mode");
	}

	void MicroProgramCompiler::DefaultCompileMode::finish(char* finish) {
		MIMA_ASSERT_ERROR(!operatorBuffer.isBufferOccupied(), "Binary operator remaining after finishing compilation with remaining operand");
		//MIMA_ASSERT_ERROR(currentCode == 0, "Failed to write last line in compilation 0x({:08X}) into dedicated memory position 0x{:02X}", currentCode, compiler.firstFree);
	}



	// ---------------------
	// Microprogram compiler
	// ---------------------

	MicroProgramCompiler::MicroProgramCompiler() : memory(new MicroProgramCode[0x100]) {
		currentCompileMode.reset(new DefaultCompileMode(*this));

		//0xFF is reserved for halt
		labels.insert({ "halt", HALT_RESERVED });
		memory[HALT_RESERVED].setJump(HALT_RESERVED);

		MIMA_LOG_INFO("Initialized microcode compiler");
	};


	bool MicroProgramCompiler::isControl(const char& control) {
		//check if a char is a control character usable for addToken()
		return control == char_COMPILER_DIRECTIVE
			|| currentCompileMode->isControl(control);
	}

	void MicroProgramCompiler::addToken(const char& control, char* token) {
		//if the control char is not a compiler directive indicator, pass it to the current compile mode
		if (control != char_COMPILER_DIRECTIVE) {
			currentCompileMode->addToken(control, token);
			return;
		}


		//otherwise, execute compiler directive
		if (token == "cm_default") {
			if (!lineStart) {
				MIMA_LOG_ERROR("Discarding invalid compiler directive '!cm_default' which is not a line start");
				return;
			}

			currentCompileMode->closeCompileMode();
			currentCompileMode.reset(new DefaultCompileMode(*this));

			MIMA_LOG_TRACE("Switched to a default compiler compile mode");
			return;
		}

		if (token == "cm_conditionalDecode") {
			if (!lineStart) {
				MIMA_LOG_ERROR("Discarding invalid compiler directive '!cm_decode' which is not a line start");
				return;
			}

			currentCompileMode->closeCompileMode();

			MIMA_LOG_TRACE("Switched to a conditional decode compiler compile mode");
			return;
		}

		MIMA_LOG_ERROR("Discarding unknwon compiler directive '{}'", token);
	}


	MicroProgram MicroProgramCompiler::finish(char* remaining) {
		MIMA_ASSERT_WARN(*remaining == 0, "Found '{}' after finishing compilation", remaining);

		MIMA_LOG_INFO("Finished microprogram compilation at 0x{:02X}", firstFree);

		MicroProgram program(memory);
		return program;
	}



	// ---------------------
	// Compilation interface
	// ---------------------

	//Interface: read input from a pointer to a char array containing the code for the program.
	MicroProgram MicroProgramCompiler::compile(char* microProgramCode) {
		MIMA_LOG_INFO("Compiling microprogram from given code string");

		BufferedCharStream microProgramCodeStream(microProgramCode);
		return compile(microProgramCodeStream);
	}

	//Interface: read input from an inputstream providing the code for the program.
	MicroProgram MicroProgramCompiler::compile(std::istream& microProgramCode) {
		MIMA_LOG_INFO("Starting microprogram compilation");

		InputCharStream microProgramCodeStream(microProgramCode);
		return compile(microProgramCodeStream);
	}


	//Interface: read input from a file containing the code for the program.
	MicroProgram MicroProgramCompiler::compileFile(char* fileName) {
		std::ifstream file;
		file.open(fileName);

		MicroProgram program = compile(file);

		file.close();
		return program;
	}


	MicroProgram MicroProgramCompiler::compile(CharStream& microProgramCodeStream) {
		//initialize token buffer
		std::vector<char> tokenBuffer;

		//initialize micro code builder
		MicroProgramCompiler compiler;
		char input;

		//check for control token
		while (input = microProgramCodeStream.get()) {
			if (isspace(input)) {
				continue;
			}

			//control character terminate tokens
			if (compiler.isControl(input)) {
				tokenBuffer.push_back(0);

				MIMA_LOG_TRACE("Found microprogram token: '{}'", tokenBuffer.data());
				MIMA_LOG_TRACE("Found microprogram control character: '{}'", input);

				compiler.addToken(input, tokenBuffer.data());

				tokenBuffer.clear();
				continue;
			}

			//token not terminated, add current character to the token
			tokenBuffer.push_back(input);
		}

		//add final token
		tokenBuffer.push_back(0);
		MicroProgram program = compiler.finish(tokenBuffer.data());

		return program;
	}
}
