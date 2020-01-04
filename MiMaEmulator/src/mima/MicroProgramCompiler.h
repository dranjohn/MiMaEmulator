#pragma once

#include <stdint.h>
#include <cctype>
#include <memory>
#include <string>
#include <map>
#include <istream>
#include <fstream>
#include <functional>

#include "MicroProgram.h"

#include "debug/Log.h"
#include "util/BinaryOperatorBuffer.h"
#include "util/CharStream.h"

namespace MiMa {
	//Utility: binary operators understandable by the micro program compiler
	typedef std::function<void(MicroProgramCode&)> MicroProgramCodeModifier;
	typedef MicroProgramCodeModifier(*BinaryOperator)(const std::string&, const std::string&);

	class MicroProgramCompiler {
	private:
		class CompileMode {
		protected:
			MicroProgramCompiler& compiler;
			CompileMode(MicroProgramCompiler& compiler) : compiler(compiler) {}

			//adds a new label and resolves unresolved jumps to the given label
			void addLabel(std::string label);
			//adds a jump instruction to the current microcode
			void addJump(std::string label, bool& fixedJump, MicroProgramCode& currentCode, bool overrideFixed = false);

			//ends the current line
			void endOfLine(bool& fixedJump, MicroProgramCode& currentCode);
		public:
			virtual bool isControl(const char& control) = 0;
			virtual void addToken(const char& control, char* token) = 0;

			virtual void closeCompileMode() = 0;
			virtual void finish(char* remaining) = 0;
		};


	private:
		class DefaultCompileMode : public CompileMode {
		private:
			//current line of code encoding
			bool fixedJump = false;

			//binary operator buffers
			BinaryOperatorBuffer<std::string, MicroProgramCodeModifier> operatorBuffer;
		public:
			DefaultCompileMode(MicroProgramCompiler& compiler);

			bool isControl(const char& control) override;
			void addToken(const char& control, char* token) override;

			void closeCompileMode() override;
			void finish(char* remaining) override;
		};


	private:
		//current compile mode
		std::unique_ptr<CompileMode> currentCompileMode;

		//a pointer to the memory to manipulate
		std::shared_ptr<MicroProgramCode[]> memory;
		uint8_t firstFree = 0;

		//label tracking
		std::unordered_map<std::string, uint8_t> labels;
		std::multimap<std::string, std::function<bool(const uint8_t&)>> labelAddListeners;

		//line tracking
		bool lineStart = true;
	public:
		MicroProgramCompiler();

		bool isControl(const char& control);
		void addToken(const char& control, char* token);

		MicroProgram finish(char* remaining);


		// ------------------------------------------------------
		// Compilation utility methods
		// Use these for simple compilation of common input types
		// ------------------------------------------------------
		static MicroProgram compile(char* microProgramCode);
		static MicroProgram compile(std::istream& microProgramCode);
		 
		static MicroProgram compileFile(char* fileName);

	private:
		static MicroProgram compile(CharStream& microProgramCodeStream);
	};
}
