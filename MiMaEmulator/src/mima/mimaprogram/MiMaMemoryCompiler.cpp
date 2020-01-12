#include "MiMaMemoryCompiler.h"

#include <sstream>

namespace MiMa {
	const std::regex MiMaMemoryCompiler::assignmentMatcher(R"(\s*(.*?)\s*\=\s*(.*?)\s*)");
	const std::regex MiMaMemoryCompiler::instructionMatcher(R"(\s*(?:([^\s]+?)\:\s*)?([^\s]+?)(?:\s+([^\s]+?))?\s*)");

	const std::regex MiMaMemoryCompiler::identifiers(R"([_a-zA-Z][_a-zA-Z0-9]*)");
	const std::regex MiMaMemoryCompiler::decNumber(R"(\-?[0-9]+)");
	const std::regex MiMaMemoryCompiler::hexNumber(R"(\$\-?[a-fA-F0-9]+)");


#define SET_FUNCTION(givenName, name, storage, value) if (givenName == name) { storage = value; return true; }

	bool MiMaMemoryCompiler::addFunction(const std::string& functionName) {
		if (functionName == "DS")
			return true;

		SET_FUNCTION(functionName, "HALT", mimaMemory[compilationAddress], 0xF00000)
		SET_FUNCTION(functionName, "NOT", mimaMemory[compilationAddress], 0xF10000)
		SET_FUNCTION(functionName, "RAR", mimaMemory[compilationAddress], 0xF20000)

		return false;
	}

	bool MiMaMemoryCompiler::addUnaryFunction(const std::string& functionName, const uint32_t& argument) {
		SET_FUNCTION(functionName, "DS", mimaMemory[compilationAddress], argument)
		SET_FUNCTION(functionName, "LDC", mimaMemory[compilationAddress], argument)
		SET_FUNCTION(functionName, "LDV", mimaMemory[compilationAddress], 0x100000 | argument)
		SET_FUNCTION(functionName, "STV", mimaMemory[compilationAddress], 0x200000 | argument)
		SET_FUNCTION(functionName, "ADD", mimaMemory[compilationAddress], 0x300000 | argument)
		SET_FUNCTION(functionName, "AND", mimaMemory[compilationAddress], 0x400000 | argument)
		SET_FUNCTION(functionName, "OR", mimaMemory[compilationAddress], 0x500000 | argument)
		SET_FUNCTION(functionName, "XOR", mimaMemory[compilationAddress], 0x600000 | argument)
		SET_FUNCTION(functionName, "EQL", mimaMemory[compilationAddress], 0x700000 | argument)
		SET_FUNCTION(functionName, "JMP", mimaMemory[compilationAddress], 0x800000 | argument)
		SET_FUNCTION(functionName, "JMN", mimaMemory[compilationAddress], 0x900000 | argument)

		return false;
	}


	void MiMaMemoryCompiler::addLine(const std::string& line) {
		//remove any comments from the code line
		std::smatch matches;
		std::string codeLine = line.substr(0, line.find(';'));
		MIMA_LOG_TRACE("Found code line '{}'", codeLine);

		//attempt to match the line as an assignment
		if (std::regex_match(codeLine, matches, assignmentMatcher)) {
			MIMA_LOG_TRACE("Matched code line as assignment");
			int compilationStart;

			if (std::regex_match(matches[2].str(), decNumber)) {
				//dec number from matches[2]
				compilationStart = std::stoi(matches[2], nullptr, 10);
				MIMA_LOG_TRACE("Interpreted {} as a decimal number", matches[2].str());
			}
			else if (std::regex_match(matches[2].str(), hexNumber)) {
				//hex number from matches[2].substr(1)
				compilationStart = std::stoi(matches[2].str().substr(1), nullptr, 16);
				MIMA_LOG_TRACE("Interpreted {} as a hexadecimal number", matches[2].str());
			}
			else {
				//invalid compilation start from matches[2]
				MIMA_LOG_ERROR("Failed to convert value {} to a decimal or hexadecimal number", matches[2].str());
				return;
			}

			if (matches[1] == "*") {
				//range check compilation start
				if (compilationStart < 0) {
					//compilation start may not be negative
					MIMA_LOG_ERROR("The compilation start point may not be assigned the negative number 0x{:X}", compilationStart);
					return;
				}
				if (compilationStart >= MEMORY_CAPACITY) {
					//compilation start may not exceed memory capacity
					MIMA_LOG_ERROR("The compilation start point 0x{:X} may not exceed the memory capacity 0x{:X}", compilationStart, MEMORY_CAPACITY);
					return;
				}

				//set compilation start
				compilationAddress = compilationStart;
				MIMA_LOG_TRACE("Set compilation start to 0x{:X}", compilationAddress);
				return;
			}
			else if (std::regex_match(matches[1].str(), identifiers)) {
				//constant definition
				//identifier reservation
				return;
			}
			
			//unknown assignment to matches[1]
			MIMA_LOG_ERROR("'{}' is not a valid identifier to have a value assigned too", matches[1].str());
			return;
		}

		//attempt to match the line as a unary instruction
		if (std::regex_match(codeLine, matches, instructionMatcher)) {
			if (matches[1] != "") {
				//add label from matches[1]
			}

			if (matches[3] != "") {
				//instruction has an argument, add unary instruction from matches[2] with argument matches[3]
				uint32_t argument;

				if (std::regex_match(matches[3].str(), decNumber)) {
					//dec number from matches[3]
					argument = std::stoi(matches[3], nullptr, 10);
					MIMA_LOG_TRACE("Interpreted {} as a decimal number", matches[3].str());
				}
				else if (std::regex_match(matches[3].str(), hexNumber)) {
					//hex number from matches[3].substr(1)
					argument = std::stoi(matches[3].str().substr(1), nullptr, 16);
					MIMA_LOG_TRACE("Interpreted {} as a hexadecimal number", matches[3].str());
				}
				else {
					//invalid number from matches[2]
					MIMA_LOG_ERROR("Failed to convert value {} to a decimal or hexadecimal number", matches[3].str());
					return;
				}

				if (addUnaryFunction(matches[2], argument)) {
					compilationAddress++;
				}
				else {
					//invalid function found
					MIMA_LOG_ERROR("Unknown parameterizerd instruction '{}'", matches[2].str());
				}
			}
			else {
				//instruction has no argument, add function from matches[2]
				if (addFunction(matches[2])) {
					compilationAddress++;
				}
				else {
					//invalid function found
					MIMA_LOG_ERROR("Unknown instruction '{}'", matches[2].str());
				}
			}

			return;
		}

		//no valid match
		MIMA_LOG_ERROR("Failed to compiler '{}'", codeLine);
	}


	void MiMaMemoryCompiler::finish() {

	}


	MemoryCell* MiMaMemoryCompiler::compile(char* mimaProgramCode) {
		MiMaMemoryCompiler compiler;

		std::istringstream mimaProgramCodeStream(mimaProgramCode);
		std::string codeLine;
		while (std::getline(mimaProgramCodeStream, codeLine)) {
			compiler.addLine(codeLine);
		}

		return compiler.mimaMemory;
	}
}
