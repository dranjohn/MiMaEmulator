//std library
#include <memory>

//minimal machine
#include "mimaCLI/MiMaCommandExecutor.h"

//internal classes
#include "CLI/Command.h"
#include "CLI/CommandLineInterface.h"


int main() {
	std::shared_ptr<MiMaCLI::MiMaCLIState> state = std::make_shared<MiMaCLI::MiMaCLIState>();
	std::shared_ptr<MiMaCLI::MiMaCommandExecutor> mimaRootCommand = std::make_shared<MiMaCLI::MiMaCommandExecutor>(state);
	MiMaCLI::CommandLineInterface cli(mimaRootCommand);

	cli.run();

	return 0;
}