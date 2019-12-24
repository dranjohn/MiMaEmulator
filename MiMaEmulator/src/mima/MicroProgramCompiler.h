#pragma once

#include <stdint.h>
#include <cctype>
#include <memory>
#include <string>
#include <map>
#include <stack>
#include <istream>

#include "debug/Log.h"
#include "util/BinaryOperatorBuffer.h"

namespace MiMa {
	//Utility: binary operators understandable by the micro program compiler
	typedef uint32_t(*BinaryOperator)(const std::string&, const std::string&);

	class MicroProgramCompiler {
	private:
		class Scope {
		protected:
			MicroProgramCompiler& compiler;
			Scope(MicroProgramCompiler& compiler) : compiler(compiler) {}

			//adds a new label and resolves unresolved jumps to the given label
			void addLabel(std::string label);
			//adds a jump instruction to the current microcode
			void addJump(std::string label, bool& fixedJump, uint32_t& currentCode);

			//ends the current line
			void endOfLine(bool& fixedJump, uint32_t& currentCode);
		public:
			virtual bool isControl(const char& control) = 0;
			virtual void addToken(const char& control, char* token) = 0;

			virtual void finish(char* remaining) = 0;

			virtual bool canOpenScope(char* scopeName) = 0;
			virtual Scope* openScope(char* scopeName) = 0;
			virtual void cleanUpScope() = 0;
		};


	private:
		//scope stack
		std::stack<std::unique_ptr<Scope>> scopeStack;

		//a pointer to the memory to manipulate
		std::shared_ptr<uint32_t[]> memory;
		uint8_t& firstFree;

		//label tracking
		std::map<std::string, uint8_t>& labels;
		std::multimap<std::string, uint8_t>& unresolvedLabels;


	private:
		class GlobalScope : public Scope {
		private:
			//current line of code encoding
			bool fixedJump = false;
			uint32_t currentCode = 0;

			//binary operator buffers
			BinaryOperatorBuffer<std::string, uint32_t> operatorBuffer;
		public:
			GlobalScope(MicroProgramCompiler& compiler);

			bool isControl(const char& control) override;
			void addToken(const char& control, char* token) override;

			void finish(char* remaining) override;

			bool canOpenScope(char* scopeName) override;
			Scope* openScope(char* scopeName) override;
			void cleanUpScope() override;
		};


	public:
		MicroProgramCompiler(std::shared_ptr<uint32_t[]>& memory, uint8_t& startingPoint, std::map<std::string, uint8_t>& labels, std::multimap<std::string, uint8_t>& unresolvedLabels);

		bool isControl(const char& control);
		void addToken(const char& control, char* token);

		void finish(char* remaining);
	};

}
