#include "LogFormat.h"

#include <vector>
#include <sstream>

namespace MiMa {
	constexpr unsigned char char_ELEMENT = 0xC3; //box-building char top-right-bot
	constexpr unsigned char char_LAST_ELEMENT = 0xC0; //box-building char top-right
	constexpr unsigned char char_SKIP_ELEMENT = 0xB3; //box-building char top-bot
	constexpr unsigned char char_INDENT = 0x20; //space

	typedef typename std::vector<DataNode<std::string>>::const_iterator StringNodeIterator;
	void addChildren(StringNodeIterator iterator, const StringNodeIterator& end, std::ostream& output, std::vector<unsigned char>& format = std::vector<unsigned char>());

	std::string formatHierarchy(const Tree<std::string>& hierarchy) {
		std::stringstream output;
		const DataNode<std::string>& root = hierarchy.getConstRoot();

		output << root.getData() << "\n";
		addChildren(root.getChildren(), root.getChildrenEnd(), output);

		return output.str();
	}

	void addChildren(StringNodeIterator iterator, const StringNodeIterator& end, std::ostream& output, std::vector<unsigned char>& format) {
		while (iterator != end) {
			std::string data = iterator->getData();
			StringNodeIterator& children = iterator->getChildren();
			const StringNodeIterator& childrenEnd = iterator->getChildrenEnd();

			++iterator;

			for (const unsigned char& formatChar : format) {
				output << formatChar << char_INDENT;
			}

			if (iterator == end) {
				output << char_LAST_ELEMENT;
				format.push_back(char_INDENT);
			}
			else {
				output << char_ELEMENT;
				format.push_back(char_SKIP_ELEMENT);
			}
			output << char_INDENT << data << "\n";

			addChildren(children, childrenEnd, output, format);
			

			if (!format.empty()) {
				format.pop_back();

				while (!format.empty() && format.back() == char_INDENT && iterator == end) {
					format.pop_back();
				}
			}
		}
	}
}
