#pragma once

#include <memory>
#include <string>
#include <sstream>

namespace MiMa {
	constexpr char char_ELEMENT = 0xC3;
	constexpr char char_LAST_ELEMENT = 0xC0;
	constexpr char char_SKIP_ELEMENT = 0xB3;
	constexpr char char_INDENT = 0x20;
	
	class HierarchyFormat {
	private:
		bool completed = false;
		unsigned int indent = 0;
		unsigned int skip = 0;

		std::stringstream buffer;
	public:
		template<typename Content>
		HierarchyFormat(const Content& root) { buffer << root << std::endl; };

		inline std::string getBuffer() const { return buffer.str(); };

		void addElement(std::string content, bool last = false);
		void addChild(std::string content, bool last = false);

		void emptyLine();
	};
}
