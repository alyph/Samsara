#pragma once

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define LEN(array) (sizeof(array)/sizeof(array[0]))