#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// arkas
#include "../../../source/art.h"

void* Int16(uint16_t value) {
	static uint8_t ret[2];
	ret[0] = (uint8_t) (value & 0xFF);
	ret[1] = (uint8_t) ((value & 0xFF00) >> 8);

	return (void*) &ret;
}

int main(int argc, const char** argv) {
	const char* inFile  = NULL;
	const char* outFile = NULL;

	if (argc == 1) {
		printf("%s FILE [-o OUT.art]\n", argv[0]);
		return 0;
	}

	for (int i = 1; i < argc; ++ i) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-o") == 0) {
				++ i;

				if (i == argc) {
					fprintf(stderr, "-o expects OUT.art parameter");
					return 1;
				}

				outFile = argv[i];
			}
			else {
				fprintf(stderr, "Unknown flag '%s'", argv[i]);
				return 1;
			}
		}
		else {
			if (inFile) {
				fprintf(stderr, "Input file set multiple times\n");
				return 1;
			}

			inFile = argv[i];
		}
	}

	if (!inFile) {
		fprintf(stderr, "No input file provided\n");
		return 1;
	}

	if (!outFile) {
		fprintf(stderr, "No output file provided\n");
	}

	int w, h, ch;
	uint8_t* data = stbi_load(inFile, &w, &h, &ch, 0);

	printf("Image is %dx%d\n", w, h);

	if (!data) {
		fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
		return 1;
	}

	FILE* out = fopen(outFile, "wb");

	if (!out) {
		fprintf(stderr, "Failed to open output file\n");
		return 1;
	}

	fprintf(out, "ART");

	if (ch == 3) {
		fputc(AE_ART_TYPE_RGB, out);
		puts("Image has 3 channels");
	}
	else if (ch == 4) {
		fputc(AE_ART_TYPE_RGBA, out);
		puts("Image has 4 channels");
	}

	fwrite(Int16((uint16_t) w), 2, 1, out);
	fwrite(Int16((uint16_t) h), 2, 1, out);

	if ((int) fwrite(data, ch, w * h, out) != w * h) {
		fprintf(stderr, "Failed to write to file\n");
		return 1;
	}

	fclose(out);
	free(data);
	return 0;
}
