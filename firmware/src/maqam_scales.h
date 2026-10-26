/* SPDX-License-Identifier: GPL-3.0-only
 * 12-key reference tunings; factual source/provenance: assets/maqam/scales.json.
 * First two entries preserve the existing RAST/BAYATI IDs. */
#define MAQAM_EXISTING(X) \
 X(RAST, "RAST", 0) \
 X(BAYATI, "BAYATI", 1)
#define MAQAM_ADDED(X) \
 X(AJAM, "AJAM", 2) \
 X(AJAM_USHAYRAN, "AJAM USHAYRAN", 2) \
 X(ATHAR_KURD, "ATHAR KURD", 2) \
 X(AWJ_IRAQ, "AWJ IRAQ", 3) \
 X(BASTANIKAR, "BASTANIKAR", 4) \
 X(BAYATI_SHURI, "BAYATI SHURI", 1) \
 X(DALANSHIN, "DALANSHIN", 0) \
 X(FARAHFAZA, "FARAHFAZA", 2) \
 X(HIJAZ, "HIJAZ", 2) \
 X(HIJAZKAR, "HIJAZKAR", 2) \
 X(HIJAZKAR_KURD, "HIJAZKAR KURD", 2) \
 X(HUSAYNI, "HUSAYNI", 5) \
 X(HUZAM, "HUZAM", 6) \
 X(IRAQ, "IRAQ", 7) \
 X(JIHARKAH, "JIHARKAH", 8) \
 X(KIRDAN, "KIRDAN", 0) \
 X(KURD, "KURD", 2) \
 X(LAMI, "LAMI", 2) \
 X(MAHUR, "MAHUR", 9) \
 X(MUHAYYAR, "MUHAYYAR", 1) \
 X(MUSTAAR, "MUSTAAR", 10) \
 X(NAHAWAND, "NAHAWAND", 2) \
 X(NAHAWAND_MURASSAA, "NAHAWAND MURASSAA", 2) \
 X(NAIRUZ, "NAIRUZ", 11) \
 X(NAWA_ATHAR, "NAWA ATHAR", 2) \
 X(NIKRIZ, "NIKRIZ", 2) \
 X(RAHAT_AL_ARWAH, "RAHAT AL-ARWAH", 6) \
 X(SABA, "SABA", 1) \
 X(SABA_ZAMZAM, "SABA ZAMZAM", 2) \
 X(SAZKAR, "SAZKAR", 0) \
 X(SHADD_ARABAN, "SHADD ARABAN", 2) \
 X(SHAHNAZ, "SHAHNAZ", 2) \
 X(SHAWQ_AFZA, "SHAWQ AFZA", 2) \
 X(SIKAH, "SIKAH", 12) \
 X(SIKAH_BALADI, "SIKAH BALADI APPROX", 13) \
 X(SUZDALARA, "SUZDALARA", 9) \
 X(SUZIDIL, "SUZIDIL", 2) \
 X(SUZNAK, "SUZNAK", 9) \
 X(USHAQ_MASRI, "USHAQ MASRI", 14) \
 X(YAKAH, "YAKAH", 11) \
 X(ZANJARAN, "ZANJARAN", 2) \
 X(RAST_DESC, "RAST DESCENDING", 9) \
 X(BAYATI_RAST, "BAYATI UPPER RAST", 5) \
 X(HIJAZ_RAST, "HIJAZ UPPER RAST", 14)
static const uint16_t MAQAM_MASKS[][3] = { /* down 50, up 50, down 25 cents */
    {0x810,0x0,0x0},
    {0x4,0x0,0x0},
    {0x0,0x0,0x0},
    {0x334,0x800,0x0},
    {0x194,0xc00,0x0},
    {0x204,0x0,0x0},
    {0xb34,0x0,0x0},
    {0xa94,0x0,0x0},
    {0x800,0x0,0x30},
    {0x10,0x0,0x0},
    {0xad8,0x0,0x0},
    {0x210,0x0,0x0},
    {0xa54,0x0,0x0},
    {0xa14,0x0,0x20},
    {0x200,0x0,0x0},
};
static const uint16_t MAQAM_NOTE_MASKS[46] = { /* chromatic keys for documented maqam degrees */
    0xab5,
    0x5ad,
    0xab5,
    0xab5,
    0x9cb,
    0xb35,
    0xdb5,
    0x66d,
    0xab5,
    0x9ad,
    0x5b3,
    0x9b3,
    0x5ab,
    0x6ad,
    0xb35,
    0xab5,
    0xab5,
    0xab5,
    0x5ab,
    0x56b,
    0xab5,
    0x5ad,
    0xad9,
    0x9ad,
    0x66d,
    0x6b5,
    0x9cd,
    0x6cd,
    0xb35,
    0x59d,
    0x59b,
    0xab5,
    0x9b3,
    0x9b3,
    0x9b5,
    0xad5,
    0xaf5,
    0x6b5,
    0x9b3,
    0x9b5,
    0x6ad,
    0x6b5,
    0x6b3,
    0x6b5,
    0x6ad,
    0x6b3,
};
