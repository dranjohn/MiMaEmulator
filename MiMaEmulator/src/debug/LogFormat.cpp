#include "LogFormat.h"

namespace MiMa {
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
