#include "Command.h"

//external vendor libraries
#include <fmt/format.h>


namespace MiMaCLI {
	CommandResult Command::execute(const std::string&) const {
		return { true, "Attempted to execute default command construct" };
	}



	UniversalCommand::UniversalCommand(const CommandExecution& commandExecution) :
		commandExecution(commandExecution)
	{}



	const std::regex ConditionalCommand::wordPattern(R"(([^\s]+))");
	
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
			throw CommandException("can't find a subcommand when no subcommand is given");
		}

		NamedCommands::const_iterator foundCommand = commands.find(matches[1]);
		if (foundCommand == commands.end()) {
			throw CommandException(fmt::format("couldn't find (sub)command '{}'", matches[1].str()));
		}

		return (foundCommand->second)->execute(matches.suffix());
	}
}
