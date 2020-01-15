#pragma once

#include <memory>

#include "Command.h"

namespace MiMaCLI {
	class CommandLineInterface {
	private:
		std::shared_ptr<Command> rootCommand;

	public:
		CommandLineInterface(const std::shared_ptr<Command>& rootCommand);

		void run();
	};
}
