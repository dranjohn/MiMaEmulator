#include "LogFormat.h"

namespace MiMa {
	void HierarchyFormat::addElement(std::string content, bool last) {
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

	void HierarchyFormat::addChild(std::string content, bool last) {
		if (completed) {
			completed = false;
			indent++;
		}
		else {
			skip++;
		}

		addElement(content, last);
	}


	void HierarchyFormat::emptyLine() {
		if (completed) {
			//TODO: warn
			return;
		}

		for (unsigned int i = 0; i < indent + skip + 1; i++) {
			buffer << ((i < indent) ? char_INDENT : char_SKIP_ELEMENT) << char_INDENT;
		}
		buffer << std::endl;
	}
}
