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
			std::cout << "<< ";
			try {
				CommandResult result = rootCommand->execute(input);

				std::cout << result.output << std::endl;

				if (result.end) {
					break;
				}
			}
			catch (const CommandException& exc) {
				std::cout << "Failed to execute '" << input << "': " << exc.what() << std::endl;
			}

			std::cout << ">> ";
		}
	}
}
