/* Z80Clock API
       ______  ______ ______
      /\___  \/\  __ \\  __ \
 ____ \/__/  /\_\  __ \\ \/\ \ ________________________________________________
|        /\_____\\_____\\_____\                                                |
|  Zilog \/_____//_____//_____/ CPU Emulator Clock                             |
|  Copyright (C) 2021-2022 Sofía Ortega Sosa.                                  |
|                                                                              |
|  Permission is hereby granted, free of charge, to any person obtaining a     |
|  copy of this software and associated documentation files (the "Software"),  |
|  to deal in the Software without restriction, including without limitation   |
|  the rights to use, copy, modify, merge, publish, distribute, sublicense,    |
|  and/or sell copies of the Software, and to permit persons to whom the       |
|  Software is furnished to do so, subject to the following conditions:        |
|                                                                              |
|  The above copyright notice and this permission notice shall be included in  |
|  all copies or substantial portions of the Software.                         |
|                                                                              |
|  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  |
|  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    |
|  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL    |
|  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  |
|  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     |
|  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         |
|  DEALINGS IN THE SOFTWARE.                                                   |
|                                                                              |
'=============================================================================*/

#ifndef Z80Clock_H
#define Z80Clock_H

#include <Z80.h>

#ifndef Z80_CLOCK_API
#	if defined(Z80_CLOCK_STATIC) || defined(__DOXYGEN__)
#		define Z80_CLOCK_API
#	else
#		define Z80_CLOCK_API Z_API_IMPORT
#	endif
#endif

#define Z80_CLOCK_LIBRARY_VERSION_MAJOR  0
#define Z80_CLOCK_LIBRARY_VERSION_MINOR  1
#define Z80_CLOCK_LIBRARY_VERSION_MICRO  0
#define Z80_CLOCK_LIBRARY_VERSION_STRING "0.1"

typedef struct {
	Z80*    cpu;
	Z80Read read;
	void*   read_context;
	zuint32 pattern;
	zuint8  pattern_table_index;
	zuint8  cycles;
} Z80Clock;


static Z_INLINE void z80_clock_initialize(Z80Clock *self, Z80 *cpu, Z80Read read)
	{
	self->cpu = cpu;
	self->read = read;
	self->cycles = 0;
	}


static Z_INLINE zuint8 z80_clock_no_mreq_cycles(Z80Clock *self)
	{return (self->pattern >>= 4) & 0xF;}


static Z_INLINE void z80_clock_advance_m1(Z80Clock *self, zuint8 no_mreq_cycles)
	{self->cycles += 4 + no_mreq_cycles;}


static Z_INLINE void z80_clock_advance_read_write(Z80Clock *self, zuint8 no_mreq_cycles)
	{self->cycles += 3 + no_mreq_cycles;}


static Z_INLINE void z80_clock_advance_io(Z80Clock *self)
	{self->cycles += 4;}


Z80_CLOCK_API zuint8 z80_clock_m1(Z80Clock *self, zuint8 opcode);


#endif /* Z80Clock_H */
