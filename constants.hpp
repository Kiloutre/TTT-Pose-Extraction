#include <stdint.h>

typedef uint64_t gameAddr;
const unsigned int PLAYER_COUNT = 1;
const gameAddr BASE_ADDRESS = 0x20000000;
const gameAddr PLAYER_OFFSET = 0xB8A0; // Size of the player structure, distance between two players
const gameAddr MOVESET_ID = 0x884F08; // Which moveset to load
const char* EXTRACTION_FOLDER = "TTT_Poses/"; // Must end with /

// Each of these is an address to 3x floats
const std::vector<gameAddr> BONE_OFFSETS = {
	0x87A5A0,
	0x87A8A0,
	0x87B560,
	0x87B620,
	0x87BAA0,
	0x87BCE0,
	0x87BF20,
	0x87C0A0,
	0x87C3A0,
	0x87CA60,
	0x87CD60,
	0x87CFA0,
	0x87D1E0,
	0x87D360,
	0x87D660,
	0x87DD20,
	0x87E260,
	0x87E4A0,
	0x87E560,
	0x87E620,
	0x87E920,
	0x87EB60,
	0x87EC20,
	0x87ECE0
};
