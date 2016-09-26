#pragma once

#include <stdint.h>

class Message {

public:
	uint8_t length = 0;
	uint8_t * data = 0;

	Message(){};

	Message(uint8_t length) {
		this->length = length;
		this->data = new uint8_t[length];
	};

	~Message(){
		if (this->data != nullptr)
			delete this->data;
		this->data = nullptr;
		this->length = 0;
	};
};