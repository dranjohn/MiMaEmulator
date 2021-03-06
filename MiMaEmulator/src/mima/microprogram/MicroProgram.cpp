#include "mimapch.h"
#include "MicroProgram.h"


namespace MiMa {
	// ----------------------
	// Microprogram code list
	// ----------------------

	// --- List nodes ---

	MicroProgramCodeNode::MicroProgramCodeNode(const size_t& upperConditionLimit, MicroProgramCodeNode* next, MicroProgramCode code) :
		next(next),
		code(code),
		upperConditionLimit(upperConditionLimit)
	{}

	MicroProgramCodeNode::~MicroProgramCodeNode() {
		//the nodes of the singly linked list recursively delete the next one
		if (next != nullptr) delete next;
	}


	// --- List class ---

	MicroProgramCodeList::MicroProgramCodeList(const std::string& conditionName, const size_t& conditionMax) :
		conditionName(conditionName),
		conditionMax(conditionMax),
		head(new MicroProgramCodeNode(conditionMax))
	{}

	MicroProgramCodeList::~MicroProgramCodeList() {
		//delete the head node, which results in recursive deletion of all nodes
		delete head;
	}


	void MicroProgramCodeList::reset() {
		//default: reset the list to a single code with no condition
		reset("", 0);
	}

	void MicroProgramCodeList::reset(const std::string& conditionName, const size_t& conditionMax) {
		//set new condition name and maximum
		this->conditionName = conditionName;
		this->conditionMax = conditionMax;

		//delete and reset list head
		delete head;
		head = new MicroProgramCodeNode(conditionMax);
	}


	void MicroProgramCodeList::apply(const std::function<void(MicroProgramCode&)>& func, const size_t& lowerLimit, size_t upperLimit) {
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

	MicroProgramCode MicroProgramCodeList::get(const StatusBitMap& statusBits) const {
		//find the value of the given condition
		//if it is not in the map, it defaults to zero
		StatusBitMap::const_iterator conditionLocation = statusBits.find(conditionName);

		size_t condition = 0;
		if (conditionLocation != statusBits.end()) {
			condition = std::min(conditionLocation->second, conditionMax);
		}

		//return microprogram code dependent on the condition value
		MicroProgramCodeNode* current = head;
		while (current->upperConditionLimit < condition) {
			current = current->next;
		}

		return current->code;
	}
}
