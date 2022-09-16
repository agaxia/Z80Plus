// Z80Plus v0.1
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


#ifdef Z80_PLUS_STATIC
#	define Z80_PLUS_API
#else
#	define Z80_PLUS_API Z_API_EXPORT
#endif

#include <Z80Plus.hpp>

                                         /*----------------------------------------------------------------------------------------------------------------.
8-Bit Load _____________________________ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define ld_J_K                000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define ld_O_P                000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define ld_J_BYTE            0000,    34 //  7:43          |            | opcode(pc)     | argument(pc+1) |                  |             |               |
#define ld_O_BYTE            0000,    34 // 11:443         | prefix(pc) | opcode(pc+1)   | argument(pc+2) |                  |             |               |
#define ld_J_vhl             0000,    34 //  7:43          |            | opcode(pc)     | read(hl)       |                  |             |               |
#define ld_J_vXYpOFFSET     05000,   384 // 19:44353       | prefix(pc) | opcode(pc+1)   | argument(pc+2) | %:5              | read(EA)    |               |
#define ld_vhl_K             0000,    34 //  7:43          |            | opcode(pc)     | write(hl)      |                  |             |               |
#define ld_vXYpOFFSET_K     05000,   384 // 19:44353       | prefix(pc) | opcode(pc+1)   | argument(pc+2) | %:5              | write(EA)   |               |
#define ld_vhl_BYTE         00000,   334 // 10:433         |            | opcode(pc)     | argument(pc+1) | write(hl)        |             |               |
#define ld_vXYpOFFSET_BYTE 020000,  3534 // 19:44353       | prefix(pc) | opcode(pc+1)   | argument(pc+2) | argument(pc+3):2 | write(EA)   |               |
#define ld_a_vbc             0000,    34 // 7:43           |            | opcode(pc)     | read(bc)       |                  |             |               |
#define ld_a_vde             0000,    34 // 7:43           |            | opcode(pc)     | read(de)       |                  |             |               |
#define ld_a_vWORD         000000,  3334 // 13:4333        |            | opcode(pc)     | argument(pc+1) | argument(pc+2)   | read(WORD)  |               |
#define ld_vbc_a             0000,    34 //  7:43          |            | opcode(pc)     | write(bc)      |                  |             |               |
#define ld_vde_a             0000,    34 //  7:43          |            | opcode(pc)     | write(de)      |                  |             |               |
#define ld_vWORD_a         000000,  3334 // 13:4333        |            | opcode(pc)     | argument(pc+1) | argument(pc+2)   | write(WORD) |               |
#define ld_a_i                100,     5 //  9:45          | perfix(pc) | opcode(pc):1   |                |                  |             |               |
#define ld_a_r                100,     5 //  9:45          | perfix(pc) | opcode(pc):1   |                |                  |             |               |
#define ld_i_a                100,     5 //  9:45          | perfix(pc) | opcode(pc):1   |                |                  |             |               |
#define ld_r_a                100,     5 //  9:45          | perfix(pc) | opcode(pc):1   |                |                  |             |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
16-Bit Load ____________________________ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define ld_SS_WORD          00000,   334 // 10:433         |            | opcode(pc)     | argument(pc+1) | argument(pc+2)   |             |               |
#define ld_XY_WORD          00000,   334 // 14:4433        | prefix(pc) | opcode(pc+1)   | argument(pc+2) | argument(pc+3)   |             |               |
#define ld_hl_vWORD         00000,   334 // 16:43333       |            | opcode(pc)     | argument(pc+1) | argument(pc+2)   | read(WORD)  | read(WORD+1)  |
#define ld_SS_vWORD       0000000, 33334 // 20:443333      | prefix(pc) | opcode(pc+1)   | argument(pc+2) | argument(pc+3)   | read(WORD)  | read(WORD+1)  |
#define ld_XY_vWORD       0000000, 33334 // 20:443333      | prefix(pc) | opcode(pc+1)   | argument(pc+2) | argument(pc+3)   | read(WORD)  | read(WORD+1)  |
#define ld_vWORD_hl       0000000, 33334 // 16:43333       |            | opcode(pc)     | argument(pc+1) | argument(pc+2)   | write(WORD) | write(WORD+1) |
#define ld_vWORD_SS       0000000, 33334 // 20:443333      | prefix(pc) | opcode(pc+1)   | argument(pc+2) | argument(pc+3)   | write(WORD) | write(WORD+1) |
#define ld_vWORD_XY       0000000, 33334 // 20:443333      | prefix(pc) | opcode(pc+1)   | argument(pc+2) | argument(pc+3)   | write(WORD) | write(WORD+1) |
#define ld_sp_hl              200,     6 //  6:6           |            | opcode(pc):2   |                |                  |             |               |
#define ld_sp_XY              200,     6 // 10:46          | prefix(pc) | opcode(pc+1):2 |                |                  |             |               |
#define push_TT             00100,   335 // 11:533         |            | opcode(pc):1   | write(sp-1)    | write(sp-2)      |             |               |
#define push_XY             00100,   335 // 15:4533        | prefix(pc) | opcode(pc+1):1 | write(sp-1)    | write(sp-2)      |             |               |
#define pop_TT              00000,   334 // 10:433         |            | opcode(pc)     | read(sp)       | read(sp+1)       |             |               |
#define pop_XY              00000,   334 // 14:4433        | prefix(pc) | opcode(pc+1)   | read(sp)       | read(sp+1)       |             |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
Exchange, Block Transfer and Search ____ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define ex_de_hl              000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define ex_af_af_             000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define exx                   000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define ex_vsp_hl         2010000, 53434 // 19:43435       |            | opcode(pc)     | read(sp)       | read(sp+1):1     | write(sp+1) | write(sp):2   |
#define ex_vsp_XY         2010000, 53434 // 23:443435      | prefix(pc) | opcode(pc+1)   | read(sp)       | read(sp+1):1     | write(sp+1) | write(sp):2   |
#define ldX                 20000,   534 // 16:4435        | prefix(pc) | opcode(pc+1)   | read(hl)       | write(de):2      |             |               |
#define ldXr                700D8,   A34 // 21:44355       | prefix(pc) | opcode(pc+1)   | read(hl)       | write(de):2      | %:5         |               |
#define ldXr_false        0x20000        // 16:4435        | prefix(pc) | opcode(pc+1)   | read(hl)       | write(de):2      |             |               |
#define cpX                  5000,    84 // 16:4435        | prefix(pc) | opcode(pc+1)   | read(hl)       | %:5              |             |               |
#define cpXr                 A0E8,    D4 // 21:44355       | prefix(pc) | opcode(pc+1)   | read(hl)       | %:5              | %:5         |               |
#define cpXr_false         0x5000        // 16:4435        | prefix(pc) | opcode(pc+1)   | read(hl)       | %:5              |             |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
8-Bit Arithmetic and Logic _____________ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define U_a_K                 000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define U_a_P                 000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define U_a_BYTE             0000,    34 //  7:43          |            | opcode(pc)     | argument(pc+1) |                  |             |               |
#define U_a_vhl              0000,    34 //  7:43          |            | opcode(pc)     | read(hl)       |                  |             |               |
#define U_a_vXYpOFFSET      05000,   384 // 19:44353       | prefix(pc) | opcode(pc+1)   | argument(pc+2) | %:5              | read(EA)    |               |
#define V_J                   000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define V_O                   000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define V_vhl                1000,    44 // 11:443         |            | opcode(pc)     | read(hl):1     | write(hl)        |             |               |
#define V_vXYpOFFSET       015000,  3484 // 23:443543      | prefix(pc) | opcode(pc+1)   | argument(pc+2) | %:5              | read(EA):1  | write(EA)     |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
General-Purpose Arithmetic and Control _ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define daa                   000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define cpl                   000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define neg                   000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define ccf                   000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define scf                   000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define nop                   000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define halt                  000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define di                    000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define ei                    000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define im_0                  000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define im_1                  000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define im_2                  000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
16-Bit Arithmetic ______________________ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define add_hl_SS             700,     B // 11:443         |            | opcode(pc)     | %:4            | %:3              |             |               |
#define adc_hl_SS             700,     B // 15:4443        | prefix(pc) | opcode(pc+1)   | %:4            | %:3              |             |               |
#define sbc_hl_SS             700,     B // 15:4443        | prefix(pc) | opcode(pc+1)   | %:4            | %:3              |             |               |
#define add_XY_WW             700,     B // 15:4443        | prefix(pc) | opcode(pc+1)   | %:4            | %:3              |             |               |
#define inc_SS                200,     6 //  6:6           |            | opcode(pc):2   |                |                  |             |               |
#define inc_XY                200,     6 // 10:46          | prefix(pc) | opcode(pc+1):2 |                |                  |             |               |
#define dec_SS                200,     6 //  6:6           |            | opcode(pc):2   |                |                  |             |               |
#define dec_XY                200,     6 // 10:46          | prefix(pc) | opcode(pc+1):2 |                |                  |             |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
Rotate and Shift _______________________ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define rlca                  000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define rla                   000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define rrca                  000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define rra                   000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define G_K                   000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define G_vhl               01000,   344 // 15:4443        | prefix(pc) | opcode(pc+1)   | read(hl):1     | write(hl)        |             |               |
#define rld                 04000,   374 // 18:44343       | prefix(pc) | opcode(pc+1)   | read(hl)       | %:4              | write(hl)   |               |
#define rrd                 04000,   374 // 18:44343       | prefix(pc) | opcode(pc+1)   | read(hl)       | %:4              | write(hl)   |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
Bit Set, Reset and Test ________________ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define bit_N_K               000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define bit_N_vhl            1000,    44 // 12:444         | prefix(pc) | opcode(pc+1)   | read(hl):1     |                  |             |               |
#define M_N_K                 000,     4 //  8:44          | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define M_N_vhl             01000,   344 // 15:4443        | prefix(pc) | opcode(pc+1)   | read(hl):1     | write(hl)        |             |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
Jump ___________________________________ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define jp_WORD             00000,   334 // 10:433         |            | opcode(pc)     | argument(pc+1) | argument(pc+2)   |             |               |
#define jp_Z_WORD           00000,   334 // 10:433         |            | opcode(pc)     | argument(pc+1) | argument(pc+2)   |             |               |
#define jr_OFFSET            5000,    84 // 12:435         |            | opcode(pc)     | argument(pc+1) | %:5              |             |               |
#define jr_nz_OFFSET         5018,    84 // 12:435         |            | opcode(pc)     | argument(pc+1) | %:5              |             |               |
#define jr_z_OFFSET          5028,    84 // "              |            | "              | "              | "                |             |               |
#define jr_nc_OFFSET         5038,    84 // "              |            | "              | "              | "                |             |               |
#define jr_c_OFFSET          5048,    84 // "              |            | "              | "              | "                |             |               |
#define jr_Z_OFFSET_false  0x0000        //  7:43          |            | opcode(pc)     | argument(pc+1) |                  |             |               |
#define jp_hl                 000,     4 //  4:4           |            | opcode(pc)     |                |                  |             |               |
#define jp_XY                 000,     4 //  8:44          |            | opcode(pc)     |                |                  |             |               |
#define djnz_OFFSET          5108,    85 // 13:535         |            | opcode(pc):1   | argument(pc+1) | %:5              |             |               |
#define djnz_OFFSET_false  0x0100        //  8:53          |            | opcode(pc):1   | argument(pc+1) |                  |             |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
Call and Return ________________________ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define call_WORD         0010000, 33434 //   17:43433     |            | opcode(pc)     | argument(pc+1) | argument(pc+2):1 | write(sp-1) | write(sp-2)   |
#define call_nz_WORD      0010058, 33434 //   17:43433     |            | opcode(pc)     | argument(pc+1) | argument(pc+2):1 | write(sp-1) | write(sp-2)   |
#define call_z_WORD       0010068, 33434 //    "           |            | "              | "              | "                | "           | "             |
#define call_nc_WORD      0010078, 33434 //    "           |            | "              | "              | "                | "           | "             |
#define call_c_WORD       0010088, 33434 //    "           |            | "              | "              | "                | "           | "             |
#define call_po_WORD      0010098, 33434 //    "           |            | "              | "              | "                | "           | "             |
#define call_pe_WORD      00100A8, 33434 //    "           |            | "              | "              | "                | "           | "             |
#define call_p_WORD       00100B8, 33434 //    "           |            | "              | "              | "                | "           | "             |
#define call_m_WORD       00100C8, 33434 //    "           |            | "              | "              | "                | "           | "             |
#define call_Z_WORD_false 0x00000        //   10:433       |            | opcode(pc)     | argument(pc+1) | argument(pc+2)   |             |               |
#define ret                 00000,   334 //   10:433       |            | opcode(pc)     | read(sp)       | read(sp+1)       |             |               |
#define ret_Z               00100,   335 // 11/5:5[33]     |            | opcode(pc):1   | [read(sp)]     | [read(sp+1)]     |             |               |
#define reti_retn           00000,   334 //   14:4433      | prefix(pc) | opcode(pc+1)   | read(sp)       | read(sp+1)       |             |               |
#define rst_N               00100,   335 //   11:533       |            | opcode(pc):1   | write(sp-1)    | write(sp-2)      |             |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
Input and Output _______________________ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define in_a_vBYTE           0000,   434 //    11:434      |            | opcode(pc)     | argument(pc+1) | in((a<<8)|BYTE)  |             |               |
#define in_J_vc               000,    44 //    12:444      | prefix(pc) | opcode(pc+1)   | in(bc)         |                  |             |               |
#define in_vc                 000,    44 //    12:444      | prefix(pc) | opcode(pc+1)   | in(bc)         |                  |             |               |
#define inX                  0100,   345 //    16:4543     | prefix(pc) | opcode(pc+1):1 | in(bc)         | write(hl)        |             |               |
#define inXr                 51F8,   845 //    21:45435    | prefix(pc) | opcode(pc+1):1 | in(bc)         | write(hl) %:5    |             |               |
#define out_vBYTE_a         00000,   434 //    11:434      |            | opcode(pc)     | argument(pc+1) | out((a<<8)|BYTE) |             |               |
#define out_vc_J             0000,    44 //    12:444      | prefix(pc) | opcode(pc+1)   | out(bc)        |                  |             |               |
#define out_vc_0             0000,    44 //    12:444      | prefix(pc) | opcode(pc+1)   | out(bc)        |                  |             |               |
#define outX                00100,   435 //    16:4534     | prefix(pc) | opcode(pc+1):1 | read(hl)       | out(bc)          |             |               |
#define otXr                501F8,   935 //    21:45345    | prefix(pc) | opcode(pc+1):1 | read(hl)       | out(bc)          | %:5         |               |
#define inXr_otXr_false   0x00100        //    16:4543     | prefix(pc) | opcode(pc+1):1 | in(bc)         | write(hl)        |             |               |
                                         //    16:4534     | prefix(pc) | opcode(pc+1):1 | read(hl)       | out(bc)          |             |               |
                                         /*----------------+------------+----------------+----------------+------------------+-------------+---------------|
Illegal Instructions and Prefixes ______ // T-states       | prefix M1  | M1             | M2             | M3               | M4          | M5            |
                                         //----------------+------------+----------------+----------------+------------------+-------------+--------------*/
#define ed_illegal            000,     4 //     8:44       | prefix(pc) | opcode(pc+1)   |                |                  |             |               |
#define xy_cb_prefix      0120000, 34534 // 23/20:44354[3] | prefix(pc) | opcode(pc+1)   | argument(pc+2) | argument(pc+3):2 | read(EA):1  | [write(EA)]   |
#define xy_prefix               2        //================================================================================================================'
#define cb_prefix               1
#define ed_prefix               3
#define P              0x40000000 |

#define PATTERN_TABLE                                                                                                                                                                                                                                                                                    \
                                                                                                                                                                                                                                                                                                         \
/* Unprefixed instructions */                                                                                                                                                                                                                                                                            \
                                                                                                                                                                                                                                                                                                         \
/*      0                1              2               3               4                5            6               7            8               9             A               B              C                D             E             F */                                                        \
/* 0 */{_(nop),          _(ld_SS_WORD), _(ld_vbc_a),    _(inc_SS),      _(V_J),          _(V_J),      _(ld_J_BYTE),   _(rlca),     _(ex_af_af_),   _(add_hl_SS), _(ld_a_vbc),    _(dec_SS),     _(V_J),          _(V_J),       _(ld_J_BYTE), _(rrca),                                                    \
/* 1 */ _(djnz_OFFSET),  _(ld_SS_WORD), _(ld_vde_a),    _(inc_SS),      _(V_J),          _(V_J),      _(ld_J_BYTE),   _(rla),      _(jr_OFFSET),   _(add_hl_SS), _(ld_a_vde),    _(dec_SS),     _(V_J),          _(V_J),       _(ld_J_BYTE), _(rra),                                                     \
/* 2 */ _(jr_nz_OFFSET), _(ld_SS_WORD), _(ld_vWORD_hl), _(inc_SS),      _(V_J),          _(V_J),      _(ld_J_BYTE),   _(daa),      _(jr_z_OFFSET), _(add_hl_SS), _(ld_hl_vWORD), _(dec_SS),     _(V_J),          _(V_J),       _(ld_J_BYTE), _(cpl),                                                     \
/* 3 */ _(jr_nc_OFFSET), _(ld_SS_WORD), _(ld_vWORD_a),  _(inc_SS),      _(V_vhl),        _(V_vhl),    _(ld_vhl_BYTE), _(scf),      _(jr_c_OFFSET), _(add_hl_SS), _(ld_a_vWORD),  _(dec_SS),     _(V_J),          _(V_J),       _(ld_J_BYTE), _(ccf),                                                     \
/* 4 */ _(ld_J_K),       _(ld_J_K),     _(ld_J_K),      _(ld_J_K),      _(ld_J_K),       _(ld_J_K),   _(ld_J_vhl),    _(ld_J_K),   _(ld_J_K),      _(ld_J_K),    _(ld_J_K),      _(ld_J_K),     _(ld_J_K),       _(ld_J_K),    _(ld_J_vhl),  _(ld_J_K),                                                  \
/* 5 */ _(ld_J_K),       _(ld_J_K),     _(ld_J_K),      _(ld_J_K),      _(ld_J_K),       _(ld_J_K),   _(ld_J_vhl),    _(ld_J_K),   _(ld_J_K),      _(ld_J_K),    _(ld_J_K),      _(ld_J_K),     _(ld_J_K),       _(ld_J_K),    _(ld_J_vhl),  _(ld_J_K),                                                  \
/* 6 */ _(ld_J_K),       _(ld_J_K),     _(ld_J_K),      _(ld_J_K),      _(ld_J_K),       _(ld_J_K),   _(ld_J_vhl),    _(ld_J_K),   _(ld_J_K),      _(ld_J_K),    _(ld_J_K),      _(ld_J_K),     _(ld_J_K),       _(ld_J_K),    _(ld_J_vhl),  _(ld_J_K),                                                  \
/* 7 */ _(ld_vhl_K),     _(ld_vhl_K),   _(ld_vhl_K),    _(ld_vhl_K),    _(ld_vhl_K),     _(ld_vhl_K), _(halt),        _(ld_vhl_K), _(ld_J_K),      _(ld_J_K),    _(ld_J_K),      _(ld_J_K),     _(ld_J_K),       _(ld_J_K),    _(ld_J_vhl),  _(ld_J_K),                                                  \
/* 8 */ _(U_a_K),        _(U_a_K),      _(U_a_K),       _(U_a_K),       _(U_a_K),        _(U_a_K),    _(U_a_vhl),     _(U_a_K),    _(U_a_K),       _(U_a_K),     _(U_a_K),       _(U_a_K),      _(U_a_K),        _(U_a_K),     _(U_a_vhl),   _(U_a_K),                                                   \
/* 9 */ _(U_a_K),        _(U_a_K),      _(U_a_K),       _(U_a_K),       _(U_a_K),        _(U_a_K),    _(U_a_vhl),     _(U_a_K),    _(U_a_K),       _(U_a_K),     _(U_a_K),       _(U_a_K),      _(U_a_K),        _(U_a_K),     _(U_a_vhl),   _(U_a_K),                                                   \
/* A */ _(U_a_K),        _(U_a_K),      _(U_a_K),       _(U_a_K),       _(U_a_K),        _(U_a_K),    _(U_a_vhl),     _(U_a_K),    _(U_a_K),       _(U_a_K),     _(U_a_K),       _(U_a_K),      _(U_a_K),        _(U_a_K),     _(U_a_vhl),   _(U_a_K),                                                   \
/* B */ _(U_a_K),        _(U_a_K),      _(U_a_K),       _(U_a_K),       _(U_a_K),        _(U_a_K),    _(U_a_vhl),     _(U_a_K),    _(U_a_K),       _(U_a_K),     _(U_a_K),       _(U_a_K),      _(U_a_K),        _(U_a_K),     _(U_a_vhl),   _(U_a_K),                                                   \
/* C */ _(ret_Z),        _(pop_TT),     _(jp_Z_WORD),   _(jp_WORD),     _(call_nz_WORD), _(push_TT),  _(U_a_BYTE),    _(rst_N),    _(ret_Z),       _(ret),       _(jp_Z_WORD),   cb_prefix,     _(call_z_WORD),  _(call_WORD), _(U_a_BYTE),  _(rst_N),                                                   \
/* D */ _(ret_Z),        _(pop_TT),     _(jp_Z_WORD),   _(out_vBYTE_a), _(call_nc_WORD), _(push_TT),  _(U_a_BYTE),    _(rst_N),    _(ret_Z),       _(exx),       _(jp_Z_WORD),   _(in_a_vBYTE), _(call_c_WORD),  xy_prefix,    _(U_a_BYTE),  _(rst_N),                                                   \
/* E */ _(ret_Z),        _(pop_TT),     _(jp_Z_WORD),   _(ex_vsp_hl),   _(call_po_WORD), _(push_TT),  _(U_a_BYTE),    _(rst_N),    _(ret_Z),       _(jp_hl),     _(jp_Z_WORD),   _(ex_de_hl),   _(call_pe_WORD), ed_prefix,    _(U_a_BYTE),  _(rst_N),                                                   \
/* F */ _(ret_Z),        _(pop_TT),     _(jp_Z_WORD),   _(di),          _(call_p_WORD),  _(push_TT),  _(U_a_BYTE),    _(rst_N),    _(ret_Z),       _(ld_sp_hl),  _(jp_Z_WORD),   _(ei),         _(call_m_WORD),  xy_prefix,    _(U_a_BYTE),  _(rst_N)},                                                  \
                                                                                                                                                                                                                                                                                                         \
/* Instructions with CBh prefix */                                                                                                                                                                                                                                                                       \
                                                                                                                                                                                                                                                                                                         \
/*      0             1             2             3             4             5             6               7             8             9             A             B             C             D             E               F */                                                                       \
/* 0 */{P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_vhl),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_vhl),     _(G_K),                                                                    \
/* 1 */ P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_vhl),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_vhl),     _(G_K),                                                                    \
/* 2 */ P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_vhl),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_vhl),     _(G_K),                                                                    \
/* 3 */ P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_vhl),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_K),     P _(G_vhl),     _(G_K),                                                                    \
/* 4 */ P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_vhl), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_vhl), _(bit_N_K),                                                                \
/* 5 */ P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_vhl), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_vhl), _(bit_N_K),                                                                \
/* 6 */ P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_vhl), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_vhl), _(bit_N_K),                                                                \
/* 7 */ P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_vhl), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_K), P _(bit_N_vhl), _(bit_N_K),                                                                \
/* 8 */ P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   _(M_N_K),                                                                  \
/* 9 */ P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   _(M_N_K),                                                                  \
/* A */ P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   _(M_N_K),                                                                  \
/* B */ P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   _(M_N_K),                                                                  \
/* C */ P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   _(M_N_K),                                                                  \
/* D */ P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   _(M_N_K),                                                                  \
/* E */ P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   _(M_N_K),                                                                  \
/* F */ P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_K),   P _(M_N_vhl),   _(M_N_K)},                                                                 \
                                                                                                                                                                                                                                                                                                         \
/* Instructions with DDh or FDh prefix */                                                                                                                                                                                                                                                                \
                                                                                                                                                                                                                                                                                                         \
/*      0                   1                   2                   3                   4                   5                   6                      7                   8               9              A               B                C                D             E                   F */       \
/* 0 */{_(nop),             _(ld_SS_WORD),      _(ld_vbc_a),        _(inc_SS),          _(V_O),             _(V_O),             _(ld_O_BYTE),          _(rlca),            _(ex_af_af_),   _(add_XY_WW),  _(ld_a_vbc),    _(dec_SS),       _(V_O),          _(V_O),       _(ld_O_BYTE),       _(rrca),   \
/* 1 */ _(djnz_OFFSET),     _(ld_SS_WORD),      _(ld_vde_a),        _(inc_SS),          _(V_O),             _(V_O),             _(ld_O_BYTE),          _(rla),             _(jr_OFFSET),   _(add_XY_WW),  _(ld_a_vde),    _(dec_SS),       _(V_O),          _(V_O),       _(ld_O_BYTE),       _(rra),    \
/* 2 */ _(jr_nz_OFFSET),    _(ld_XY_WORD),      _(ld_vWORD_XY),     _(inc_XY),          _(V_O),             _(V_O),             _(ld_O_BYTE),          _(daa),             _(jr_z_OFFSET), _(add_XY_WW),  _(ld_XY_vWORD), _(dec_XY),       _(V_O),          _(V_O),       _(ld_O_BYTE),       _(cpl),    \
/* 3 */ _(jr_nc_OFFSET),    _(ld_SS_WORD),      _(ld_vWORD_a),      _(inc_SS),          _(V_vXYpOFFSET),    _(V_vXYpOFFSET),    _(ld_vXYpOFFSET_BYTE), _(scf),             _(jr_c_OFFSET), _(add_XY_WW),  _(ld_a_vWORD),  _(dec_SS),       _(V_O),          _(V_O),       _(ld_O_BYTE),       _(ccf),    \
/* 4 */ _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_J_vXYpOFFSET),    _(ld_O_P),          _(ld_O_P),      _(ld_O_P),     _(ld_O_P),      _(ld_O_P),       _(ld_O_P),       _(ld_O_P),    _(ld_J_vXYpOFFSET), _(ld_O_P), \
/* 5 */ _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_J_vXYpOFFSET),    _(ld_O_P),          _(ld_O_P),      _(ld_O_P),     _(ld_O_P),      _(ld_O_P),       _(ld_O_P),       _(ld_O_P),    _(ld_J_vXYpOFFSET), _(ld_O_P), \
/* 6 */ _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_O_P),          _(ld_J_vXYpOFFSET),    _(ld_O_P),          _(ld_O_P),      _(ld_O_P),     _(ld_O_P),      _(ld_O_P),       _(ld_O_P),       _(ld_O_P),    _(ld_J_vXYpOFFSET), _(ld_O_P), \
/* 7 */ _(ld_vXYpOFFSET_K), _(ld_vXYpOFFSET_K), _(ld_vXYpOFFSET_K), _(ld_vXYpOFFSET_K), _(ld_vXYpOFFSET_K), _(ld_vXYpOFFSET_K), _(halt),               _(ld_vXYpOFFSET_K), _(ld_O_P),      _(ld_O_P),     _(ld_O_P),      _(ld_O_P),       _(ld_O_P),       _(ld_O_P),    _(ld_J_vXYpOFFSET), _(ld_O_P), \
/* 8 */ _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_vXYpOFFSET),     _(U_a_P),           _(U_a_P),       _(U_a_P),      _(U_a_P),       _(U_a_P),        _(U_a_P),        _(U_a_P),     _(U_a_vXYpOFFSET),  _(U_a_P),  \
/* 9 */ _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_vXYpOFFSET),     _(U_a_P),           _(U_a_P),       _(U_a_P),      _(U_a_P),       _(U_a_P),        _(U_a_P),        _(U_a_P),     _(U_a_vXYpOFFSET),  _(U_a_P),  \
/* A */ _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_vXYpOFFSET),     _(U_a_P),           _(U_a_P),       _(U_a_P),      _(U_a_P),       _(U_a_P),        _(U_a_P),        _(U_a_P),     _(U_a_vXYpOFFSET),  _(U_a_P),  \
/* B */ _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_P),           _(U_a_vXYpOFFSET),     _(U_a_P),           _(U_a_P),       _(U_a_P),      _(U_a_P),       _(U_a_P),        _(U_a_P),        _(U_a_P),     _(U_a_vXYpOFFSET),  _(U_a_P),  \
/* C */ _(ret_Z),           _(pop_TT),          _(jp_Z_WORD),       _(jp_WORD),         _(call_nz_WORD),    _(push_TT),         _(U_a_BYTE),           _(rst_N),           _(ret_Z),       _(ret),        _(jp_Z_WORD),   _(xy_cb_prefix), _(call_z_WORD),  _(call_WORD), _(U_a_BYTE),        _(rst_N),  \
/* D */ _(ret_Z),           _(pop_TT),          _(jp_Z_WORD),       _(out_vBYTE_a),     _(call_nc_WORD),    _(push_TT),         _(U_a_BYTE),           _(rst_N),           _(ret_Z),       _(exx),        _(jp_Z_WORD),   _(in_a_vBYTE),   _(call_c_WORD),  xy_prefix,    _(U_a_BYTE),        _(rst_N),  \
/* E */ _(ret_Z),           _(pop_XY),          _(jp_Z_WORD),       _(ex_vsp_XY),       _(call_po_WORD),    _(push_XY),         _(U_a_BYTE),           _(rst_N),           _(ret_Z),       _(jp_XY),      _(jp_Z_WORD),   _(ex_de_hl),     _(call_pe_WORD), ed_prefix,    _(U_a_BYTE),        _(rst_N),  \
/* F */ _(ret_Z),           _(pop_TT),          _(jp_Z_WORD),       _(di),              _(call_p_WORD),     _(push_TT),         _(U_a_BYTE),           _(rst_N),           _(ret_Z),       _(ld_sp_XY),   _(jp_Z_WORD),   _(ei),           _(call_m_WORD),  xy_prefix,    _(U_a_BYTE),        _(rst_N)}, \
                                                                                                                                                                                                                                                                                                         \
/* Instructions with EDh prefix */                                                                                                                                                                                                                                                                       \
                                                                                                                                                                                                                                                                                                         \
/*      0                1                2                3                 4                5                6                7                8                9                A                B                 C                D                E                F */                            \
/* 0 */{P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* 1 */ P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* 2 */ P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* 3 */ P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* 4 */ P _(in_J_vc),    P _(out_vc_J),   P _(sbc_hl_SS),  P _(ld_vWORD_SS), P _(neg),        P _(reti_retn),  P _(im_0),       P _(ld_i_a),     P _(in_J_vc),    P _(out_vc_J),   P _(adc_hl_SS),  P _(ld_SS_vWORD), P _(neg),        P _(reti_retn),  P _(im_0),       P _(ld_r_a),                    \
/* 5 */ P _(in_J_vc),    P _(out_vc_J),   P _(sbc_hl_SS),  P _(ld_vWORD_SS), P _(neg),        P _(reti_retn),  P _(im_1),       P _(ld_a_i),     P _(in_J_vc),    P _(out_vc_J),   P _(adc_hl_SS),  P _(ld_SS_vWORD), P _(neg),        P _(reti_retn),  P _(im_2),       P _(ld_a_r),                    \
/* 6 */ P _(in_J_vc),    P _(out_vc_J),   P _(sbc_hl_SS),  P _(ld_vWORD_SS), P _(neg),        P _(reti_retn),  P _(im_0),       P _(rrd),        P _(in_J_vc),    P _(out_vc_J),   P _(adc_hl_SS),  P _(ld_SS_vWORD), P _(neg),        P _(reti_retn),  P _(im_0),       P _(rld),                       \
/* 7 */ P _(in_vc),      P _(out_vc_0),   P _(sbc_hl_SS),  P _(ld_vWORD_SS), P _(neg),        P _(reti_retn),  P _(im_1),       P _(ed_illegal), P _(in_J_vc),    P _(out_vc_J),   P _(adc_hl_SS),  P _(ld_SS_vWORD), P _(neg),        P _(reti_retn),  P _(im_2),       P _(ed_illegal),                \
/* 8 */ P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* 9 */ P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* A */ P _(ldX),        P _(cpX),        P _(inX),        P _(outX),        P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ldX),        P _(cpX),        P _(inX),        P _(outX),        P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* B */ P _(ldXr),       P _(cpXr),       P _(inXr),       P _(otXr),        P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ldXr),       P _(cpXr),       P _(inXr),       P _(otXr),        P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* C */ P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* D */ P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* E */ P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),                \
/* F */ P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal),  P _(ed_illegal), P _(ed_illegal), P _(ed_illegal), P _(ed_illegal)}


#define _(patterns) PATTERN(patterns)

#define PATTERN(a, b) (0x##a << 4)
zuint32 const Z80Plus::no_mreq_table[4][256] = {PATTERN_TABLE};
#undef  PATTERN

#define PATTERN(a, b) (0x##b << 4)
zuint32 const Z80Plus::m_cycle_table[4][256] = {PATTERN_TABLE};

#undef halt
#undef nop
#undef ld_r_a
#undef ld_i_a


Z80Plus::Z80Plus() noexcept
:	direct_read        (nullptr),
	accumulated_cycles (0),
	instruction_cycles (0),
	pattern_table_index(0),
	pattern		   (0)
	{
	context      = nullptr;
	fetch_opcode = nullptr;
	fetch        = nullptr;
	read         = nullptr;
	write        = nullptr;
	in           = nullptr;
	out          = nullptr;
	halt         = nullptr;
	nop          = nullptr;
	nmia         = nullptr;
	inta         = nullptr;
	int_fetch    = nullptr;
	ld_i_a       = nullptr;
	ld_r_a       = nullptr;
	reti         = nullptr;
	retn         = nullptr;
	hook         = nullptr;
	options      = Z80_MODEL_ZILOG_NMOS;
	}


zuint8 Z80Plus::m_no_mreq(zuint8 opcode) noexcept
	{
	zuint32 p = no_mreq_table[pattern_table_index][opcode];

	instruction_cycles = p >> 28;

	if (p & 8) switch ((p >> 4) & 0xF)
		{
		case 0x0: /* djnz OFFSET  */ if (  b() == 1     ) p = djnz_OFFSET_false; break;
		case 0x1: /* jr nz,OFFSET */ if (  f() & Z80_ZF ) p = jr_Z_OFFSET_false; break;
		case 0x2: /* jr z,OFFSET  */ if (!(f() & Z80_ZF)) p = jr_Z_OFFSET_false; break;
		case 0x3: /* jr nc,OFFSET */ if (  f() & Z80_CF ) p = jr_Z_OFFSET_false; break;
		case 0x4: /* jr c,OFFSET  */ if (!(f() & Z80_CF)) p = jr_Z_OFFSET_false; break;
		case 0x5: /* call nz,WORD */ if (  f() & Z80_ZF ) p = call_Z_WORD_false; break;
		case 0x6: /* call z,WORD  */ if (!(f() & Z80_ZF)) p = call_Z_WORD_false; break;
		case 0x7: /* call nc,WORD */ if (  f() & Z80_CF ) p = call_Z_WORD_false; break;
		case 0x8: /* call c,WORD  */ if (!(f() & Z80_CF)) p = call_Z_WORD_false; break;
		case 0x9: /* call po,WORD */ if (  f() & Z80_PF ) p = call_Z_WORD_false; break;
		case 0xA: /* call pe,WORD */ if (!(f() & Z80_PF)) p = call_Z_WORD_false; break;
		case 0xB: /* call p,WORD  */ if (  f() & Z80_SF ) p = call_Z_WORD_false; break;
		case 0xC: /* call m,WORD  */ if (!(f() & Z80_SF)) p = call_Z_WORD_false; break;
		case 0xD: /* ldir / lddr  */ if ( bc() == 1     ) p = ldXr_false;        break;

		case 0xE: /* cpir / cpdr */
			if (bc() == 1 || a() == direct_read(context, hl())) p = cpXr_false;
			break;

		case 0xF: /* inir / indr / otir / otdr */
			if (b() == 1) p = inXr_otXr_false;
			break;
		}

	pattern_table_index = p & 3;
	return (pattern = p >> 8) & 0xF;
	}


/* Z80Plus.cpp EOF */
