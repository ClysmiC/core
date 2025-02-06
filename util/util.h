#include "direction2d.h"
#include "sort.h"
#include "misc.h"

inline bool
char_is_is_whitespace(char c)
{
    bool result = (c == ' ' || c == '\n' || c == '\t' || c == '\r');
    return result;
}
