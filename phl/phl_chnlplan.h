/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#ifndef _PHL_CHNLPLAN_H_
#define _PHL_CHNLPLAN_H_

#define REGULATION_CHPLAN_VERSION 55

enum REGULATION_DOMAIN_2G {
    RD2_WORLD       =  0,
    RD2_ETSI1       =  1,
    RD2_FCC1        =  2,
    RD2_MKK1        =  3,
    RD2_ETSI2       =  4,
    RD2_GLOBAL      =  5,
    RD2_MKK2        =  6,
    RD2_FCC2        =  7,
    RD2_NULL        =  8,
    RD2_IC1         =  9,
    RD2_WORLD1      = 10,
    RD2_KCC1        = 11,
    RD2_IC2         = 12,
    RD2_ACMA1       = 13,
};

enum REGULATION_DOMAIN_5G {
    RD5_NULL        =  0,
    RD5_ETSI1       =  1,
    RD5_ETSI2       =  2,
    RD5_ETSI3       =  3,
    RD5_FCC1        =  4,
    RD5_FCC2        =  5,
    RD5_FCC3        =  6,
    RD5_FCC4        =  7,
    RD5_FCC5        =  8,
    RD5_FCC6        =  9,
    RD5_FCC7        = 10,
    RD5_IC1         = 11,
    RD5_KCC1        = 12,
    RD5_MKK1        = 13,
    RD5_MKK2        = 14,
    RD5_MKK3        = 15,
    RD5_NCC1        = 16,
    RD5_NCC2        = 17,
    RD5_NCC3        = 18,
    RD5_ETSI4       = 19,
    RD5_ETSI5       = 20,
    RD5_FCC8        = 21,
    RD5_ETSI6       = 22,
    RD5_ETSI7       = 23,
    RD5_ETSI8       = 24,
    RD5_ETSI9       = 25,
    RD5_ETSI10      = 26,
    RD5_ETSI11      = 27,
    RD5_NCC4        = 28,
    RD5_ETSI12      = 29,
    RD5_FCC9        = 30,
    RD5_ETSI13      = 31,
    RD5_FCC10       = 32,
    RD5_MKK4        = 33,
    RD5_ETSI14      = 34,
    RD5_FCC11       = 35,
    RD5_ETSI15      = 36,
    RD5_MKK5        = 37,
    RD5_ETSI16      = 38,
    RD5_ETSI17      = 39,
    RD5_FCC12       = 40,
    RD5_FCC13       = 41,
    RD5_FCC14       = 42,
    RD5_FCC15       = 43,
    RD5_FCC16       = 44,
    RD5_ETSI18      = 45,
    RD5_ETSI19      = 46,
    RD5_FCC17       = 47,
    RD5_ETSI20      = 48,
    RD5_IC2         = 49,
    RD5_ETSI21      = 50,
    RD5_FCC18       = 51,
    RD5_WORLD       = 52,
    RD5_CHILE1      = 53,
    RD5_ACMA1       = 54,
    RD5_WORLD1      = 55,
    RD5_CHILE2      = 56,
    RD5_KCC2        = 57,
    RD5_KCC3        = 58,
    RD5_MKK6        = 59,
    RD5_MKK7        = 60,
    RD5_MKK8        = 61,
    RD5_MEX1        = 62,
    RD5_ETSI22      = 63,
    RD5_MKK9        = 64,
    RD5_FCC19       = 65,
    RD5_FCC20       = 66,
    RD5_FCC21       = 67,
    RD5_ETSI23      = 68,
    RD5_ETSI24      = 69,
    RD5_ETSI25      = 70,
    RD5_MKK10       = 71,
    RD5_ETSI26      = 72,
    RD5_MKK11       = 73,
    RD5_ETSI27      = 74,
    RD5_ETSI28      = 75,
    RD5_ACMA2       = 76,
    RD5_ETSI29      = 77,
};

enum REGULATION {
    REGULATION_WW        =  0,
    REGULATION_ETSI      =  1,
    REGULATION_FCC       =  2,
    REGULATION_MKK       =  3,
    REGULATION_NA        =  4,
    REGULATION_IC        =  5,
    REGULATION_KCC       =  6,
    REGULATION_ACMA      =  7,
    REGULATION_NCC       =  8,
    REGULATION_CHILE     =  9,
    REGULATION_MEX       = 10,
};

struct regulatory_domain_2ghz {
    u8 idx; /* regulatory domain index */
    u8 regulation;

    /* support channel list
     * support_ch[0]: bit(0~7) stands for ch(1~8)
     * support_ch[1]: bit(0~5) stands for ch (9~14)
     */
    u8 support_ch[2];

    /* passive ch list
     * passive[0]: bit(0~7) stands for ch(1~8)
     * passive[1]: bit(0~5) stands for ch (9~14)
     */
    u8 passive[2];
};

struct regulatory_domain_5ghz {
    u8 idx; /* regulatory domain index */
    u8 regulation;
    /*
     * band1 support channel list, passive and dfs
     * bit0 stands for ch36
     * bit1 stands for ch40
     * bit2 stands for ch44
     * bit3 stands for ch48
     */
    u8 support_ch_b1;
    u8 passive_b1;
    u8 dfs_b1;

    /*
     * band2 support channel list, passive and dfs
     * bit0 stands for ch52
     * bit1 stands for ch56
     * bit2 stands for ch60
     * bit3 stands for ch64
     */
    u8 support_ch_b2;
    u8 passive_b2;
    u8 dfs_b2;

    /*
     * band3 support channel list, passive and dfs
     * byte[0]:
     *    bit0 stands for ch100
     *     bit1 stands for ch104
     *     bit2 stands for ch108
     *     bit3 stands for ch112
     *     bit4 stands for ch116
     *     bit5 stands for ch120
     *     bit6 stands for ch124
     *     bit7 stands for ch128
     * byte[1]:
     *    bit0 stands for ch132
     *     bit1 stands for ch136
     *     bit2 stands for ch140
     *     bit3 stands for ch144
     */
    u8 support_ch_b3[2];
    u8 passive_b3[2];
    u8 dfs_b3[2];

    /*
     * band4 support channel list, passive and dfs
     * bit0 stands for ch149
     * bit1 stands for ch153
     * bit2 stands for ch157
     * bit3 stands for ch161
     * bit4 stands for ch165
     */
    u8 support_ch_b4;
    u8 passive_b4;
    u8 dfs_b4;
};

struct regulatory_domain_mapping {
    u8 domain_code;
    u8 idx2g;
    u8 idx5g;
};

#define MAX_RD_2GHZ 14
#define MAX_RD_5GHZ 78
#define MAX_RD_MAP_NUM 106



#endif /* _PHL_CHNLPLAN_H_ */
