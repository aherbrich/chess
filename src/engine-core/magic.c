// /* THE FOLLWOING FUNCTIONS ARE NOT USED AT RUNTIME BUT ARE RATHER USED FOR THE
//  * PRECALCULATION OF MAGIC NUMBERS AND BITBOARDS (see ROOK_MAGIC and
//  * BISHOP_MAGIC above) */

// /* finds magic numbers by trail and error */
// bitboard_t find_magic(int sq, int nr_bits, int for_bishop) {
//     bitboard_t mask, magic;
//     bitboard_t blockermap[4096], attackmap[4096], used[4096];

//     mask = (for_bishop) ? bishop_mask(sq) : rook_mask(sq);

//     /* generate all possible blocker maps and the resulting attack maps for the
//      * piece at sq */
//     for (int i = 0; i < (1 << nr_bits); i++) {
//         blockermap[i] = index_to_bitboard(i, nr_bits, mask);
//         attackmap[i] = (for_bishop) ? bishop_attacks(sq, blockermap[i])
//                                     : rook_attacks(sq, blockermap[i]);
//     }

//     for (int k = 0; k < 100000000; k++) {
//         magic = random_uint64_fewbits();

//         for (int i = 0; i < 4096; i++) {
//             used[i] = 0ULL;
//         }

//         int fail = 0;

//         for (int i = 0; i < (1 << nr_bits); i++) {
//             int j = transform(blockermap[i], magic, nr_bits);
//             if (used[j] == 0ULL) {
//                 used[j] = attackmap[i];
//             } else if (used[j] != attackmap[i]) {
//                 fail = 1;
//                 break;
//             }
//         }

//         if (!fail) {
//             return magic;
//         }
//     }
//     fprintf(stderr, "*** Failed to find magic number ***\n");
//     return 0ULL;
// }

// /* determines and prints all magic numbers (for all squares) */
// void print_magic_numbers() {
//     int square;
//     fprintf(stderr, "uint64_t ROOK_MAGIC[64] = {\n");
//     for (square = 0; square < 64; square++)
//         fprintf(stderr, "  0x%llxULL,\n",
//                 find_magic(square, ROOK_BITS[square], 0));
//     fprintf(stderr, "};\n\n");

//     fprintf(stderr, "uint64_t BISHOP_MAGIC[64] = {\n");
//     for (square = 0; square < 64; square++)
//         fprintf(stderr, "  0x%llxULL,\n",
//                 find_magic(square, BISHOP_BITS[square], 1));
//     fprintf(stderr, "};\n\n");
// }