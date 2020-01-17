#include "MiMaCommandExecutor.h"

#include <algorithm>
#include <regex>
#include <vector>

#include <fmt/format.h>

#include "mima/microprogram/MicroProgramCompiler.h"
#include "mima/mimaprogram/MiMaCompiler.h"
#include "mima/CompilerException.h"

#include "CLI/CommandUtility.h"

namespace MiMaCLI {
	const std::regex MiMaCLIState::identifierPattern(R"([_a-zA-Z][_a-zA-Z0-9]*)");

	MiMaCLIStateCommand::MiMaCLIStateCommand(const std::shared_ptr<MiMaCLIState>& state, const MiMaCLIStateModifier& commandExecution) :
		UniversalCommand([state, commandExecution](const std::string& input)->CommandResult { return commandExecution(input, state); })
	{}



	// ---------------------------
	// Command utility definitions
	// ---------------------------

	// --- Constants definitions ---

	static const std::regex emptyLinePattern(R"(\s*)");
	

	// -----------------------------
	// Command functions definitions
	// -----------------------------

	// --- Exit function ---

	static const CommandExecution exitFunction = [](const std::string& input)->CommandResult {
		CommandUtility::assertNoArguments(input);

		return { true, "Exiting MiMa CLI" };
	};


	// --- Microprogram command functions ---

	static const MiMaCLIStateModifier microprogramCompileFunction = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = CommandUtility::getArguments(input, 2);

		CommandUtility::validateIdentifier(arguments[0], MiMaCLIState::identifierPattern);

		try {
			std::shared_ptr<const MiMa::MicroProgram> microprogram = MiMa::MicroProgramCompiler::compileFile(arguments[1]);
			(state->microprograms).insert({ arguments[0], microprogram });
		}
		catch (const MiMa::CompilerException& exc) {
			throw CommandException(exc);
		}

		return { false, fmt::format("Compiled microprogram '{}'", arguments[1]) };
	};

	static const size_t maxUpperLimit = 0xFF;
	static const MiMaCLIStateModifier microprogramShowFunction = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = CommandUtility::getArguments(input, 3);

		CommandUtility::validateIdentifier(arguments[0], MiMaCLIState::identifierPattern);
		size_t lowerLimit = std::min(CommandUtility::validatePositiveDecimalInteger(arguments[1]), maxUpperLimit);
		size_t upperLimit = std::min(CommandUtility::validatePositiveDecimalInteger(arguments[2]), maxUpperLimit);

		NamedMicroPrograms::const_iterator foundMicroprogram = (state->microprograms).find(arguments[0]);

		if (foundMicroprogram == (state->microprograms).end()) {
			throw CommandException(fmt::format("No microprogram under the name '{}' exists", arguments[0]));
		}

		std::string microprogramOutputFormat = fmt::format("Microprogram '{{}}':\n{}:{},{}{}", '{', lowerLimit, upperLimit, '}');
		return { false, fmt::format(microprogramOutputFormat, foundMicroprogram->first, *(foundMicroprogram->second)) };
	};


	// --- Minimal machine command functions ---

	static const MiMaCLIStateModifier minimalMachineCompile = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = CommandUtility::getArguments(input, 3);

		CommandUtility::validateIdentifier(arguments[0], MiMaCLIState::identifierPattern);
		CommandUtility::validateIdentifier(arguments[2], MiMaCLIState::identifierPattern);

		NamedMicroPrograms::const_iterator foundMicroprogram = (state->microprograms).find(arguments[2]);
		if (foundMicroprogram == (state->microprograms).end()) {
			throw CommandException(fmt::format("No microprogram under the name '{}' exists", arguments[2]));
		}

		try {
			(state->minimalMachines).insert({ arguments[0], std::make_shared<MiMa::MinimalMachine>(foundMicroprogram->second, MiMa::MiMaMemoryCompiler::compileFile(arguments[1])) });
		}
		catch (const MiMa::CompilerException& exc) {
			throw CommandException(exc);
		}

		return { false, fmt::format("Created minimal machine '{}' with the microprogram '{}'", arguments[0], foundMicroprogram->first) };
	};

	static const MiMaCLIStateModifier minimalMachineShow = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = CommandUtility::getArguments(input, 1);

		CommandUtility::validateIdentifier(arguments[0], MiMaCLIState::identifierPattern);

		NamedMinimalMachines::const_iterator foundMinimalMachine = (state->minimalMachines).find(arguments[0]);

		if (foundMinimalMachine == (state->minimalMachines).end()) {
			return { false, fmt::format("No minimal machine under the name '{}' exists", arguments[0]) };
		}

		return { false, fmt::format("Minimal machine '{}':\n{}", foundMinimalMachine->first, *(foundMinimalMachine->second)) };
	};

	static const MiMaCLIStateModifier minimalMachineEmulate = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = CommandUtility::getArguments(input, 2);

		CommandUtility::validateIdentifier(arguments[0], MiMaCLIState::identifierPattern);

		NamedMinimalMachines::iterator foundMinimalMachine = (state->minimalMachines).find(arguments[0]);

		if (foundMinimalMachine == (state->minimalMachines).end()) {
			throw CommandException(fmt::format("No minimal machine under the name '{}' exists", arguments[0]));
		}


		if (arguments[1] == "cycle") {
			(foundMinimalMachine->second)->emulateClockCycle();
			return { false, fmt::format("Emulated clock cycle for minimal machine '{}'", foundMinimalMachine->first) };
		}

		if (arguments[1] == "instruction") {
			(foundMinimalMachine->second)->emulateInstructionCycle();
			return { false, fmt::format("Emulated instruction cycle for minimal machine '{}'", foundMinimalMachine->first) };
		}

		if (arguments[1] == "lifetime") {
			(foundMinimalMachine->second)->emulateLifeTime();
			return { false, fmt::format("Emulated lifetime for minimal machine '{}'", foundMinimalMachine->first) };
		}


		throw CommandException(fmt::format("Unknown emulation step size '{}'", arguments[1]));
	};



	MiMaCommandExecutor::MiMaCommandExecutor(const std::shared_ptr<MiMaCLIState>& state) :
		ConditionalCommand({
			{ "exit", new UniversalCommand(exitFunction) },
			{ "microprogram", new ConditionalCommand({
				{ "compile", new MiMaCLIStateCommand(state, microprogramCompileFunction) },
				{ "show", new MiMaCLIStateCommand(state, microprogramShowFunction) }
			}) },
			{ "mima", new ConditionalCommand({
				{ "compile", new MiMaCLIStateCommand(state, minimalMachineCompile) },
				{ "show", new MiMaCLIStateCommand(state, minimalMachineShow) },
				{ "emulate", new MiMaCLIStateCommand(state, minimalMachineEmulate) }
			}) }
		})
	{}
}
