#include "attribute.h"

Id new_attr_id()
{
	static Id next_id = 1;
	return next_id++;
}