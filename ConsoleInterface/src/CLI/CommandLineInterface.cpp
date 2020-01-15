#include "CommandLineInterface.h"

#include <iostream>
#include <string>

namespace MiMaCLI {
	CommandLineInterface::CommandLineInterface(const std::shared_ptr<Command>& rootCommand) :
		rootCommand(rootCommand)
	{}


	void CommandLineInterface::run() {
		std::string input;

		std::cout << ">> ";
		while (std::getline(std::cin, input)) {
			CommandResult result;

			try {
				result = rootCommand->execute(input);
			}
			catch (const CommandException& exc) {
				result = { false, exc.what() };
			}

			std::cout << "<< " << result.output << std::endl;

			if (result.end) {
				break;
			}

			std::cout << ">> ";
		}
	}
}
