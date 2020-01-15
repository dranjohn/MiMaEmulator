#include "MiMaCommandExecutor.h"

#include <algorithm>
#include <regex>
#include <vector>

#include <fmt/format.h>

#include "mima/microprogram/MicroProgramCompiler.h"
#include "mima/mimaprogram/MiMaMemoryCompiler.h"
#include "mima/CompilerException.h"

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


	// --- Utility function definitions ---

	std::vector<std::string> getArguments(const std::string& input) {
		std::sregex_iterator argumentsBegin = std::sregex_iterator(input.begin(), input.end(), Command::wordPattern);
		std::sregex_iterator argumentsEnd = std::sregex_iterator();

		std::vector<std::string> arguments;
		while (argumentsBegin != argumentsEnd) {
			arguments.push_back((*argumentsBegin)[0]);
			++argumentsBegin;
		}

		return arguments;
	}
	

	// -----------------------------
	// Command functions definitions
	// -----------------------------

	// --- Exit function ---

	static const CommandExecution exitFunction = [](const std::string& input)->CommandResult {
		if (!std::regex_match(input, emptyLinePattern)) {
			return { false, "The exit command doesn't take any arguments" };
		}

		return { true, "Exiting MiMa CLI" };
	};


	// --- Microprogram command functions ---

	static const MiMaCLIStateModifier microprogramCompileFunction = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = getArguments(input);

		if (arguments.size() != 2) {
			return { false, "Two arguments are required for 'microprogram compile'" };
		}
		if (!std::regex_match(arguments[0], MiMaCLIState::identifierPattern)) {
			return { false, fmt::format("'{}' is not a valid identifier", arguments[0]) };
		}

		try {
			MiMa::MicroProgram microprogram = MiMa::MicroProgramCompiler::compileFile(arguments[1]);
			(state->microprograms).insert({ arguments[0], std::make_shared<MiMa::MicroProgram>(microprogram) });
		}
		catch (const MiMa::CompilerException & exc) {
			throw CommandException(exc);
		}

		return { false, fmt::format("Compiled microprogram '{}'", arguments[1]) };
	};

	static const std::regex positiveDecPattern(R"([0-9]+)");
	static const size_t maxUpperLimit = 0xFF;
	static const MiMaCLIStateModifier microprogramShowFunction = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = getArguments(input);

		if (arguments.size() != 3) {
			return { false, "Three arguments are required for 'microprogram show'" };
		}
		if (!std::regex_match(arguments[0], MiMaCLIState::identifierPattern)) {
			return { false, fmt::format("'{}' is not a valid identifier", arguments[0]) };
		}
		if (!(std::regex_match(arguments[1], positiveDecPattern) && std::regex_match(arguments[2], positiveDecPattern))) {
			return { false, "Argument 2 and 3 both need to be positive decimal numbers" };
		}

		NamedMicroPrograms::const_iterator foundMicroprogram = (state->microprograms).find(arguments[0]);

		if (foundMicroprogram == (state->microprograms).end()) {
			return { false, fmt::format("No microprogram under the name '{}' exists", arguments[0]) };
		}

		size_t lowerLimit = std::min((size_t)std::stoi(arguments[1]), maxUpperLimit);
		size_t upperLimit = std::min((size_t)std::stoi(arguments[2]), maxUpperLimit);

		std::string x = fmt::format("Microprogram '{}':\n{}:{},{}{}", foundMicroprogram->first, '{', lowerLimit, upperLimit, '}');
		return { false, fmt::format(x, *(foundMicroprogram->second)) };
	};


	// --- Minimal machine command functions ---

	static const MiMaCLIStateModifier minimalMachineCompile = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = getArguments(input);

		if (arguments.size() != 3) {
			return { false, "Two arguments are required for 'mima compile'" };
		}
		if (!std::regex_match(arguments[0], MiMaCLIState::identifierPattern)) {
			return { false, fmt::format("'{}' is not a valid identifier", arguments[0]) };
		}
		if (!std::regex_match(arguments[2], MiMaCLIState::identifierPattern)) {
			return { false, fmt::format("'{}' is not a valid identifier", arguments[2]) };
		}

		NamedMicroPrograms::const_iterator foundMicroprogram = (state->microprograms).find(arguments[2]);
		if (foundMicroprogram == (state->microprograms).end()) {
			return { false, fmt::format("No microprogram under the name '{}' exists", arguments[0]) };
		}

		(state->minimalMachines).insert({ arguments[0], std::make_shared<MiMa::MinimalMachine>(*(foundMicroprogram->second), MiMa::MiMaMemoryCompiler::compileFile(arguments[1])) });

		return { false, fmt::format("Created minimal machine '{}' with the microprogram '{}'", arguments[0], foundMicroprogram->first) };
	};

	static const MiMaCLIStateModifier minimalMachineShow = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = getArguments(input);

		if (arguments.size() != 1) {
			return { false, "One argument are required for 'mima show'" };
		}
		if (!std::regex_match(arguments[0], MiMaCLIState::identifierPattern)) {
			return { false, fmt::format("'{}' is not a valid identifier", arguments[0]) };
		}

		NamedMinimalMachines::const_iterator foundMinimalMachine = (state->minimalMachines).find(arguments[0]);

		if (foundMinimalMachine == (state->minimalMachines).end()) {
			return { false, fmt::format("No minimal machine under the name '{}' exists", arguments[0]) };
		}

		return { false, fmt::format("Minimal machine '{}':\n{}", foundMinimalMachine->first, *(foundMinimalMachine->second)) };
	};

	static const MiMaCLIStateModifier minimalMachineEmulate = [](const std::string& input, const std::shared_ptr<MiMaCLIState>& state)->CommandResult {
		std::vector<std::string> arguments = getArguments(input);

		if (arguments.size() != 2) {
			return { false, "Two arguments are required for 'mima emulate'" };
		}

		NamedMinimalMachines::iterator foundMinimalMachine = (state->minimalMachines).find(arguments[0]);

		if (foundMinimalMachine == (state->minimalMachines).end()) {
			return { false, fmt::format("No minimal machine under the name '{}' exists", arguments[0]) };
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


		return { false, fmt::format("Unknown emulation step size '{}'", arguments[1]) };
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
