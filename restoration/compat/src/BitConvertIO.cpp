#include "IO/BitConvertIO.h"

unsigned long BitIO::getshiftbit(unsigned long mask) const {
    if (!mask) {
        return 0;
    }
    unsigned long shift = 0;
    while ((mask & 1UL) == 0UL) {
        mask >>= 1UL;
        ++shift;
    }
    return shift;
}

unsigned long BitIO::getnbbits(unsigned long mask) const {
    unsigned long count = 0;
    while (mask) {
        count += (mask & 1UL);
        mask >>= 1UL;
    }
    return count;
}
