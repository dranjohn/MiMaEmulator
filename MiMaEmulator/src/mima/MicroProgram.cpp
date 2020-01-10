#include "MicroProgram.h"

namespace MiMa {
	MicroProgramCodeNode::MicroProgramCodeNode(const size_t& upperConditionLimit, MicroProgramCodeNode* next, UnconditionalMicroProgramCode code) :
		next(next),
		code(code),
		upperConditionLimit(upperConditionLimit)
	{}



	MicroProgramCodeList::MicroProgramCodeList(const std::string& conditionName, const size_t& conditionMax) :
		conditionName(conditionName),
		conditionMax(conditionMax),
		head(new MicroProgramCodeNode(conditionMax))
	{}

	MicroProgramCodeList::~MicroProgramCodeList() {
		MicroProgramCodeNode* current = head;
		MicroProgramCodeNode* next;

		while (current != nullptr) {
			next = current->next;
			delete current;

			current = next;
		}
	}


	void MicroProgramCodeList::apply(const std::function<void(UnconditionalMicroProgramCode&)>& func, const size_t& lowerLimit, size_t upperLimit) {
		upperLimit = std::min(upperLimit, conditionMax);
		if (lowerLimit > upperLimit) {
			return;
		}

		MicroProgramCodeNode* prev = nullptr;
		MicroProgramCodeNode* current = head;

		//find the first node which is affected by the application of the microprogram code function,
		//i.e. the first node which has an upper limit equal or above the lower limit of the effect
		//=> the previous node has an upper limit equal or below the lower limit
		while (current->upperConditionLimit < lowerLimit) {
			prev = current;
			current = current->next;
		}

		//in case the previous node has an upper limit below the lower limit
		if (prev == nullptr) {
			if (lowerLimit != 0) {
				size_t limitingNodeLimit = lowerLimit - 1;

				head = new MicroProgramCodeNode(limitingNodeLimit, head, head->code);
				prev = head;
			}
		}
		else {
			if (prev->upperConditionLimit + 1 != lowerLimit) {
				size_t limitingNodeLimit = lowerLimit - 1;

				prev->next = new MicroProgramCodeNode(limitingNodeLimit, current, current->code);
				prev = prev->next;
			}
		}

		//apply function to all nodes with an upper limit under the given upper limit
		while (current->upperConditionLimit < upperLimit) {
			func(current->code);

			prev = current;
			current = current->next;
		}

		//if there is a node matching the given upper limit, apply the function to it as well and return
		if (current->upperConditionLimit == upperLimit) {
			func(current->code);
			return;
		}

		//if there is no node matching the given upper limit, create one and apply the function to it as well
		MicroProgramCodeNode* upperLimitNode = new MicroProgramCodeNode(upperLimit, current, current->code);

		if (prev == nullptr) {
			head = upperLimitNode;
		}
		else {
			prev->next = upperLimitNode;
		}
		func(upperLimitNode->code);
	}

	UnconditionalMicroProgramCode& MicroProgramCodeList::get(const size_t& condition) {
		size_t upperLimit = std::min(condition, conditionMax);
		MicroProgramCodeNode* current = head;

		while (current->upperConditionLimit < upperLimit) {
			current = current->next;
		}

		return current->code;
	}
}
