/**
 * @file font8x8_readable.h
 *
 * @brief a clean simple font to use with an ssd1306 screen
 */

#ifndef MAIN_FONT8X8_SPACE_H
#define MAIN_FONT8X8_SPACE_H

// Font: READABLE.pf

#ifdef CONFIG_SSID_FONT_SPACE
static uint8_t font8x8[128][8] =
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Char 000 (.)
        {0xFC, 0x02, 0x2B, 0x62, 0x62, 0x2A, 0x02, 0xFC}, // Char 001 (.)
        {0xFC, 0xFF, 0xD7, 0x9F, 0x9F, 0xD7, 0xFF, 0xFC}, // Char 002 (.)
        {0x1C, 0x3F, 0x7F, 0xFD, 0x7F, 0x3F, 0x1D, 0x00}, // Char 003 (.)
        {0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00}, // Char 004 (.)
        {0x30, 0x74, 0xFE, 0xFE, 0xFE, 0x74, 0x30, 0x00}, // Char 005 (.)
        {0x20, 0x71, 0xF9, 0xFF, 0xF9, 0x71, 0x20, 0x00}, // Char 006 (.)
        {0x00, 0x01, 0x31, 0x79, 0x79, 0x31, 0x00, 0x00}, // Char 007 (.)
        {0xFE, 0xFE, 0xCE, 0x86, 0x86, 0xCE, 0xFE, 0xFE}, // Char 008 (.)
        {0x01, 0x79, 0xCD, 0x85, 0x85, 0xCD, 0x79, 0x01}, // Char 009 (.)
        {0xFE, 0x86, 0x32, 0x7A, 0x7A, 0x32, 0x86, 0xFE}, // Char 010 (.)
        {0xE1, 0xF1, 0x11, 0x11, 0xFB, 0xFF, 0x0F, 0x1F}, // Char 011 (.)
        {0x00, 0x9D, 0xBF, 0xE3, 0xE3, 0xBE, 0x9C, 0x00}, // Char 012 (.)
        {0x80, 0xC0, 0xFE, 0xFF, 0x0B, 0x0A, 0x0E, 0x0E}, // Char 013 (.)
        {0x81, 0xFF, 0xFF, 0x0A, 0x0A, 0xCA, 0xFE, 0x7E}, // Char 014 (.)
        {0x33, 0xB5, 0x78, 0xCE, 0xCE, 0x78, 0xB4, 0x32}, // Char 015 (.)
        {0xFF, 0x7C, 0x7C, 0x39, 0x39, 0x10, 0x10, 0x01}, // Char 016 (.)
        {0x10, 0x10, 0x38, 0x38, 0x7C, 0x7C, 0xFE, 0x00}, // Char 017 (.)
        {0x00, 0x48, 0xCC, 0xFE, 0xFE, 0xCC, 0x48, 0x00}, // Char 018 (.)
        {0x00, 0xBE, 0xBE, 0x01, 0x01, 0xBE, 0xBE, 0x00}, // Char 019 (.)
        {0x0C, 0x1E, 0x12, 0xFE, 0xFE, 0x02, 0xFE, 0xFE}, // Char 020 (.)
        {0x80, 0xB4, 0x7E, 0x4A, 0xFA, 0xB2, 0x06, 0x04}, // Char 021 (.)
        {0x00, 0xE1, 0xE1, 0xE1, 0xE1, 0xE0, 0xE0, 0x00}, // Char 022 (.)
        {0x00, 0x28, 0x6C, 0xFE, 0xFE, 0x6C, 0x28, 0x00}, // Char 023 (.)
        {0x01, 0x09, 0x0D, 0xFF, 0xFF, 0x0D, 0x09, 0x01}, // Char 024 (.)
        {0x00, 0x20, 0x60, 0xFE, 0xFE, 0x60, 0x20, 0x00}, // Char 025 (.)
        {0x10, 0x10, 0x10, 0x54, 0x7C, 0x38, 0x10, 0x00}, // Char 026 (.)
        {0x10, 0x38, 0x7C, 0x54, 0x10, 0x10, 0x10, 0x00}, // Char 027 (.)
        {0x78, 0x78, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00}, // Char 028 (.)
        {0x10, 0x38, 0x7C, 0x10, 0x10, 0x7C, 0x38, 0x10}, // Char 029 (.)
        {0x60, 0x70, 0x78, 0x7C, 0x7C, 0x78, 0x70, 0x60}, // Char 030 (.)
        {0x0C, 0x1C, 0x3C, 0x7C, 0x7C, 0x3C, 0x1C, 0x0C}, // Char 031 (.)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Char 032 ( )
        {0x00, 0x0C, 0xBE, 0xBE, 0x0C, 0x00, 0x00, 0x00}, // Char 033 (!)
        {0x00, 0x0E, 0x0E, 0x00, 0x0E, 0x0E, 0x00, 0x00}, // Char 034 (")
        {0x28, 0xFE, 0xFE, 0x28, 0xFE, 0xFE, 0x28, 0x00}, // Char 035 (#)
        {0x48, 0x5C, 0xD6, 0xD6, 0x74, 0x24, 0x00, 0x00}, // Char 036 ($)
        {0x8C, 0xCC, 0x60, 0x30, 0x18, 0xCC, 0xC4, 0x00}, // Char 037 (%)
        {0x60, 0xF4, 0x9E, 0xBA, 0x6E, 0xF4, 0x90, 0x00}, // Char 038 (&)
        {0x08, 0x0E, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00}, // Char 039 (')
        {0x00, 0x38, 0x7C, 0xC6, 0x82, 0x00, 0x00, 0x00}, // Char 040 (()
        {0x00, 0x82, 0xC6, 0x7C, 0x38, 0x00, 0x00, 0x00}, // Char 041 ())
        {0x10, 0x54, 0x7C, 0x38, 0x38, 0x7C, 0x54, 0x10}, // Char 042 (*)
        {0x10, 0x10, 0x7C, 0x7C, 0x10, 0x10, 0x00, 0x00}, // Char 043 (+)
        {0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00}, // Char 044 (,)
        {0x10, 0x11, 0x11, 0x10, 0x10, 0x10, 0x00, 0x00}, // Char 045 (-)
        {0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00}, // Char 046 (.)
        {0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00}, // Char 047 (/)
        {0xFE, 0xF2, 0xA2, 0x92, 0x8A, 0x8E, 0xFE, 0x00}, // Char 048 (0)
        {0x00, 0x00, 0xF0, 0xFE, 0x00, 0x00, 0x00, 0x00}, // Char 049 (1)
        {0x00, 0xC4, 0xE2, 0xA2, 0x92, 0x9E, 0x8C, 0x00}, // Char 050 (2)
        {0x00, 0xC4, 0xC2, 0x92, 0x92, 0x9E, 0x6C, 0x00}, // Char 051 (3)
        {0x00, 0x3E, 0x20, 0x20, 0xE6, 0xFE, 0x00, 0x00}, // Char 052 (4)
        {0x00, 0xCE, 0xCA, 0x8A, 0x8A, 0x8A, 0x72, 0x00}, // Char 053 (5)
        {0x00, 0x7C, 0xF6, 0x92, 0x92, 0x96, 0x64, 0x00}, // Char 054 (6)
        {0x00, 0x02, 0x02, 0x02, 0xE6, 0xFE, 0x00, 0x00}, // Char 055 (7)
        {0x00, 0x6C, 0xF2, 0x92, 0x92, 0x9E, 0x6C, 0x00}, // Char 056 (8)
        {0x00, 0x4C, 0xD2, 0x92, 0x92, 0x9E, 0x7C, 0x00}, // Char 057 (9)
        {0x00, 0x00, 0xCC, 0xCC, 0x00, 0x00, 0x00, 0x00}, // Char 058 (:)
        {0x00, 0x00, 0xCC, 0xCC, 0x00, 0x00, 0x00, 0x00}, // Char 059 (;)
        {0x10, 0x39, 0x6D, 0xC6, 0x82, 0x00, 0x00, 0x00}, // Char 060 (<)
        {0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x00, 0x00}, // Char 061 (=)
        {0x00, 0x82, 0xC6, 0x6C, 0x38, 0x10, 0x00, 0x00}, // Char 062 (>)
        {0x04, 0x06, 0xA2, 0xB2, 0x1E, 0x0C, 0x00, 0x00}, // Char 063 (?)
        {0x7C, 0xFE, 0x82, 0xBA, 0xBA, 0x3E, 0x3C, 0x00}, // Char 064 (@)
        {0x00, 0xFE, 0xE2, 0x22, 0x22, 0x26, 0xFE, 0x00}, // Char 065 (A)
        {0x00, 0xFE, 0xF2, 0x92, 0x96, 0x9E, 0xF0, 0x00}, // Char 066 (B)
        {0x00, 0xFE, 0xF2, 0x82, 0x82, 0x86, 0xC6, 0x00}, // Char 067 (C)
        {0x00, 0xFE, 0xE2, 0x82, 0x82, 0x8E, 0x7C, 0x00}, // Char 068 (D)
        {0x00, 0xFE, 0xF2, 0x92, 0x92, 0x86, 0xC6, 0x00}, // Char 069 (E)
        {0x00, 0xFE, 0xF2, 0x12, 0x12, 0x06, 0x06, 0x00}, // Char 070 (F)
        {0x00, 0xFE, 0xE2, 0x82, 0x92, 0x96, 0xF6, 0x00}, // Char 071 (G)
        {0x00, 0xFE, 0xF0, 0x10, 0x16, 0xFE, 0x00, 0x00}, // Char 072 (H)
        {0x00, 0x00, 0x00, 0xFE, 0xF0, 0x00, 0x00, 0x00}, // Char 073 (I)
        {0x00, 0xE0, 0xE0, 0x80, 0x80, 0x86, 0xFE, 0x00}, // Char 074 (J)
        {0x00, 0xFE, 0xF0, 0x10, 0x18, 0x2E, 0xC6, 0x00}, // Char 075 (K)
        {0x00, 0xFE, 0xF0, 0x80, 0x80, 0x80, 0x80, 0x00}, // Char 076 (L)
        {0xFE, 0xE6, 0x0C, 0x38, 0x0C, 0x0E, 0xFE, 0x00}, // Char 077 (M)
        {0x00, 0xFE, 0xE8, 0x10, 0x20, 0x46, 0xFE, 0x00}, // Char 078 (N)
        {0x00, 0xFE, 0xF2, 0x82, 0x82, 0x8E, 0xFE, 0x00}, // Char 079 (O)
        {0x00, 0xFE, 0xF2, 0x12, 0x12, 0x1E, 0x1E, 0x00}, // Char 080 (P)
        {0x00, 0x7E, 0x72, 0xC2, 0xC2, 0xCE, 0x7E, 0x00}, // Char 081 (Q)
        {0x00, 0xFE, 0xF2, 0x12, 0x32, 0x5F, 0x8E, 0x00}, // Char 082 (R)
        {0x00, 0xCE, 0xCA, 0x92, 0x92, 0xA6, 0xE6, 0x00}, // Char 083 (S)
        {0x00, 0x02, 0x02, 0xFE, 0xF2, 0x06, 0x06, 0x00}, // Char 084 (T)
        {0x00, 0xFE, 0xF0, 0x80, 0x80, 0x86, 0xFE, 0x00}, // Char 085 (U)
        {0x00, 0x3E, 0x60, 0xC0, 0x40, 0x2E, 0x1E, 0x00}, // Char 086 (V)
        {0xFE, 0xF0, 0x60, 0x30, 0x60, 0xC6, 0xFE, 0x00}, // Char 087 (W)
        {0xC6, 0xE8, 0x70, 0x10, 0x28, 0x4E, 0x86, 0x00}, // Char 088 (X)
        {0x00, 0x0E, 0x10, 0xF0, 0xF0, 0x16, 0x0E, 0x00}, // Char 089 (Y)
        {0xC2, 0xC2, 0xA2, 0x92, 0x8A, 0x86, 0x86, 0x00}, // Char 090 (Z)
        {0x00, 0xFE, 0xFE, 0x82, 0x82, 0x00, 0x00, 0x00}, // Char 091 ([)
        {0x02, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x00}, // Char 092 (\)
        {0x00, 0x82, 0x82, 0xFE, 0xFE, 0x00, 0x00, 0x00}, // Char 093 (])
        {0x10, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x10, 0x00}, // Char 094 (^)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Char 095 (_)
        {0x01, 0x01, 0x07, 0x0F, 0x09, 0x01, 0x01, 0x01}, // Char 096 (`)
        {0x00, 0xE8, 0xE8, 0xA8, 0xA8, 0xB8, 0xF8, 0x00}, // Char 097 (a)
        {0x00, 0xFE, 0xF0, 0x90, 0x90, 0x90, 0xF0, 0x00}, // Char 098 (b)
        {0x00, 0xF8, 0xE8, 0x88, 0x88, 0x98, 0x98, 0x00}, // Char 099 (c)
        {0x00, 0xF0, 0xF0, 0x90, 0x90, 0x96, 0xFE, 0x00}, // Char 100 (d)
        {0x00, 0xF8, 0xE8, 0xA8, 0xA8, 0xB8, 0xB8, 0x00}, // Char 101 (e)
        {0x00, 0xF0, 0xF8, 0x14, 0x16, 0x06, 0x00, 0x00}, // Char 102 (f)
        {0x00, 0x78, 0x48, 0x48, 0x48, 0xD8, 0xF8, 0x00}, // Char 103 (g)
        {0x00, 0xFF, 0xF1, 0x11, 0x11, 0x10, 0xF0, 0x00}, // Char 104 (h)
        {0x00, 0x00, 0x00, 0xFA, 0xE0, 0x00, 0x00, 0x00}, // Char 105 (i)
        {0x00, 0xC0, 0xC0, 0x00, 0x0A, 0xFA, 0x00, 0x00}, // Char 106 (j)
        {0x00, 0xFF, 0xE1, 0x21, 0x31, 0x59, 0x88, 0x00}, // Char 107 (k)
        {0x00, 0x00, 0x00, 0xFE, 0xE0, 0x00, 0x00, 0x00}, // Char 108 (l)
        {0xF8, 0xC8, 0x08, 0xF8, 0x08, 0x38, 0xF8, 0x00}, // Char 109 (m)
        {0x00, 0xF8, 0xE8, 0x08, 0x08, 0x18, 0xF8, 0x00}, // Char 110 (n)
        {0x00, 0xF8, 0xE8, 0x88, 0x88, 0x98, 0xF8, 0x00}, // Char 111 (o)
        {0x00, 0xF8, 0xC8, 0x48, 0x48, 0x58, 0x78, 0x00}, // Char 112 (p)
        {0x00, 0x79, 0x49, 0x48, 0x48, 0x58, 0xF8, 0x00}, // Char 113 (q)
        {0x00, 0xF8, 0xE8, 0x08, 0x08, 0x18, 0x19, 0x00}, // Char 114 (r)
        {0x00, 0xB8, 0xA8, 0xA8, 0xA8, 0xE8, 0x00, 0x00}, // Char 115 (s)
        {0x00, 0x00, 0x08, 0xFE, 0xC8, 0x88, 0x00, 0x00}, // Char 116 (t)
        {0x00, 0xF8, 0xE0, 0x80, 0x80, 0x98, 0xF8, 0x00}, // Char 117 (u)
        {0x00, 0x38, 0x60, 0xC0, 0x80, 0x78, 0x38, 0x00}, // Char 118 (v)
        {0x78, 0xE0, 0xC0, 0x70, 0xC0, 0xD8, 0x78, 0x00}, // Char 119 (w)
        {0x00, 0x88, 0xD0, 0x60, 0x30, 0x58, 0x88, 0x00}, // Char 120 (x)
        {0x00, 0x78, 0x60, 0x40, 0x40, 0x58, 0xF8, 0x00}, // Char 121 (y)
        {0x00, 0x89, 0xC9, 0xE9, 0xB9, 0x99, 0x88, 0x00}, // Char 122 (z)
        {0x10, 0x10, 0x7C, 0xEE, 0x82, 0x82, 0x00, 0x00}, // Char 123 ({)
        {0x00, 0x00, 0x00, 0xEE, 0xEE, 0x00, 0x00, 0x00}, // Char 124 (|)
        {0x82, 0x82, 0xEE, 0x7C, 0x10, 0x10, 0x00, 0x00}, // Char 125 (})
        {0x04, 0x06, 0x02, 0x06, 0x04, 0x06, 0x02, 0x00}, // Char 126 (~)
        {0xE0, 0xF0, 0x98, 0x8C, 0x98, 0xF0, 0xE0, 0x00}, // Char 127 (.)
};
#endif // CONFIG_SSID_FONT_SPACE

#endif /* MAIN_FONT8X8_SPACE_H */