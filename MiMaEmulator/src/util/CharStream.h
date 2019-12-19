#pragma once

#include <istream>

class CharStream {
public:
	virtual char get() = 0;
};

class BufferedCharStream : public CharStream {
private:
	char* buffer;
public:
	BufferedCharStream(char* buffer) : buffer(buffer) {}; //TODO: fix broken ownership

	char get() override {
		char toReturn = *buffer;
		if (toReturn != 0) {
			buffer++;
		}

		return toReturn;
	}
};

class InputCharStream : public CharStream {
private:
	std::istream& input;
public:
	InputCharStream(std::istream& input) : input(input) {}; //TODO: fix broken ownership

	char get() override {
		char toReturn;
		input >> toReturn;
		
		return toReturn;
	}
};
