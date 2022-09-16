// Z80Plus API
//  ______  ______ ______    ___
// /\___  \/\  __ \\  __ \ _/\  \___
// \/__/  /\_\  __ \\ \/\ \\___   __\
//    /\_____\\_____\\_____\__/\__\_/
//    \/_____//_____//_____/  \/__/
// Copyright (C) 2021-2022 Sofía Ortega Sosa.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef Z80Plus_HPP
#define Z80Plus_HPP

#include <Z80.h>

#ifndef Z80_PLUS_API
#	if defined(Z80_PLUS_STATIC) || defined(__DOXYGEN__)
#		define Z80_PLUS_API
#	else
#		define Z80_PLUS_API Z_API_IMPORT
#	endif
#endif

#define Z80_PLUS_LIBRARY_VERSION_MAJOR  0
#define Z80_PLUS_LIBRARY_VERSION_MINOR  1
#define Z80_PLUS_IBRARY_VERSION_MICRO   0
#define Z80_PLUS_LIBRARY_VERSION_STRING "0.1"


struct Z80Plus : Z80 {
	using Read    = Z80Read;
	using Write   = Z80Write;
	using Halt    = Z80HALT;
	using Notify  = Z80Notify;
	using Illegal = Z80Illegal;

	static const zuint32 m_cycle_table[4][256];
	static const zuint32 no_mreq_table[4][256];

	Z80Read direct_read;
	zusize  accumulated_cycles;
	zuint8  instruction_cycles;
	zuint8  pattern_table_index;
	zuint8  pattern;

	Z80Plus() noexcept;


	inline Z80Plus &power(bool state) noexcept
		{
		z80_power(this, state);
		return *this;
		}


	inline Z80Plus &special_reset() noexcept
		{
		z80_special_reset(this);
		return *this;
		}


	inline Z80Plus &reset() noexcept
		{
		z80_instant_reset(this);
		return *this;
		}


	inline Z80Plus &irq(bool state) noexcept
		{
		z80_int(this, state);
		return *this;
		}


	inline Z80Plus &nmi() noexcept
		{
		z80_nmi(this);
		return *this;
		}


	inline zuint16 refresh_address() const noexcept
		{return z80_refresh_address(this);}


	inline zusize cycle() const noexcept
		{return accumulated_cycles + cycles + instruction_cycles;}


	inline zuint8 in_cycle() const noexcept
		{return z80_in_cycle(this);}


	inline zuint8 out_cycle() const noexcept
		{return z80_out_cycle(this);}


	inline zuint8 full_r() const noexcept
		{return z80_r(this);}


	inline Z80Plus &execute() noexcept
		{
		accumulated_cycles += z80_execute(this, 1);
		return *this;
		}


	inline Z80Plus &execute(zusize cycles_) noexcept
		{
		accumulated_cycles += z80_execute(this, cycles_);
		return *this;
		}


	inline Z80Plus &execute_until(zusize cycle) noexcept
		{
		if (cycle > accumulated_cycles)
			accumulated_cycles += z80_execute(this, cycle - accumulated_cycles);

		return *this;
		}


	inline Z80Plus &run() noexcept
		{
		accumulated_cycles += z80_run(this, 1);
		return *this;
		}


	inline Z80Plus &run(zusize cycles_) noexcept
		{
		accumulated_cycles += z80_run(this, cycles_);
		return *this;
		}


	inline Z80Plus &run_until(zusize cycle) noexcept
		{
		if (cycle > accumulated_cycles)
			accumulated_cycles += z80_run(this, cycle - accumulated_cycles);

		return *this;
		}


	inline void m(zuint8 opcode) noexcept
		{
		pattern_table_index = (pattern = m_cycle_table[pattern_table_index][opcode]) & 3;
		instruction_cycles = pattern >> 28;
		}


	inline void m() noexcept
		{instruction_cycles += (pattern >>= 4) & 0xF;}


	zuint8 m_no_mreq(zuint8 opcode) noexcept;


	inline zuint8 m_no_mreq() noexcept
		{return (pattern >>= 4) & 0xF;}


	/* 16-bit register accessors */

	inline zuint16 &pc    () noexcept {return Z80::pc.uint16_value;}
	inline zuint16 &sp    () noexcept {return Z80::sp.uint16_value;}
	inline zuint16 &memptr() noexcept {return Z80::memptr.uint16_value;}
	inline zuint16 &xy    () noexcept {return Z80::xy.uint16_value;}
	inline zuint16 &ix    () noexcept {return Z80::ix_iy[0].uint16_value;}
	inline zuint16 &iy    () noexcept {return Z80::ix_iy[1].uint16_value;}
	inline zuint16 &af    () noexcept {return Z80::af.uint16_value;}
	inline zuint16 &bc    () noexcept {return Z80::bc.uint16_value;}
	inline zuint16 &de    () noexcept {return Z80::de.uint16_value;}
	inline zuint16 &hl    () noexcept {return Z80::hl.uint16_value;}
	inline zuint16 &af_   () noexcept {return Z80::af_.uint16_value;}
	inline zuint16 &bc_   () noexcept {return Z80::bc_.uint16_value;}
	inline zuint16 &de_   () noexcept {return Z80::de_.uint16_value;}
	inline zuint16 &hl_   () noexcept {return Z80::hl_.uint16_value;}

	/* 8-bit register accessors */

	inline zuint8 &pch    () noexcept {return Z80::pc.uint8_values.at_1;}
	inline zuint8 &pcl    () noexcept {return Z80::pc.uint8_values.at_0;}
	inline zuint8 &sph    () noexcept {return Z80::sp.uint8_values.at_1;}
	inline zuint8 &spl    () noexcept {return Z80::sp.uint8_values.at_0;}
	inline zuint8 &memptrh() noexcept {return Z80::memptr.uint8_values.at_1;}
	inline zuint8 &memptrl() noexcept {return Z80::memptr.uint8_values.at_0;}
	inline zuint8 &xyh    () noexcept {return Z80::xy.uint8_values.at_1;}
	inline zuint8 &xyl    () noexcept {return Z80::xy.uint8_values.at_0;}
	inline zuint8 &ixh    () noexcept {return Z80::ix_iy[0].uint8_values.at_1;}
	inline zuint8 &ixl    () noexcept {return Z80::ix_iy[0].uint8_values.at_0;}
	inline zuint8 &iyh    () noexcept {return Z80::ix_iy[1].uint8_values.at_1;}
	inline zuint8 &iyl    () noexcept {return Z80::ix_iy[1].uint8_values.at_0;}
	inline zuint8 &a      () noexcept {return Z80::af.uint8_values.at_1;}
	inline zuint8 &f      () noexcept {return Z80::af.uint8_values.at_0;}
	inline zuint8 &b      () noexcept {return Z80::bc.uint8_values.at_1;}
	inline zuint8 &c      () noexcept {return Z80::bc.uint8_values.at_0;}
	inline zuint8 &d      () noexcept {return Z80::de.uint8_values.at_1;}
	inline zuint8 &e      () noexcept {return Z80::de.uint8_values.at_0;}
	inline zuint8 &h      () noexcept {return Z80::hl.uint8_values.at_1;}
	inline zuint8 &l      () noexcept {return Z80::hl.uint8_values.at_0;}
	inline zuint8 &a_     () noexcept {return Z80::af_.uint8_values.at_1;}
	inline zuint8 &f_     () noexcept {return Z80::af_.uint8_values.at_0;}
	inline zuint8 &b_     () noexcept {return Z80::bc_.uint8_values.at_1;}
	inline zuint8 &c_     () noexcept {return Z80::bc_.uint8_values.at_0;}
	inline zuint8 &d_     () noexcept {return Z80::de_.uint8_values.at_1;}
	inline zuint8 &e_     () noexcept {return Z80::de_.uint8_values.at_0;}
	inline zuint8 &h_     () noexcept {return Z80::hl_.uint8_values.at_1;}
	inline zuint8 &l_     () noexcept {return Z80::hl_.uint8_values.at_0;}
};


#endif // Z80Plus_HPP
