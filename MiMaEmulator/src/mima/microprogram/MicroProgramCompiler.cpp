#include "MicroProgramCompiler.h"

#include <sstream>

#include "StatusBit.h"
#include "mima/CompilerException.h"

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

	// --- Constants definitions

	const std::regex MicroProgramCompiler::CompileMode::emptyLinePattern("\s*");


	// --- Functions ---

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


	void MicroProgramCompiler::CompileMode::finish() {
		//close compiler mode
		closeCompileMode();
	}



	// ----------------------------------------------------------
	// Default (global) compile mode of the microprogram compiler
	// ----------------------------------------------------------

	// --- Constants definitions ---

	const std::regex MicroProgramCompiler::DefaultCompileMode::labelMatcher(R"(\s*(.*?)\:(.*))");
	const std::regex MicroProgramCompiler::DefaultCompileMode::instructionMatcher(R"((.*?);)");

	const std::regex MicroProgramCompiler::DefaultCompileMode::registerTransferMatcher(R"(\s*(.*?)\s*\->\s*(.*?)\s*)");
	const std::regex MicroProgramCompiler::DefaultCompileMode::assignmentMatcher(R"(\s*(.*?)\s*\=\s*(.*?)\s*)");
	const std::regex MicroProgramCompiler::DefaultCompileMode::jumpMatcher(R"(\s*#(.*?)\s*)");


	// --- Functions ---
	
	MicroProgramCompiler::DefaultCompileMode::DefaultCompileMode(MicroProgramCompiler& compiler) :
		CompileMode(compiler),
		operatorBuffer(std::string(""), [](const std::string&, const std::string&) { return NO_MICROPROGRAM_CODE_MODIFICATION; })
	{
		//reset code memory where the program is written too
		compiler.memory[compiler.firstFree].reset();

		MIMA_LOG_TRACE("Opened default compiler mode");
	}

	bool MicroProgramCompiler::DefaultCompileMode::addLine(const std::string& line) {
		std::string codeLine = line;
		std::smatch matches;

		if (std::regex_match(codeLine, matches, labelMatcher)) {
			CompileMode::addLabel(matches[1]);
			codeLine = matches[2];
		}

		std::sregex_iterator instructionsStart(codeLine.begin(), codeLine.end(), instructionMatcher);
		std::sregex_iterator instructionsEnd;

		for (std::sregex_iterator i = instructionsStart; i != instructionsEnd; ++i) {
			std::string instruction = (*i)[1];

			if (std::regex_match(instruction, emptyLinePattern)) {
				continue;
			}

			if (std::regex_match(instruction, matches, registerTransferMatcher)) {
				//get the read and write functions for the microprogram code modification
				MicroProgramCodeSetFunction setWrite = getWriteBit(matches[1]);
				MicroProgramCodeSetFunction setRead = getReadBit(matches[2]);

				//create a modifier executing both these functions
				MicroProgramCodeModifier registerTransfer = [setWrite, setRead](MicroProgramCode& microProgramCode) {
					(microProgramCode.*setWrite)();
					(microProgramCode.*setRead)();
				};

				compiler.memory[compiler.firstFree].apply(registerTransfer);

				continue;
			}

			if (std::regex_match(instruction, matches, assignmentMatcher)) {
				//the left operand "R" marks that storage read is being (un)set
				if (matches[1] == "R") {
					if (matches[2] == "1") {
						compiler.memory[compiler.firstFree].apply(&MicroProgramCode::enableMemoryRead);
					}
					if (matches[2] == "0") {
						compiler.memory[compiler.firstFree].apply(&MicroProgramCode::disableMemoryRead);
					}
				}

				//the left operand "R" marks that storage write is being (un)set
				if (matches[1] == "W") {
					if (matches[2] == "1") {
						compiler.memory[compiler.firstFree].apply(&MicroProgramCode::enableMemoryWrite);
					}
					if (matches[2] == "0") {
						compiler.memory[compiler.firstFree].apply(&MicroProgramCode::disableMemoryWrite);
					}
				}

				//the left operand "ALU" marks that an ALU code (in [0, 7]) is being set
				if (matches[1] == "ALU") {
					compiler.memory[compiler.firstFree].apply(&MicroProgramCode::setALUCode, getALUCode(matches[2]));
				}

				continue;
			}

			if (std::regex_match(instruction, matches, jumpMatcher)) {
				CompileMode::addJump(matches[1], fixedJump, compiler.memory[compiler.firstFree]);
				continue;
			}

			//unknown instruction
			MIMA_LOG_ERROR("Found unknown instruction '{}'", instruction);
			return false;
		}

		CompileMode::endOfLine(fixedJump, compiler.memory[compiler.firstFree]);
		return true;
	}


	void MicroProgramCompiler::DefaultCompileMode::closeCompileMode() {
		//ensure there is nothing remaining in the binary operator buffer
		MIMA_ASSERT_ERROR(!operatorBuffer.isBufferOccupied(), "Binary operator remaining after finishing compilation with remaining operand");


		//log closing of the default compiler mode
		MIMA_LOG_TRACE("Closed default compiler mode");
	}



	// ----------------------------------------------------------
	// Conditional jump compile mode of the microprogram compiler
	// ----------------------------------------------------------

	// --- Constants definitions ---

	const std::regex MicroProgramCompiler::ConditionalCompileMode::conditionalJumpMatcher(R"(\s*\[\s*(.*?)\s*\,\s*(.*?)\s*\]\s*#(.*?)\s*(;?)\s*)");


	// --- Functions ---

	MicroProgramCompiler::ConditionalCompileMode::ConditionalCompileMode(MicroProgramCompiler& compiler, const std::string& conditionName, const size_t& conditionMax) :
		CompileMode(compiler),
		conditionName(conditionName),
		conditionMax(conditionMax)
	{
		compiler.memory[compiler.firstFree].reset(conditionName, conditionMax);
		MIMA_LOG_TRACE("Opened conditional jump compiler mode for condition '{}' up to 0x{:X}", conditionName, conditionMax);
	}


	bool MicroProgramCompiler::ConditionalCompileMode::addLine(const std::string& line) {
		std::smatch matches;

		if (!std::regex_match(line, matches, conditionalJumpMatcher)) {
			return false;
		}

		const size_t lowerLimit = std::stoi(matches[1]);
		const size_t upperLimit = (matches[2] == "max") ? conditionMax : std::stoi(matches[2]);

		CompileMode::addJump(matches[3], compiler.memory[compiler.firstFree], lowerLimit, upperLimit);

		if (matches[4] == ";") {
			CompileMode::endOfLine(compiler.memory[compiler.firstFree]);
		}
		return true;
	}


	void MicroProgramCompiler::ConditionalCompileMode::closeCompileMode() {
		//log closing of the conditional compiler mode
		MIMA_LOG_TRACE("Closed conditional compiler mode");
	}



	// ---------------------
	// Microprogram compiler
	// ---------------------

	// --- Constants definitions ---

	const std::regex MicroProgramCompiler::compilerDirectivePattern(R"(\s*\!(.*))");
	const std::regex MicroProgramCompiler::compilerDirectiveFunctionMatcher(R"((?:(.*)\((.*)\)))");
	const std::regex MicroProgramCompiler::compilerDirectiveParameterMatcher(R"((.+?)(?:$|,)\s*)");


	// --- Functions ---

	MicroProgramCompiler::MicroProgramCompiler() : memory(new MicroProgramCodeList[0x100]) {
		currentCompileMode.reset(new DefaultCompileMode(*this));

		//0xFF is reserved for halt
		labels.insert({ "halt", HALT_RESERVED });
		memory[HALT_RESERVED].apply(&MicroProgramCode::setJump, HALT_RESERVED, 0, HALT_RESERVED);

		MIMA_LOG_INFO("Initialized microcode compiler");
	};


	void MicroProgramCompiler::addDirective(const std::string& directive) {
		std::smatch matches;

		if (std::regex_match(directive, matches, compilerDirectiveFunctionMatcher)) {
			//extract function name
			std::string func = matches[1];

			//extract arguments
			std::vector<std::string> arguments;

			std::string argumentList = matches[2];
			while (std::regex_search(argumentList, matches, compilerDirectiveParameterMatcher)) {
				arguments.push_back(matches[1]);
				argumentList = matches.suffix();
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

	void MicroProgramCompiler::addLine(const std::string& line) {
		//remove comment at the start of the line
		std::string codeLine = line.substr(0, line.find("//"));

		//don't add empty lines
		if (std::regex_match(codeLine, CompileMode::emptyLinePattern)) {
			return;
		}

		//if the line starts with a compiler directive symbol, treat it as such
		std::smatch matches;
		if (std::regex_match(codeLine, matches, MicroProgramCompiler::compilerDirectivePattern)) {
			addDirective(matches[1]);
			return;
		}
		
		//otherwise, pass it on to the current compile mode
		currentCompileMode->addLine(line);
	}


	std::shared_ptr<const MicroProgram> MicroProgramCompiler::finish() {
		currentCompileMode->finish();

		for (auto unresolvedListener = labelAddListeners.begin(); unresolvedListener != labelAddListeners.end(); ++unresolvedListener) {
			MIMA_LOG_WARN("Found label listener for {} after finishing compilation", unresolvedListener->first);
		}

		//create microprogram
		MIMA_LOG_INFO("Finished microprogram compilation at 0x{:02X}", firstFree);

		std::shared_ptr<const MicroProgram> program = std::make_shared<MicroProgram>(memory);
		return program;
	}



	// ---------------------
	// Compilation interface
	// ---------------------

	//Interface: read input from a pointer to a char array containing the code for the program.
	std::shared_ptr<const MicroProgram> MicroProgramCompiler::compile(const std::string& microProgramCode) {
		MIMA_LOG_INFO("Compiling microprogram from given code string");

		MicroProgramCompiler compiler;

		std::istringstream microProgramCodeStream(microProgramCode);
		std::string codeLine;
		while (std::getline(microProgramCodeStream, codeLine)) {
			compiler.addLine(codeLine);
		}

		return compiler.finish();
	}

	//Interface: read input from an input providing the code for the program.
	std::shared_ptr<const MicroProgram> MicroProgramCompiler::compile(std::istream& microProgramCode) {
		MIMA_LOG_INFO("Compiling microprogram from given input");

		MicroProgramCompiler compiler;

		std::string codeLine;
		while (std::getline(microProgramCode, codeLine)) {
			compiler.addLine(codeLine);
		}

		return compiler.finish();
	}


	//Interface: read input from a file containing the code for the program.
	std::shared_ptr<const MicroProgram> MicroProgramCompiler::compileFile(const std::string& fileName) {
		MIMA_LOG_INFO("Compiling microprogram from an input file");

		std::ifstream fileInputStream(fileName);
		if (!fileInputStream.good()) {
			throw CompilerException(fmt::format("Failed to open microprogram code file '{}'", fileName));
		}

		std::shared_ptr<const MicroProgram> program = compile(fileInputStream);

		fileInputStream.close();
		return program;
	}
}
