#pragma once

#include "Address.h"
#include "Angle.h"

typedef struct  Neighbor{
	Address address;
	Angle     startangle;
	Angle     endangle;
	Neighbor(Address a, Angle start, Angle end) : address(a), startangle(start), endangle(end){};
} Neighbor;