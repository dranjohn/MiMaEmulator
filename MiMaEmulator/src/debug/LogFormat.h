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

		template <typename Content>
		void addElement(Content content, bool last = false) {
			if (completed) {
				//TODO: warn
			}

			for (unsigned int i = 0; i < indent + skip; i++) {
				buffer << ((i < indent) ? char_INDENT : char_SKIP_ELEMENT) << char_INDENT;
			}

			if (last) {
				buffer << char_LAST_ELEMENT;

				if (skip == 0) {
					completed = true;
				}
				else {
					skip--;
				}
			}
			else {
				buffer << char_ELEMENT;
			}

			buffer << char_INDENT << content << std::endl;

			if (last && !completed) {
				emptyLine();
			}
		}
		template <typename Content>
		void addChild(Content content, bool last = false) {
			if (completed) {
				completed = false;
				indent++;
			}
			else {
				skip++;
			}

			addElement(content, last);
		}

		void emptyLine();
	};
}
