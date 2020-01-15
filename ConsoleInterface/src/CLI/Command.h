#pragma once

#include <exception>
#include <functional>
#include <regex>
#include <string>
#include <unordered_map>

namespace MiMaCLI {
	struct CommandResult { bool end = false; std::string output = "---"; };

	class Command {
	public:
		static const std::regex wordPattern;

	public:
		virtual CommandResult execute(const std::string& input) const;
	};


	typedef std::function<CommandResult(const std::string&)> CommandExecution;

	class UniversalCommand : public Command {
	private:
		CommandExecution commandExecution;

	public:
		UniversalCommand(const CommandExecution& commandExecution);

		inline CommandResult execute(const std::string& input) const override { return commandExecution(input); }
	};


	typedef std::unordered_map<std::string, std::unique_ptr<Command>> NamedCommands;
	typedef std::unordered_map<std::string, Command*> NamedCommandPointers;

	class ConditionalCommand : public Command {
	private:
		NamedCommands commands;

	public:
		ConditionalCommand(const NamedCommandPointers& commands);

		CommandResult execute(const std::string& input) const override;
	};



	class CommandException : public std::exception {
	private:
		const std::string s_cause;
		const std::exception e_cause;

	public:
		CommandException(const std::string& cause) : s_cause(cause), e_cause(nullptr) {}
		CommandException(const std::exception& cause) : s_cause(cause.what()), e_cause(cause) {}

		virtual const char* what() const throw() {
			return s_cause.c_str();
		}
	};
}
