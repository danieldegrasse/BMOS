/** @file util.h
 * Provides common macros, such as bitmasking, for use throughout the program
 */
#ifndef UTIL_H
#define UTIL_H

#define CLEARBITS(x, y) (x) &= ~(y)
#define SETBITS(x, y) (x) |= (y)
#define CLEARFIELD(x, y, z) CLEARBITS((x), (y << z))
#define SETFIELD(x, y, z) SETBITS((x), (y << z))
#define READBITS(x, y) ((x) & (y))
#define READFIELD(x, y, z) READBITS((x), (y << z))

#define MODIFY_REG(REG, FIELDMASK, SETMASK)                                    \
    (REG) = (((REG) & ~(FIELDMASK)) | (SETMASK))

#endif