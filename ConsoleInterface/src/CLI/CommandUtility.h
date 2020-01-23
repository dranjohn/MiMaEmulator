#pragma once

//std library
#include <string>
#include <vector>
#include <regex>


namespace MiMaCLI {
	namespace CommandUtility {
		std::vector<std::string> getArguments(const std::string& input);
		std::vector<std::string> getArguments(const std::string& input, const size_t& sizeAssertion);

		void assertNoArguments(const std::string& input);

		void validateIdentifier(const std::string& identifier, const std::regex& identifierPattern);
		size_t validatePositiveDecimalInteger(const std::string& number);
	}
}
