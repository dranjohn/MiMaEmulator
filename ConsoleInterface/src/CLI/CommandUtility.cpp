#include "CommandUtility.h"

//external vendor libraries
#include <fmt/format.h>

//internal classes
#include "Command.h"


namespace MiMaCLI {
	namespace CommandUtility {
		static const std::regex wordPattern(R"(([^\s]+))");
		static const std::regex emptyLinePattern(R"(\s*)");

		static const std::regex positiveDecimalPattern(R"(\+?(?:[1-9][0-9]*|0))");


		std::vector<std::string> getArguments(const std::string& input) {
			std::sregex_iterator argumentsBegin = std::sregex_iterator(input.begin(), input.end(), wordPattern);
			std::sregex_iterator argumentsEnd = std::sregex_iterator();

			std::vector<std::string> arguments;
			while (argumentsBegin != argumentsEnd) {
				arguments.push_back((*argumentsBegin)[0]);
				++argumentsBegin;
			}

			return arguments;
		}

		std::vector<std::string> getArguments(const std::string& input, const size_t& sizeAssertion) {
			std::vector<std::string> arguments = getArguments(input);

			if (arguments.size() != sizeAssertion) {
				throw CommandException(fmt::format("expected {} arguments, got {}", sizeAssertion, arguments.size()));
			}

			return arguments;
		}


		void assertNoArguments(const std::string& input) {
			if (!std::regex_match(input, emptyLinePattern)) {
				throw CommandException("expected no arguments");
			}
		}


		void validateIdentifier(const std::string& identifier, const std::regex& identifierPattern) {
			if (!std::regex_match(identifier, identifierPattern)) {
				throw CommandException(fmt::format("'{}' is not a valid identifier", identifier));
			}
		}

		size_t validatePositiveDecimalInteger(const std::string& number) {
			if (!std::regex_match(number, positiveDecimalPattern)) {
				throw CommandException(fmt::format("'{}' doesn't match the format of a positive decimal number", number));
			}

			return (size_t)std::stoi(number);
		}
	}
}
