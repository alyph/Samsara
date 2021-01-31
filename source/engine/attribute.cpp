#include "attribute.h"

static Id next_id = 1;

Id new_attr_id()
{
	return next_id++;
}

