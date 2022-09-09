/* Z80Clock v0.1
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

#include <Z80Clock.h>

                                      /* T-states                                                         */
                                      /* Total 123456                                                     */
                                      /* ------------  -------------------------------------------------- */
#define ld_J_vXYpOFFSET      0x05000 /*    19  44353   O(pc), O(pc+1), A(pc+2), %:5, R(EA)                */
#define ld_vXYpOFFSET_K      0x05000 /*    19  44353   O(pc), O(pc+1), A(pc+2), %:5, W(EA)                */
#define ld_vXYpOFFSET_BYTE  0x020000 /*    19  44353   O(pc), O(pc+1), A(pc+2), A(pc+3):2, W(EA)          */
#define ld_a_i                 0x100 /*     9  45      O(pc), O(pc+1):1                                   */
#define ld_a_r                 0x100 /*     9  45      O(pc), O(pc+1):1                                   */
#define ld_i_a                 0x100 /*     9  45      O(pc), O(pc+1):1                                   */
#define ld_r_a                 0x100 /*     9  45      O(pc), O(pc+1):1                                   */
#define ld_sp_hl               0x200 /*     6  6       O(pc):2                                            */
#define ld_sp_XY               0x200 /*    10  46      O(pc), O(pc+1):2                                   */
#define push_TT              0x00100 /*    11  533     O(pc):1, W(sp-1), W(sp-2)                          */
#define push_XY              0x00100 /*    15  4533    O(pc), O(pc+1):1, W(sp-1), W(sp-2)                 */
#define ex_vsp_hl          0x2010000 /*    19  43435   O(pc), R(sp), R(sp+1):1, W(sp+1), W(sp):2          */
#define ex_vsp_XY          0x2010000 /*    23  443435  O(pc), O(pc+1), R(sp), R(sp+1):1, W(sp+1), W(sp):2 */
#define ldi                  0x20000 /*    16  4435    O(pc), O(pc+1), R(hl), W(de):2                     */
#define ldir                 0x700D8 /*    21  44355   O(pc), O(pc+1), R(hl), W(de):2, %:5                */
#define ldd                  0x20000 /*    16  4435    O(pc), O(pc+1), R(hl), W(de):2                     */
#define lddr                 0x700D8 /*    21  44355   O(pc), O(pc+1), R(hl), W(de):2, %:5                */
#define cpi                   0x5000 /*    16  4435    O(pc), O(pc+1), R(hl), %:5                         */
#define cpir                  0xA0E8 /*    21  44355   O(pc), O(pc+1), R(hl), %:5, %:5                    */
#define cpd                   0x5000 /*    16  4435    O(pc), O(pc+1), R(hl), %:5                         */
#define cpdr                  0xA0E8 /*    21  44355   O(pc), O(pc+1), R(hl), %:5, %:5                    */
#define U_a_vXYpOFFSET       0x05000 /*    19  44353   O(pc), O(pc+1), A(pc+2), %:5, R(EA)                */
#define V_vhl                0x01000 /*    11  443     O(pc), R(hl):1, W(hl)                              */
#define V_vXYpOFFSET        0x015000 /*    23  443543  O(pc), O(pc+1), A(pc+2), %:5, R(EA):1 W(EA)        */
#define add_hl_SS              0x700 /*    11  443     O(pc), %:4, %:3                                    */
#define adc_hl_SS              0x700 /*    15  4443    O(pc), O(pc+1), %:4, %:3                           */
#define sbc_hl_SS              0x700 /*    15  4443    O(pc), O(pc+1), %:4, %:3                           */
#define add_XY_WW              0x700 /*    15  4443    O(pc), O(pc+1), %:4, %:3                           */
#define inc_SS                 0x200 /*     6  6       O(pc):2                                            */
#define inc_XY                 0x200 /*    10  46      O(pc), O(pc+1):2                                   */
#define dec_SS                 0x200 /*     6  6       O(pc):2                                            */
#define dec_XY                 0x200 /*    10  46      O(pc), O(pc+1):2                                   */
#define G_vhl                0x01000 /*    15  4443    O(pc), O(pc+1), R(hl):1, W(hl)                     */
#define rld                  0x04000 /*    18  44343   O(pc), O(pc+1), R(hl), %:4, W(hl)                  */
#define rrd                  0x04000 /*    18  44343   O(pc), O(pc+1), R(hl), %:4, W(hl)                  */
#define bit_N_vhl             0x1000 /*    12  444     O(pc), O(pc+1), R(hl):1                            */
#define M_N_vhl              0x01000 /*    15  4443    O(pc), O(pc+1), R(hl):1, W(hl)                     */
#define jr_OFFSET             0x5000 /*    12  435     O(pc), A(pc+1), %:5                                */
#define jr_nz_OFFSET          0x5018 /*    12  435     O(pc), A(pc+1), %:5                                */
#define jr_z_OFFSET           0x5028 /*    =   =        =                                                 */
#define jr_nc_OFFSET          0x5038 /*    =   =        =                                                 */
#define jr_c_OFFSET           0x5048 /*    =   =        =                                                 */
#define djnz_OFFSET           0x5108 /*    13  535     O(pc):1, A(pc+1), %:5                              */
#define call_WORD          0x0010000 /*    17  43433   O(pc), A(pc+1), A(pc+2):1, W(sp-1), W(sp-2)        */
#define call_nz_WORD       0x0010058 /*    17  43433   O(pc), A(pc+1), A(pc+2):1, W(sp-1), W(sp-2)        */
#define call_z_WORD        0x0010068 /*    =   =       =                                                  */
#define call_nc_WORD       0x0010078 /*    =   =       =                                                  */
#define call_c_WORD        0x0010088 /*    =   =       =                                                  */
#define call_po_WORD       0x0010098 /*    =   =       =                                                  */
#define call_pe_WORD       0x00100A8 /*    =   =       =                                                  */
#define call_p_WORD        0x00100B8 /*    =   =       =                                                  */
#define call_m_WORD        0x00100C8 /*    =   =       =                                                  */
#define ret_Z                0x00100 /*  11/5  533     O(pc):1, [R(sp), R(sp+1)]                          */
#define rst_N                0x00100 /*    11  533     O(pc):1, W(sp-1), W(sp-2)                          */
#define ini                  0x00100 /*    16  4543    O(pc), O(pc+1):1, IN(bc), W(hl)                    */
#define inir                 0x501F8 /*    21  45435   O(pc), O(pc+1):1, IN(bc), W(hl), %:5               */
#define ind                  0x00100 /*    16  4543    O(pc), O(pc+1):1, IN(bc), W(hl)                    */
#define indr                 0x501F8 /*    21  45435   O(pc), O(pc+1):1, IN(bc), W(hl), %:5               */
#define outi                 0x00100 /*    16  4534    O(pc), O(pc+1):1, R(hl), OUT(bc)                   */
#define otir                 0x501F8 /*    21  45345   O(pc), O(pc+1):1, R(hl), OUT(bc), %:5              */
#define outd                 0x00100 /*    16  4534    O(pc), O(pc+1):1, R(hl), OUT(bc)                   */
#define otdr                 0x501F8 /*    21  45345   O(pc), O(pc+1):1, R(hl), OUT(bc), %:5              */
#define cb_prefix                  1
#define ed_prefix                  3
#define xy_prefix                  2
#define xy_cb_prefix       0x0120000 /* 23/20  443543  O(pc), O(pc+1), A(pc+2), A(pc+3):2, R(EA):1, [W(EA)] */

#define P                 0x40000000 |

static zuint32 const pattern_table[4][256] = {

/* Unprefixed instructions */

/*      0             1  2  3          4             5        6  7      8            9          A  B          C             D          E  F */
/* 0 */{0,            0, 0, inc_SS,    0,            0,       0, 0,     0,           add_hl_SS, 0, dec_SS,    0,            0,         0, 0,
/* 1 */ djnz_OFFSET,  0, 0, inc_SS,    0,            0,       0, 0,     jr_OFFSET,   add_hl_SS, 0, dec_SS,    0,            0,         0, 0,
/* 2 */ jr_nz_OFFSET, 0, 0, inc_SS,    0,            0,       0, 0,     jr_z_OFFSET, add_hl_SS, 0, dec_SS,    0,            0,         0, 0,
/* 3 */ jr_nc_OFFSET, 0, 0, inc_SS,    V_vhl,        V_vhl,   0, 0,     jr_c_OFFSET, add_hl_SS, 0, dec_SS,    0,            0,         0, 0,
/* 4 */ 0,            0, 0, 0,         0,            0,       0, 0,     0,           0,         0, 0,         0,            0,         0, 0,
/* 5 */ 0,            0, 0, 0,         0,            0,       0, 0,     0,           0,         0, 0,         0,            0,         0, 0,
/* 6 */ 0,            0, 0, 0,         0,            0,       0, 0,     0,           0,         0, 0,         0,            0,         0, 0,
/* 7 */ 0,            0, 0, 0,         0,            0,       0, 0,     0,           0,         0, 0,         0,            0,         0, 0,
/* 8 */ 0,            0, 0, 0,         0,            0,       0, 0,     0,           0,         0, 0,         0,            0,         0, 0,
/* 9 */ 0,            0, 0, 0,         0,            0,       0, 0,     0,           0,         0, 0,         0,            0,         0, 0,
/* A */ 0,            0, 0, 0,         0,            0,       0, 0,     0,           0,         0, 0,         0,            0,         0, 0,
/* B */ 0,            0, 0, 0,         0,            0,       0, 0,     0,           0,         0, 0,         0,            0,         0, 0,
/* C */ ret_Z,        0, 0, 0,         call_nz_WORD, push_TT, 0, rst_N, ret_Z,       0,         0, cb_prefix, call_z_WORD,  call_WORD, 0, rst_N,
/* D */ ret_Z,        0, 0, 0,         call_nc_WORD, push_TT, 0, rst_N, ret_Z,       0,         0, 0,         call_c_WORD,  xy_prefix, 0, rst_N,
/* E */ ret_Z,        0, 0, ex_vsp_hl, call_po_WORD, push_TT, 0, rst_N, ret_Z,       0,         0, 0,         call_pe_WORD, ed_prefix, 0, rst_N,
/* F */ ret_Z,        0, 0, 0,         call_p_WORD,  push_TT, 0, rst_N, ret_Z,       ld_sp_hl,  0, 0,         call_m_WORD,  xy_prefix, 0, rst_N},

/* Instructions with CBh prefix */

/*      0    1    2    3    4    5    6            7    8    9    A    B    C    D    E            F */
/* 0 */{P 0, P 0, P 0, P 0, P 0, P 0, P G_vhl,     P 0, P 0, P 0, P 0, P 0, P 0, P 0, P G_vhl,     P 0,
/* 1 */ P 0, P 0, P 0, P 0, P 0, P 0, P G_vhl,     P 0, P 0, P 0, P 0, P 0, P 0, P 0, P G_vhl,     P 0,
/* 2 */ P 0, P 0, P 0, P 0, P 0, P 0, P G_vhl,     P 0, P 0, P 0, P 0, P 0, P 0, P 0, P G_vhl,     P 0,
/* 3 */ P 0, P 0, P 0, P 0, P 0, P 0, P G_vhl,     P 0, P 0, P 0, P 0, P 0, P 0, P 0, P G_vhl,     P 0,
/* 4 */ P 0, P 0, P 0, P 0, P 0, P 0, P bit_N_vhl, P 0, P 0, P 0, P 0, P 0, P 0, P 0, P bit_N_vhl, P 0,
/* 5 */ P 0, P 0, P 0, P 0, P 0, P 0, P bit_N_vhl, P 0, P 0, P 0, P 0, P 0, P 0, P 0, P bit_N_vhl, P 0,
/* 6 */ P 0, P 0, P 0, P 0, P 0, P 0, P bit_N_vhl, P 0, P 0, P 0, P 0, P 0, P 0, P 0, P bit_N_vhl, P 0,
/* 7 */ P 0, P 0, P 0, P 0, P 0, P 0, P bit_N_vhl, P 0, P 0, P 0, P 0, P 0, P 0, P 0, P bit_N_vhl, P 0,
/* 8 */ P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0, P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0,
/* 9 */ P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0, P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0,
/* A */ P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0, P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0,
/* B */ P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0, P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0,
/* C */ P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0, P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0,
/* D */ P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0, P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0,
/* E */ P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0, P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0,
/* F */ P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0, P 0, P 0, P 0, P 0, P 0, P 0, P M_N_vhl,   P 0},

/* Instructions with DDh or FDh prefix */

/*      0                1                2              3                  4                5                6                   7                8            9          A  B             C             D          E                F */
/* 0 */{0,               0,               0,             inc_SS,            0,               0,               0,                  0,               0,           add_XY_WW, 0, dec_SS,       0,            0,         0,               0,
/* 1 */ djnz_OFFSET,     0,               0,             inc_SS,            0,               0,               0,                  0,               jr_OFFSET,   add_XY_WW, 0, dec_SS,       0,            0,         0,               0,
/* 2 */ jr_nz_OFFSET,    0,               0,             inc_XY,            0,               0,               0,                  0,               jr_z_OFFSET, add_XY_WW, 0, dec_XY,       0,            0,         0,               0,
/* 3 */ jr_nc_OFFSET,    0,               0,             inc_SS,            V_vXYpOFFSET,    V_vXYpOFFSET,    ld_vXYpOFFSET_BYTE, 0,               jr_c_OFFSET, add_XY_WW, 0, dec_SS,       0,            0,         0,               0,
/* 4 */ 0,               0,               0,             0,                 0,               0,               ld_J_vXYpOFFSET,    0,               0,           0,         0, 0,            0,            0,         ld_J_vXYpOFFSET, 0,
/* 5 */ 0,               0,               0,             0,                 0,               0,               ld_J_vXYpOFFSET,    0,               0,           0,         0, 0,            0,            0,         ld_J_vXYpOFFSET, 0,
/* 6 */ 0,               0,               0,             0,                 0,               0,               ld_J_vXYpOFFSET,    0,               0,           0,         0, 0,            0,            0,         ld_J_vXYpOFFSET, 0,
/* 7 */ ld_vXYpOFFSET_K, ld_vXYpOFFSET_K, ld_vXYpOFFSET_K, ld_vXYpOFFSET_K, ld_vXYpOFFSET_K, ld_vXYpOFFSET_K, 0,                  ld_vXYpOFFSET_K, 0,           0,         0, 0,            0,            0,         ld_J_vXYpOFFSET, 0,
/* 8 */ 0,               0,               0,             0,                 0,               0,               U_a_vXYpOFFSET,     0,               0,           0,         0, 0,            0,            0,         U_a_vXYpOFFSET,  0,
/* 9 */ 0,               0,               0,             0,                 0,               0,               U_a_vXYpOFFSET,     0,               0,           0,         0, 0,            0,            0,         U_a_vXYpOFFSET,  0,
/* A */ 0,               0,               0,             0,                 0,               0,               U_a_vXYpOFFSET,     0,               0,           0,         0, 0,            0,            0,         U_a_vXYpOFFSET,  0,
/* B */ 0,               0,               0,             0,                 0,               0,               U_a_vXYpOFFSET,     0,               0,           0,         0, 0,            0,            0,         U_a_vXYpOFFSET,  0,
/* C */ ret_Z,           0,               0,             0,                 call_nz_WORD,    push_TT,         0,                  rst_N,           ret_Z,       0,         0, xy_cb_prefix, call_z_WORD,  call_WORD, 0,               rst_N,
/* D */ ret_Z,           0,               0,             0,                 call_nc_WORD,    push_TT,         0,                  rst_N,           ret_Z,       0,         0, 0,            call_c_WORD,  xy_prefix, 0,               rst_N,
/* E */ ret_Z,           0,               0,             ex_vsp_XY,         call_po_WORD,    push_XY,         0,                  rst_N,           ret_Z,       0,         0, 0,            call_pe_WORD, ed_prefix, 0,               rst_N,
/* F */ ret_Z,           0,               0,             0,                 call_p_WORD,     push_TT,         0,                  rst_N,           ret_Z,       ld_sp_XY,  0, 0,            call_m_WORD,  xy_prefix, 0,               rst_N},

/* Instructions with EDh prefix */

/*      0       1       2            3       4    5    6    7         8       9       A            B       C    D    E    F */
/* 0 */{P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,
/* 1 */ P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,
/* 2 */ P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,
/* 3 */ P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,
/* 4 */ P 0,    P 0,    P sbc_hl_SS, P 0,    P 0, P 0, P 0, P ld_i_a, P 0,    P 0,    P adc_hl_SS, P 0,    P 0, P 0, P 0, P ld_r_a,
/* 5 */ P 0,    P 0,    P sbc_hl_SS, P 0,    P 0, P 0, P 0, P ld_a_i, P 0,    P 0,    P adc_hl_SS, P 0,    P 0, P 0, P 0, P ld_a_r,
/* 6 */ P 0,    P 0,    P sbc_hl_SS, P 0,    P 0, P 0, P 0, P rrd,    P 0,    P 0,    P adc_hl_SS, P 0,    P 0, P 0, P 0, P rld,
/* 7 */ P 0,    P 0,    P sbc_hl_SS, P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P adc_hl_SS, P 0,    P 0, P 0, P 0, P 0,
/* 8 */ P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,
/* 9 */ P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,
/* A */ P ldi,  P cpi,  P ini,       P outi, P 0, P 0, P 0, P 0,      P ldd,  P cpd,  P ind,       P outd, P 0, P 0, P 0, P 0,
/* B */ P ldir, P cpir, P inir,      P otir, P 0, P 0, P 0, P 0,      P lddr, P cpdr, P indr,      P otdr, P 0, P 0, P 0, P 0,
/* C */ P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,
/* D */ P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,
/* E */ P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,
/* F */ P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0,      P 0,    P 0,    P 0,         P 0,    P 0, P 0, P 0, P 0}
};

                                          /* T-states                                    */
                                          /*     12345                                   */
                                          /* ---------  -------------------------------- */
#define ldir_lddr_false           0x20000 /* 16  4435   O(pc), O(pc+1), R(hl), W(de):2   */
#define cpir_cpdr_false            0x5000 /* 16  4435   O(pc), O(pc+1), R(hl), %:5       */
#define jr_Z_OFFSET_false          0x0000 /*  7  43     O(pc), A(pc+1)                   */
#define djnz_OFFSET_false          0x0100 /*  8  53     O(pc):1, A(pc+1)                 */
#define call_Z_WORD_false         0x00000 /* 10  433    O(pc), A(pc+1), A(pc+2)          */
#define inir_indr_otir_otdr_false 0x00100 /* 16  4543   O(pc), O(pc+1):1, IN(bc), W(hl)  */
                                          /* 16  4534   O(pc), O(pc+1):1, R(hl), OUT(bc) */


Z80_CLOCK_API zuint8 z80_clock_m1(Z80Clock *self, zuint8 opcode)
	{
	zuint32 pattern = pattern_table[self->pattern_table_index][opcode];

	self->cycles = (pattern >> 28);

	if (pattern & 8)
		{
		Z80 *cpu = self->cpu;

		switch ((pattern >> 4) & 0xF)
			{
			case 0x0: /* djnz OFFSET  */ if (  Z80_B(*cpu) == 1     ) pattern = djnz_OFFSET_false; break;
			case 0x1: /* jr nz,OFFSET */ if (  Z80_F(*cpu) & Z80_ZF ) pattern = jr_Z_OFFSET_false; break;
			case 0x2: /* jr z,OFFSET  */ if (!(Z80_F(*cpu) & Z80_ZF)) pattern = jr_Z_OFFSET_false; break;
			case 0x3: /* jr nc,OFFSET */ if (  Z80_F(*cpu) & Z80_CF ) pattern = jr_Z_OFFSET_false; break;
			case 0x4: /* jr c,OFFSET  */ if (!(Z80_F(*cpu) & Z80_CF)) pattern = jr_Z_OFFSET_false; break;
			case 0x5: /* call nz,WORD */ if (  Z80_F(*cpu) & Z80_ZF ) pattern = call_Z_WORD_false; break;
			case 0x6: /* call z,WORD  */ if (!(Z80_F(*cpu) & Z80_ZF)) pattern = call_Z_WORD_false; break;
			case 0x7: /* call nc,WORD */ if (  Z80_F(*cpu) & Z80_CF ) pattern = call_Z_WORD_false; break;
			case 0x8: /* call c,WORD  */ if (!(Z80_F(*cpu) & Z80_CF)) pattern = call_Z_WORD_false; break;
			case 0x9: /* call po,WORD */ if (  Z80_F(*cpu) & Z80_PF ) pattern = call_Z_WORD_false; break;
			case 0xA: /* call pe,WORD */ if (!(Z80_F(*cpu) & Z80_PF)) pattern = call_Z_WORD_false; break;
			case 0xB: /* call p,WORD  */ if (  Z80_F(*cpu) & Z80_SF ) pattern = call_Z_WORD_false; break;
			case 0xC: /* call m,WORD  */ if (!(Z80_F(*cpu) & Z80_SF)) pattern = call_Z_WORD_false; break;
			case 0xD: /* ldir / lddr  */ if ( Z80_BC(*cpu) == 1)      pattern = ldir_lddr_false;   break;

			case 0xE: /* cpir / cpdr */
			if (Z80_BC(*cpu) == 1 || Z80_A(*cpu) == self->read(self->read_context, Z80_HL(*cpu)))
				pattern = cpir_cpdr_false;
			break;

			case 0xF: /* inir / indr / otir / otdr  */
			if (Z80_B(*cpu) == 1)
				pattern = inir_indr_otir_otdr_false;
			break;

			default:
			break;
			}
		}

	self->pattern_table_index = pattern & 3;
	return (self->pattern = pattern >> 8) & 0xF;
	}


/* Z80Clock.c EOF */
