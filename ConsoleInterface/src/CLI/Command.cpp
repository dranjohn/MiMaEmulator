#include "Command.h"

#include <fmt/format.h>

namespace MiMaCLI {
	const std::regex Command::wordPattern(R"(([^\s]+))");


	CommandResult Command::execute(const std::string&) const {
		return { true, "Attempted to execute default command construct" };
	}



	UniversalCommand::UniversalCommand(const CommandExecution& commandExecution) :
		commandExecution(commandExecution)
	{}



	ConditionalCommand::ConditionalCommand(const NamedCommandPointers& commands) {
		NamedCommandPointers::const_iterator it = commands.begin();
		while (it != commands.end()) {
			(this->commands).insert({ it->first, std::unique_ptr<Command>(it->second) });
			++it;
		}
	}

	CommandResult ConditionalCommand::execute(const std::string& input) const {
		std::smatch matches;
		if (!std::regex_search(input, matches, wordPattern)) {
			return { false, "Can't find a subcommand for an empty string" };
		}

		NamedCommands::const_iterator foundCommand = commands.find(matches[1]);
		if (foundCommand == commands.end()) {
			return { false, fmt::format("Couldn't find (sub)command '{}'", matches[1].str()) };
		}

		return (foundCommand->second)->execute(matches.suffix());
	}
}
