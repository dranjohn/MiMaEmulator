#pragma once

#include <cstdint>
#include <cctype>
#include <memory>
#include <string>
#include <map>
#include <istream>
#include <fstream>
#include <functional>
#include <regex>

#include "MicroProgram.h"

#include "debug/Log.h"
#include "util/BinaryOperatorBuffer.h"

namespace MiMa {
	//Utility: binary operators understandable by the micro program compiler
	typedef std::function<void(MicroProgramCode&)> MicroProgramCodeModifier;
	typedef MicroProgramCodeModifier(*BinaryOperator)(const std::string&, const std::string&);

	class MicroProgramCompiler {
	private:
		class CompileMode {
		public:
			static const std::regex emptyLinePattern;

		protected:
			MicroProgramCompiler& compiler;
			CompileMode(MicroProgramCompiler& compiler) : compiler(compiler) {}

			//adds a new label and resolves unresolved jumps to the given label
			void addLabel(std::string label);
			//adds a jump instruction to the current microcode
			void addJump(std::string label, bool& fixedJump, MicroProgramCodeList& currentCode, bool overrideFixed = false);
			void addJump(std::string label, MicroProgramCodeList& currentCode, const size_t& lowerLimit, const size_t& upperLimit);

			//ends the current line
			void endOfLine(MicroProgramCodeList& currentCode);
			void endOfLine(bool& fixedJump, MicroProgramCodeList& currentCode);
		public:
			virtual bool addLine(const std::string& line) = 0;

			virtual void closeCompileMode() = 0;
			virtual void finish();
		};


	private:
		class DefaultCompileMode : public CompileMode {
		private:
			static const std::regex labelMatcher;
			static const std::regex instructionMatcher;

			static const std::regex registerTransferMatcher;
			static const std::regex assignmentMatcher;
			static const std::regex jumpMatcher;

		private:
			//current line of code encoding
			bool fixedJump = false;

			//binary operator buffers
			BinaryOperatorBuffer<std::string, MicroProgramCodeModifier> operatorBuffer;
		public:
			DefaultCompileMode(MicroProgramCompiler& compiler);

			bool addLine(const std::string& line) override;

			void closeCompileMode() override;
		};

		class ConditionalCompileMode : public CompileMode {
		private:
			static const std::regex conditionalJumpMatcher;

		private:
			const std::string conditionName;
			const size_t conditionMax;
		public:
			ConditionalCompileMode(MicroProgramCompiler& compiler, const std::string& conditionName, const size_t& conditionMax);

			bool addLine(const std::string& line) override;

			void closeCompileMode() override;
		};


	private:
		static const std::regex compilerDirectivePattern;
		static const std::regex compilerDirectiveFunctionMatcher;
		static const std::regex compilerDirectiveParameterMatcher;

	private:
		//current compile mode
		std::unique_ptr<CompileMode> currentCompileMode;

		//a pointer to the memory to manipulate
		std::shared_ptr<MicroProgramCodeList[]> memory;
		uint8_t firstFree = 0;

		//label tracking
		std::unordered_map<std::string, uint8_t> labels;
		std::multimap<std::string, std::function<bool(const uint8_t&)>> labelAddListeners;


		void addDirective(const std::string& directive);

	public:
		MicroProgramCompiler();

		void addLine(const std::string& line);

		std::shared_ptr<const MicroProgram> finish();


		// ------------------------------------------------------
		// Compilation utility methods
		// Use these for simple compilation of common input types
		// ------------------------------------------------------
		static std::shared_ptr<const MicroProgram> compile(const std::string& microProgramCode);
		static std::shared_ptr<const MicroProgram> compile(std::istream& microProgramCode);
		
		static std::shared_ptr<const MicroProgram> compileFile(const std::string& fileName);
	};
}
