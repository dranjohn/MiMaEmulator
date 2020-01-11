#include "MicroProgramCompiler.h"

#include <regex>

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
	constexpr char char_REGISTER_TRANSFER = '>';
	constexpr char char_SET = '=';

	constexpr char char_RANGE_LOWER_LIMIT = '+';
	constexpr char char_RANGE_UPPER_LIMIT = '-';
	constexpr char char_RANGE_POINT = '=';

	constexpr char char_BREAK = ';';
	constexpr char char_COMPILER_DIRECTIVE = '!';

	//Utility: ALU codes
	uint32_t getALUCode(const std::string& input) {
		if (input == "ADD")
			return 1;
		if (input == "RAR")
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
	MicroProgramCodeSetFunction getWriteBit(const std::string& identifier) {
		if (identifier == "SDR")
			return &MicroProgramCode::setStorageDataRegisterWriting;
		if (identifier == "IR")
			return &MicroProgramCode::setInstructionRegisterWriting;
		if (identifier == "IAR")
			return &MicroProgramCode::setInstructionAddressRegisterWriting;
		if (identifier == "ONE")
			return &MicroProgramCode::setConstantOneWriting;
		if (identifier == "Z")
			return &MicroProgramCode::setALUResultWriting;
		if (identifier == "ACCU")
			return &MicroProgramCode::setAccumulatorRegisterWriting;

		return &MicroProgramCode::pass;
	}

	//Utility: convert register read identifiers to corresponding control bits
	MicroProgramCodeSetFunction getReadBit(const std::string& identifier) {
		if (identifier == "SAR")
			return &MicroProgramCode::setStorageAddressRegisterReading;
		if (identifier == "SDR")
			return &MicroProgramCode::setStorageDataRegisterReading;
		if (identifier == "IR")
			return &MicroProgramCode::setInstructionRegisterReading;
		if (identifier == "IAR")
			return &MicroProgramCode::setInstructionAddressRegisterReading;
		if (identifier == "X")
			return &MicroProgramCode::setLeftALUOperandReading;
		if (identifier == "Y")
			return &MicroProgramCode::setRightALUOperandReading;
		if (identifier == "ACCU")
			return &MicroProgramCode::setAccumulatorRegisterReading;

		return &MicroProgramCode::pass;
	}

	//Utility: predefined binary operators in compilation
	static const std::function<void(MicroProgramCode&)> NO_MICROPROGRAM_CODE_MODIFICATION = [](MicroProgramCode&) {};

	BinaryOperator MOVE_OPERATOR = [](const std::string& leftOperand, const std::string& rightOperand)->MicroProgramCodeModifier {
		//get the read and write functions for the microprogram code modification
		MicroProgramCodeSetFunction setWrite = getWriteBit(leftOperand);
		MicroProgramCodeSetFunction setRead = getReadBit(rightOperand);

		//return a modifier executing both these functions
		MicroProgramCodeModifier registerTransfer = [setWrite, setRead](MicroProgramCode& microProgramCode) {
			(microProgramCode.*setWrite)();
			(microProgramCode.*setRead)();
		};
		return registerTransfer;
	};
	BinaryOperator SET_OPERATOR = [](const std::string& leftOperand, const std::string& rightOperand)->MicroProgramCodeModifier {
		//the left operand "R" marks that storage read is being (un)set
		if (leftOperand == "R") {
			if (rightOperand == "1") {
				return [](MicroProgramCode& code) { code.enableMemoryRead(); };
			}
			if (rightOperand == "0") {
				return [](MicroProgramCode& code) { code.disableMemoryRead(); };
			}
			return NO_MICROPROGRAM_CODE_MODIFICATION;
		}

		//the left operand "R" marks that storage write is being (un)set
		if (leftOperand == "W") {
			if (rightOperand == "1") {
				return [](MicroProgramCode& code) { code.enableMemoryWrite(); };
			}
			if (rightOperand == "0") {
				return [](MicroProgramCode& code) { code.disableMemoryWrite(); };
			}
			return NO_MICROPROGRAM_CODE_MODIFICATION;
		}

		//the left operand "ALU" marks that an ALU code (in [0, 7]) is being set
		if (leftOperand == "ALU") {
			uint8_t ALUCode = getALUCode(rightOperand);
			return [ALUCode](MicroProgramCode& code) { code.setALUCode(ALUCode); };
		}

		//if none of these things is being set, return no modification
		return NO_MICROPROGRAM_CODE_MODIFICATION;
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
		std::multimap<std::string, std::function<bool(const uint8_t&)>>::iterator mapEnd = compiler.labelAddListeners.end();

		//iterate over listeners
		while (labelAddListener != mapEnd && labelAddListener->first == label) {
			if ((labelAddListener->second)(compiler.firstFree)) {
				//if calling the listener returns true, remove it from the listener list and move on to the next
				labelAddListener = compiler.labelAddListeners.erase(labelAddListener++);
			}
			else {
				//otherwise, just move on to the next
				++labelAddListener;
			}

			MIMA_LOG_TRACE("Called listeners for label at 0x{:02X}", compiler.firstFree);
		}
	}

	void MicroProgramCompiler::CompileMode::addJump(std::string label, bool& fixedJump, MicroProgramCodeList& currentCode, bool overrideFixed) {
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
			currentCode.apply(&MicroProgramCode::setJump, labelLocation->second, 0, 0xFF);
		}
		else { //label not found, add this to unresolved references
			MIMA_LOG_TRACE("Found unresolved microprogram jump from 0x{:02X}", compiler.firstFree);

			//create a listener adding a jump once this label is found
			uint8_t firstFreeCopy = compiler.firstFree;
			std::shared_ptr<MicroProgramCodeList[]>& memoryReference = compiler.memory;

			std::function<bool(const uint8_t&)> labelAddListener = [firstFreeCopy, memoryReference](const uint8_t& labelAddress) {
				memoryReference[firstFreeCopy].apply(&MicroProgramCode::setJump, labelAddress, 0, 0xFF);
				
				return true;
			};

			//add the listener to the list of listeners
			compiler.labelAddListeners.insert({ label, labelAddListener });
		}

		//mark this instruction as having a fixed jump location
		fixedJump = true;
	}

	void MicroProgramCompiler::CompileMode::addJump(std::string label, MicroProgramCodeList& currentCode, const size_t& lowerLimit, const size_t& upperLimit) {
		//find label location (ignoring the jump marker)
		std::unordered_map<std::string, uint8_t>::iterator labelLocation = compiler.labels.find(label);

		if (labelLocation != compiler.labels.end()) { //label found
			MIMA_LOG_TRACE("Found microprogram jump instruction from 0x{:02X} to 0x{:02X} in range from 0x{:08X} to 0x{:08X}", compiler.firstFree, labelLocation->second, lowerLimit, upperLimit);
			currentCode.apply(&MicroProgramCode::setJump, labelLocation->second, lowerLimit, upperLimit);
		}
		else { //label not found, add this to unresolved references
			MIMA_LOG_TRACE("Found unresolved microprogram jump from 0x{:02X} for range from 0x{:08X} to 0x{:08X}", compiler.firstFree, lowerLimit, upperLimit);

			//create a listener adding a jump once this label is found
			uint8_t firstFreeCopy = compiler.firstFree;
			std::shared_ptr<MicroProgramCodeList[]>& memoryReference = compiler.memory;

			std::function<bool(const uint8_t&)> labelAddListener = [firstFreeCopy, memoryReference, lowerLimit, upperLimit](const uint8_t& labelAddress) {
				memoryReference[firstFreeCopy].apply(&MicroProgramCode::setJump, labelAddress, lowerLimit, upperLimit);
				MIMA_LOG_TRACE("Resolved label at 0x{:02X} to 0x{:02X} for range from 0x{:02X} to 0x{:02X}, current value: {}", firstFreeCopy, labelAddress, lowerLimit, upperLimit, memoryReference[firstFreeCopy]);

				return true;
			};

			//add the listener to the list of listeners
			compiler.labelAddListeners.insert({ label, labelAddListener });
		}
	}


	void MicroProgramCompiler::CompileMode::endOfLine(MicroProgramCodeList& currentCode) {
		MIMA_LOG_TRACE("Compiled at 0x{:02X}: microcode {}", compiler.firstFree, currentCode);

		//increment location of the first free memory spot to write into
		compiler.firstFree++;
		compiler.firstFree %= 0x100;
		MIMA_ASSERT_WARN(compiler.firstFree != 0, "Memory position overflow in compilation, continuing to compile at 0x00.");

		//the compiler is now again at a line start
		compiler.lineStart = true;
	}

	void MicroProgramCompiler::CompileMode::endOfLine(bool& fixedJump, MicroProgramCodeList& currentCode) {
		//set jump to next if no fixed jump was given
		if (!fixedJump) {
			MIMA_LOG_TRACE("Setting automatic jump to next address 0x{:02X} for microprogram instruction {}", compiler.firstFree + 1, currentCode);
			currentCode.apply(&MicroProgramCode::setJump, compiler.firstFree + 1, 0, 0xFF);
		}

		//reset the fixedJump variable to false for the next line of code
		fixedJump = false;

		//remaining end-of-line responsibilities (which don't concern jumping)
		endOfLine(currentCode);
	}


	void MicroProgramCompiler::CompileMode::finish(char* remaining) {
		//close compiler mode
		closeCompileMode();

		//assert that there no trailing characters, the last character in the code should be a control token (only followed by whitespaces)
		MIMA_ASSERT_ERROR(*remaining == 0, "Found trailing tokens {} after finishing compilation", remaining);
	}



	// ----------------------------------------------------------
	// Default (global) compile mode of the microprogram compiler
	// ----------------------------------------------------------
	
	MicroProgramCompiler::DefaultCompileMode::DefaultCompileMode(MicroProgramCompiler& compiler) :
		CompileMode(compiler),
		operatorBuffer(std::string(""), [](const std::string&, const std::string&) { return NO_MICROPROGRAM_CODE_MODIFICATION; })
	{
		//reset code memory where the program is written too
		compiler.memory[compiler.firstFree].reset();

		MIMA_LOG_TRACE("Opened default compiler mode");
	}


	bool MicroProgramCompiler::DefaultCompileMode::isControl(const char& control) {
		//in default compile:
		return control == char_LABEL // labels can be added
			|| control == char_REGISTER_TRANSFER // register transfers can be added
			|| control == char_SET // booleans can be set
			|| control == char_BREAK; // break character for controlling code structure
	}

	void MicroProgramCompiler::DefaultCompileMode::addToken(const char& control, char* token) {
		switch (control) {
		case char_LABEL:
			CompileMode::addLabel(token);
			break;

		case char_REGISTER_TRANSFER:
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
				compiler.memory[compiler.firstFree].reset();
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
				compiler.memory[compiler.firstFree].apply(operatorBuffer.apply(token), 0, 0xFF);
			}
			break;
		default:
			MIMA_LOG_ERROR("Found unknown control symbol '{}'", control);
			break;
		}
	}


	void MicroProgramCompiler::DefaultCompileMode::closeCompileMode() {
		//ensure the last line has been closed properly
		if (!compiler.lineStart) {
			compiler.memory[compiler.firstFree].reset();
			MIMA_LOG_ERROR("Unfinished microprogram code line at 0x{:02X} has been reset", compiler.firstFree);
		}

		//ensure there is nothing remaining in the binary operator buffer
		MIMA_ASSERT_ERROR(!operatorBuffer.isBufferOccupied(), "Binary operator remaining after finishing compilation with remaining operand");


		//log closing of the default compiler mode
		MIMA_LOG_TRACE("Closed default compiler mode");
	}



	// ----------------------------------------------------------
	// Conditional jump compile mode of the microprogram compiler
	// ----------------------------------------------------------

	MicroProgramCompiler::ConditionalCompileMode::ConditionalCompileMode(MicroProgramCompiler& compiler, const std::string& conditionName, const size_t& conditionMax) :
		CompileMode(compiler),
		conditionName(conditionName),
		conditionMax(conditionMax),
		lowerLimit(0),
		upperLimit(conditionMax)
	{
		compiler.memory[compiler.firstFree].reset(conditionName, conditionMax);
		MIMA_LOG_TRACE("Opened conditional jump compiler mode for condition '{}' up to 0x{:X}", conditionName, conditionMax);
	}


	bool MicroProgramCompiler::ConditionalCompileMode::isControl(const char& control) {
		//in default compile:
		return control == char_RANGE_LOWER_LIMIT // the lower condition limit of an instruction can be set
			|| control == char_RANGE_UPPER_LIMIT // the upper condition limit of an instruction can be set
			|| control == char_RANGE_POINT  // the lower and upper condition limit of an instruction can be set in one command
			|| control == char_BREAK; // break character for controlling code structure
	}

	void MicroProgramCompiler::ConditionalCompileMode::addToken(const char& control, char* token) {
		switch (control) {
			//limit setting
		case char_RANGE_LOWER_LIMIT:
			lowerLimit = std::stoi(token);
			break;
		case char_RANGE_UPPER_LIMIT:
			upperLimit = std::stoi(token);
			break;
		case char_RANGE_POINT:
			lowerLimit = std::stoi(token);
			upperLimit = std::stoi(token);
			break;

		case char_BREAK:
			//if the token is empty, the break operator stands for end-of-line
			if (*token == 0) {
				CompileMode::endOfLine(compiler.memory[compiler.firstFree]);
				compiler.memory[compiler.firstFree].reset(conditionName, conditionMax);
				break;
			}

			//if the token starts with a #, it's a jump instruction
			if (*token == '#') {
				token++;
				CompileMode::addJump(token, compiler.memory[compiler.firstFree], lowerLimit, upperLimit);

				lowerLimit = 0;
				upperLimit = conditionMax;
				break;
			}

			//if the token is "next", the jump is supposed to go to the next instruction
			if (std::string(token) == "next") {
				compiler.memory[compiler.firstFree].apply(&MicroProgramCode::setJump, compiler.firstFree + 1, lowerLimit, upperLimit);
				MIMA_LOG_INFO("Added jump to instruction {:02X} in range from {} to {}", compiler.firstFree + 1, lowerLimit, upperLimit);

				lowerLimit = 0;
				upperLimit = conditionMax;
				break;
			}

			MIMA_LOG_ERROR("Invalid jump instruction to '{}' in conditional compiling mode", token);
			break;

		default:
			MIMA_LOG_ERROR("Found unknown control symbol '{}'", control);
			break;
		}
	}


	void MicroProgramCompiler::ConditionalCompileMode::closeCompileMode() {
		//ensure the last line has been closed properly
		if (!compiler.lineStart) {
			compiler.memory[compiler.firstFree].reset();
			MIMA_LOG_ERROR("Unfinished microprogram code line at 0x{:02X} has been reset", compiler.firstFree);
		}


		//log closing of the default compiler mode
		MIMA_LOG_TRACE("Closed conditional compiler mode");
	}



	// ---------------------
	// Microprogram compiler
	// ---------------------

	MicroProgramCompiler::MicroProgramCompiler() : memory(new MicroProgramCodeList[0x100]) {
		currentCompileMode.reset(new DefaultCompileMode(*this));

		//0xFF is reserved for halt
		labels.insert({ "halt", HALT_RESERVED });
		memory[HALT_RESERVED].apply(&MicroProgramCode::setJump, HALT_RESERVED, 0, HALT_RESERVED);

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
		if (!lineStart) {
			MIMA_LOG_ERROR("Discarding invalid compiler directive '{}' which is not a line start", token);
			return;
		}
		std::string directive(token);
		
		//a compiler directive starting with "//" is a comment
		if (directive.rfind("//", 0) == 0) {
			MIMA_LOG_TRACE("Discarding comment '{}'", directive);
			return;
		}

		//if the compiler directive is not a comment, it is a function
		//use regex to match the function name and parameters
		std::smatch matches;

		//functions are structured like in C++:
		//function_name(param_0, param_1, ..., param_n)
		std::regex funcRegex(R"((?:(.*)\((.*)\)))");
		std::regex paramRegex(R"((.+?)(?:$|,)\s*)");

		if (std::regex_match(directive, matches, funcRegex)) {
			std::string func = matches[1];
			std::vector<std::string> arguments;

			std::string parameters = matches[2];
			while (std::regex_search(parameters, matches, paramRegex)) {
				arguments.push_back(matches[1]);
				parameters = matches.suffix();
			}

			//compile mode function
			if (func == "cm") {

				//switch to default compile mode
				if (arguments[0] == "default") {
					//no other arguments are expected
					if (arguments.size() != 1) {
						MIMA_LOG_ERROR("Expected one argument for compiler directive function {}, found {}", func, arguments.size());
						return;
					}

					//switch compile mode
					currentCompileMode->closeCompileMode();
					currentCompileMode.reset(new DefaultCompileMode(*this));

					MIMA_LOG_TRACE("Switched to a default compiler compile mode");
					return;
				}

				if (arguments[0] == "conditional") {
					//two mode arguments are expected:
					// 1 | name of condition
					// 2 | max value of condtion
					if (arguments.size() != 3) {
						MIMA_LOG_ERROR("Expected three arguments for compiler directive function {}, only found {}", func, arguments.size());
						return;
					}

					//check if the maximal condition value is valid
					size_t conditionMax;
					try {
						conditionMax = std::stoi(arguments[2]);
					}
					catch (const std::invalid_argument&) {
						MIMA_LOG_ERROR("Couldn't convert {} into a condition maximum", arguments[2]);
					}

					//switch compile mode
					currentCompileMode->closeCompileMode();
					currentCompileMode.reset(new ConditionalCompileMode(*this, arguments[1], conditionMax));

					MIMA_LOG_TRACE("Switched to a conditional decode compiler compile mode");
					return;
				}
			}

			MIMA_LOG_ERROR("Expected at least one argument for compiler directive function {}", func);
			return;
		}

		MIMA_LOG_ERROR("Discarding unknwon compiler directive '{}!'", directive);
	}


	MicroProgram MicroProgramCompiler::finish(char* remaining) {
		currentCompileMode->finish(remaining);

		//create microprogram
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
	MicroProgram MicroProgramCompiler::compileFile(const char*& fileName) {
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
