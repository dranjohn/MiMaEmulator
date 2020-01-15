#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "mima/microprogram/MicroProgram.h"
#include "mima/MinimalMachine.h"

#include "CLI/Command.h"

namespace MiMaCLI {
	typedef std::unordered_map<std::string, std::shared_ptr<MiMa::MicroProgram>> NamedMicroPrograms;
	typedef std::unordered_map<std::string, std::shared_ptr<MiMa::MinimalMachine>> NamedMinimalMachines;

	struct MiMaCLIState {
		static const std::regex identifierPattern;

		NamedMicroPrograms microprograms = NamedMicroPrograms();
		NamedMinimalMachines minimalMachines = NamedMinimalMachines();
	};


	typedef std::function<CommandResult(const std::string&, const std::shared_ptr<MiMaCLIState>&)> MiMaCLIStateModifier;

	class MiMaCLIStateCommand : public UniversalCommand {
	public:
		MiMaCLIStateCommand(const std::shared_ptr<MiMaCLIState>& state, const MiMaCLIStateModifier& commandExecution);
	};


	class MiMaCommandExecutor : public ConditionalCommand {
	public:
		MiMaCommandExecutor(const std::shared_ptr<MiMaCLIState>& state);
	};
}
