#include "my_actor_id_converter.h"

uint32_t CMyActorIDConverter::convertToServiceID(uint64_t nActorID)
{
	if (nActorID == 3)
		return 4;
	else if (nActorID == 2)
		return 5;

	return 0;
}