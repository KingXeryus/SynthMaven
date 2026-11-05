/* SPDX-License-Identifier: GPL-3.0-only
 * 12-key reference tunings; factual source/provenance: assets/maqam/scales.json.
 * First two entries preserve the existing RAST/BAYATI IDs. */

/* In each 12-bit mask, bit 0 (the rightmost/low bit) is the tonic; bit n
 * represents the chromatic key n semitones above it. */

#define MAQAM_EXISTING(X) \
 X(RAST, "RAST", 0,                            0b101010110101) \
 X(BAYATI, "BAYATI", 1,                        0b010110101101)
#define MAQAM_ADDED(X) \
 X(AJAM, "AJAM", 2,                            0b101010110101) \
 X(AJAM_USHAYRAN, "AJAM USHAYRAN", 2,          0b101010110101) \
 X(ATHAR_KURD, "ATHAR KURD", 2,                0b100111001011) \
 X(AWJ_IRAQ, "AWJ IRAQ", 3,                    0b100110011011) \
 X(BASTANIKAR, "BASTANIKAR", 4,                0b110011101011) \
 X(BAYATI_SHURI, "BAYATI SHURI", 1,            0b011001101101) \
 X(DALANSHIN, "DALANSHIN", 0,                  0b101010110101) \
 X(FARAHFAZA, "FARAHFAZA", 2,                  0b100110101101) \
 X(HIJAZ, "HIJAZ", 2,                          0b010110110011) \
 X(HIJAZKAR, "HIJAZKAR", 2,                    0b100110110011) \
 X(HIJAZKAR_KURD, "HIJAZKAR KURD", 2,          0b010110101011) \
 X(HUSAYNI, "HUSAYNI", 5,                      0b011010101101) \
 X(HUZAM, "HUZAM", 3,                          0b010110011011) \
 X(IRAQ, "IRAQ", 4,                            0b010101101011) \
 X(JIHARKAH, "JIHARKAH", 8,                    0b101010110101) \
 X(KIRDAN, "KIRDAN", 0,                        0b101010110101) \
 X(KURD, "KURD", 2,                            0b010110101011) \
 X(LAMI, "LAMI", 2,                            0b010101101011) \
 X(MAHUR, "MAHUR", 9,                          0b101010110101) \
 X(MUHAYYAR, "MUHAYYAR", 1,                    0b010110101101) \
 X(MUSTAAR, "MUSTAAR", 10,                     0b010101101101) \
 X(NAHAWAND, "NAHAWAND", 2,                    0b100110101101) \
 X(NAHAWAND_MURASSAA, "NAHAWAND MURASSAA", 2,  0b011001101101) \
 X(NAIRUZ, "NAIRUZ", 11,                       0b011010110101) \
 X(NAWA_ATHAR, "NAWA ATHAR", 2,                0b100111001101) \
 X(NIKRIZ, "NIKRIZ", 2,                        0b011011001101) \
 X(RAHAT_AL_ARWAH, "RAHAT AL-ARWAH", 6,        0b101100110101) \
 X(SABA, "SABA", 1,                            0b010110011101) \
 X(SABA_ZAMZAM, "SABA ZAMZAM", 2,              0b010110011011) \
 X(SAZKAR, "SAZKAR", 0,                        0b101010110101) \
 X(SHADD_ARABAN, "SHADD ARABAN", 2,            0b100110110011) \
 X(SHAHNAZ, "SHAHNAZ", 2,                      0b100110110011) \
 X(SHAWQ_AFZA, "SHAWQ AFZA", 2,                0b100110110101) \
 X(SIKAH, "SIKAH", 12,                         0b010110101011) \
 X(SIKAH_BALADI, "SIKAH BALADI APPROX", 13,    0b101011110101) \
 X(SUZDALARA, "SUZDALARA", 9,                  0b011010110101) \
 X(SUZIDIL, "SUZIDIL", 2,                      0b100110110011) \
 X(SUZNAK, "SUZNAK", 9,                        0b100110110101) \
 X(USHAQ_MASRI, "USHAQ MASRI", 14,             0b011010101101) \
 X(YAKAH, "YAKAH", 11,                         0b011010110101) \
 X(ZANJARAN, "ZANJARAN", 2,                    0b011010110011) \
 X(RAST_DESC, "RAST DESCENDING", 9,            0b011010110101) \
 X(BAYATI_RAST, "BAYATI UPPER RAST", 5,        0b011010101101) \
 X(HIJAZ_RAST, "HIJAZ UPPER RAST", 14,         0b011010110011)
static const uint16_t MAQAM_MASKS[][3] = { /* down 50, up 50, down 25 cents */
    {0b100000010000,0b000000000000,0b000000000000},
    {0b000000000100,0b000000000000,0b000000000000},
    {0b000000000000,0b000000000000,0b000000000000},
    {0b000000000001,0b000000000000,0b000000000000},
    {0b000000100001,0b000000000000,0b000000000000},
    {0b001000000100,0b000000000000,0b000000000000},
    {0b101100110100,0b000000000000,0b000000000000},
    {0b101010010100,0b000000000000,0b000000000000},
    {0b100000000000,0b000000000000,0b000000110000},
    {0b000000010000,0b000000000000,0b000000000000},
    {0b000000000001,0b000000000000,0b000000000000},
    {0b001000010000,0b000000000000,0b000000000000},
    {0b000010000001,0b000000000000,0b000000000000},
    {0b101000010100,0b000000000000,0b000000100000},
    {0b001000000000,0b000000000000,0b000000000000},
};
#define SM_MAQAM_NOTE_MASK(id,name,profile,note_mask) note_mask,
static const uint16_t MAQAM_NOTE_MASKS[46] = { /* chromatic keys for documented maqam degrees */
    MAQAM_EXISTING(SM_MAQAM_NOTE_MASK) MAQAM_ADDED(SM_MAQAM_NOTE_MASK)
};
#undef SM_MAQAM_NOTE_MASK
