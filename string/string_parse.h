// TODO - replace
#include <cstdlib>

int int_parse(String string)
{
    // @HACK - fixed buffer to guarantee null-termination, since atoi expects it
    char zstr[1024];
    int length = min(1023, string.length);
    zstr[length] = '\0';
    mem_copy(zstr, string.data, length);

    // TODO - better API that includes errors... atoi silently returns 0
    int result = std::atoi(zstr);
    return result;
}
