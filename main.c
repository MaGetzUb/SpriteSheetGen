#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void TestArgumentSanity(int index, int num_args, bool verbose) {
	if((index + 1) == num_args && verbose) {
		puts("ERROR: Not enough arguments!");
		exit(-1);
	}
}

int main(int argc, char* argv[]) {

	int i, j, k;
	
	int sprite_width = 0;
	int sprite_height = 0;
	int sprites_per_row = 0;

	int frame_count=0;
	char* pattern = NULL;
	char* input_dir = NULL;
	char* output_file = NULL;
	char* tmp_ptr;
	stbi_uc** images = NULL;
	stbi_uc* result_buffer = NULL;

	char file_path[512] = { 0 };
	char file[512] = { 0 };

	int x = 0;
	int y = 0;
	int cmp = 0;
	int start_index = 0;

	int result_size_x;
	int result_size_y;

	unsigned pixel;
	bool verbose = true;

	stbi_uc* write = NULL;
	stbi_uc* read = NULL;

	clock_t start, result1, result2, result3;


#define dbg_puts(msg) if(verbose) puts(msg)
#ifdef _MSC_VER
#define dbg_printf(msg, ...) if(verbose) printf(msg, __VA_ARGS__);
#else 
#define dbg_printf(msg, ...) if(verbose) printf(msg, #__VA_ARGS__);
#endif 
	if(argc < 1) {
		dbg_puts("No thing to do, no arguments given. Try --help");
	}

	for(i = 1; i < argc; i++) {

		if(strstr(argv[i], "--pattern") || strstr(argv[i], "-pt")) {
			
			TestArgumentSanity(i, argc, verbose);
			
			if(!pattern) {
				pattern = argv[++i];
				if(pattern[0] != '$') {
					dbg_puts("ERROR: Invalid pattern!");
				}
				else {
					pattern[0]='%';
				}
			}
			else
				dbg_puts("Pattern is already set.");
		} else if(strstr(argv[i], "--sprites-per-row") || strstr(argv[i], "-spr")) {

			TestArgumentSanity(i, argc, verbose);

			if(!sprites_per_row) {
				char* end_ptr;
				sprites_per_row = strtol(argv[++i], &end_ptr, 10);
				if(errno == ERANGE) {
					dbg_puts("Value given to argument is not a number!");
					exit(-1);
				}
			} else {
				dbg_puts("Sprites per row is already set!");
			}
		}
		else if(strstr(argv[i], "--start-index") || strstr(argv[i], "-si")) {

			TestArgumentSanity(i, argc, verbose);

			if(!sprites_per_row) {
				char* end_ptr;
				start_index = strtol(argv[++i], &end_ptr, 10);
				if(errno == ERANGE) {
					dbg_puts("Value given to argument is not a number!");
					exit(-1);
				}
			}
			else {
				dbg_puts("Sprites per row is already set!");
			}
		}
		else if(strstr(argv[i], "--input-dir") || strstr(argv[i], "-idir")) {
			
			TestArgumentSanity(i, argc, verbose);

			if(!input_dir) {
				input_dir = argv[++i];
				strcpy(file_path, input_dir);
			}
			else {
				dbg_puts("Input directory is already set!");
			}
		}
		else if(strstr(argv[i], "--frame-count") || strstr(argv[i], "-fc")) {

			TestArgumentSanity(i, argc, verbose);

			if(!frame_count) {
				char* endptr;
				frame_count = strtol(argv[++i], &endptr, 10);
				if(errno == ERANGE) {
					dbg_puts("Value given to argument is not a number!");
					exit(-1);
				}
			}
			else {
				dbg_puts("Frame count is already set!");
			}

		}
		else if(strstr(argv[i], "--output") || strstr(argv[i], "-o")) {
			TestArgumentSanity(i, argc, verbose);
			
			if(!output_file) {
				output_file = argv[++i];
			}
			else {
				dbg_puts("Output directory already set.");
			}

		}
		else if(strstr(argv[i], "--silent") || strstr(argv[i], "-si")) {
			dbg_puts("Prints nothing into console.");

		}
		else if(strstr(argv[i], "-h") || strstr(argv[i], "?") || strstr(argv[i], "-help") != NULL) {
			puts("This is a spritesheet gen. By MaGetzUb 2k20");
			puts("You can generate sprite sheets from singular files with this utility.");
			puts("");
			puts("Supported image formats are: .png, .bmp, .jpg and .tga");
			puts("");
			puts("Arguments: ");
			puts("--pattern <pattern>, -pt <pattern>				Sets a file search pattern using C-formatting,");
			puts("\t\t\t\t\t\t\t\texcept percentage sign (%)  is replaced with $,");
			puts("\t\t\t\t\t\t\t\tso $04d for example");
			puts("--sprites-per-row <number>, -spr <number>			Sets number of sprites per row in the output file.");
			puts("--start-index <number>, -si <number>				Sets the start index for filename aka. For example 1, for 0001.png");
			puts("--input-dir <directory path>, -idir <pattern>		Sets a directory where to scan the images.");
			puts("--frame-count <number>, -fc <number>				How many images are we reading from the current directory.");
			puts("--output <file path>, -o <file path>				Sets an output file path.");
			exit(0);
		}
	}
	
	
	images = malloc(sizeof(stbi_uc*) * frame_count);

	j = strlen(file_path);
	if(file_path[j - 1] != '\\' && file_path[j - 1] != '/') {
#ifdef _WIN32
		strcat(file_path, "\\");
#else 
		strcat(file_path, "/");
#endif 
	}

	start = clock();
	j = 0; 
	for(i = start_index; i < frame_count; i++, j++) {
		sprintf(file, pattern, i);
		sprintf(file_path, "%s%s", input_dir, file);
		images[j] = stbi_load(file_path, &x, &y, &cmp, 4);
		if(images[j] == NULL) {
			dbg_printf("Cannot load image: %s\n", file_path);
			dbg_puts("Skipping frame...");
			j--;
			frame_count--;
		} else {
			if(!sprite_width) {
				sprite_width = x;
				sprite_height = y;

				result_size_x = sprite_width * sprites_per_row;
				result_size_y = sprite_height * (frame_count / sprites_per_row);
			}
			else {
				if(sprite_width != x && sprite_height != y) {
					dbg_printf("ERROR: Inconsistent sprite size in file: %s\n", file_path);
					exit(0);
				}
			}
		}
	}

	result1 = clock();

	if(result_size_x <= 0) {
		dbg_puts("ERROR: no files found.");
		exit(-1);
	}

	result_buffer = malloc(result_size_x * result_size_y * 4);
	memset(result_buffer, 0, result_size_x* result_size_y * 4);

	start = clock();
	x = 0;
	y = 0;

	for(i = start_index; i < start_index + frame_count; i++) {
		x = i % sprites_per_row;
		y = i / sprites_per_row;
		for(j = 0; j < sprite_height; j++) {

			write = result_buffer + (x * sprite_width * 4) + (y * sprite_height * result_size_x * 4 + j * result_size_x * 4);
			read = images[i] + (j * sprite_width * 4);

			memcpy(write, read, sprite_width * 4);

		}
		stbi_image_free(images[i]);
	}
	
	free(images);

	result2 = clock() - start;

	tmp_ptr = strchr(output_file, '.');

	if(tmp_ptr == NULL) {
		puts("ERROR! No output file specified!");
		exit(-1);
	}

	start = clock();

	if(!strcmp(tmp_ptr, ".bmp")) {
		if(!stbi_write_bmp(output_file, result_size_x, result_size_y, 4, result_buffer, 0)) {
			dbg_puts("ERROR! JPG write failed!");
			exit(-1);
		}
	} else if(!strcmp(tmp_ptr, ".png")) {
		if(!stbi_write_png(output_file, result_size_x, result_size_y, 4, result_buffer, 0)) {
			dbg_puts("ERROR! JPG write failed!");
			exit(-1);
		}
	} else if(!strcmp(tmp_ptr, ".jpg")) {
		if(!stbi_write_jpg(output_file, result_size_x, result_size_y, 4, result_buffer, 9)) {
			dbg_puts("ERROR! JPG write failed!");
			exit(-1);
		}
	} else if(!strcmp(tmp_ptr, ".tga")) {
		if(!stbi_write_tga(output_file, result_size_x, result_size_y, 4, result_buffer)) {
			dbg_puts("ERROR! JPG write failed!");
			exit(-1);
		}
	} else {
		dbg_printf("Only supported formats are: .png, .bmp, .jpg, .tga.");
		exit(0);
	}

	result3 = clock() - start;

	if(verbose) {
		printf("Loading %d images took: %Fs\n", frame_count, (((double)result1) / CLOCKS_PER_SEC));
		printf("Stitching took: %Fs\n", (((double)result2) / CLOCKS_PER_SEC));
		printf("Saving result took: %Fs\n", (((double)result2) / CLOCKS_PER_SEC));
		printf("All major operations took: %Fs combined.\n", (((double)(result1 + result2 + result3)) / CLOCKS_PER_SEC));
		i = getchar();
	}
	return true;

}
