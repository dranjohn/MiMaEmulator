#pragma once

#include <exception>
#include <string>

namespace MiMa {
	class CompilerException : public std::exception {
	private:
		std::string exceptionCause;

	public:
		CompilerException(const std::string& exceptionCause) : exceptionCause(exceptionCause) {}

		virtual const char* what() const throw() {
			return exceptionCause.c_str();
		}
	};
}
