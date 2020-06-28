#include "random.h"
#include <cstdlib>

static unsigned int generate_default_seed()
{
	std::random_device rd;
	const auto rd_seed = rd();
	return rd_seed;
}

std::default_random_engine global_rand_engine(generate_default_seed());



