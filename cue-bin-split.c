#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#define MIN(a, b) ((a < b)? a : b)
#define FRAMES_PER_SEC 75
#define BUFFER_SIZE    1024*1024 /* Meh, good enough */

/* Grabs a 'mm:ss:ff' stamp from stdin */
double get_sec()
{
	int mm = 0;
	int ss = 0;
	int ff = 0;
	int items = fscanf(stdin, "%d:%d:%d\n", &mm, &ss, &ff);

	/* EOF means this is the last track; return invalid time */
	if (items == EOF)
		return -1;

	if (items != 3) {
		fprintf(stderr, "Timestamp malformed\n");
		exit(-1);
	}
	return (mm*60 + ss + ((double)ff)/FRAMES_PER_SEC);
}

/* Constructs an output filename in the specified buffer based on the given format and track number
 * Main purpose is to catch buffer overflow with snprintf
 */
int construct_out_name(char *buffer, size_t buffer_size, char* name, unsigned int track)
{
	if (snprintf(buffer, buffer_size, "%03d%s", track, name) >= buffer_size - 1) {
		fprintf(stderr, "Filename too large for buffer (max %zd)\n", buffer_size);
		return -1;
	}
	return 0;
}

/* Displays the help/syntax message and dies
 * Does what it says on the tin
 */
void die_help()
{
	fprintf(stderr,
		"\n"
		"Options: \n"
		"  -r bitrate_Hz\n"
		"  -c channel_count\n"
		"  -i input_file\n"
		"  -s size of a single channel's sample (bytes)\n"
		"  -n output file name (prepended with track number)\n"
	);
	exit(1);
}

int main(int argc, char **argv)
{
	/* File handles */
	FILE *fin = NULL;
	FILE *fout = NULL;

	/* Command line options */
	char *name = NULL;
	char *in_fname = NULL;
	int channels = 0;
	int rate = 0;
	int sample_size = 0;

	/* Misc */
	char opt = '\0';
	char out_fname[1024]; /* That should do it. Note: overflow IS caught */
	int track = 0;
	int items = 0;
	unsigned long i = 0;
	char buffer[BUFFER_SIZE];

	/* Boundaries */
	double start_sec = 0;
	double finish_sec = 0;
	unsigned long start_sample = 0;
	unsigned long finish_sample = 0;

	while ((opt = getopt(argc, argv, "r:c:i:s:n:")) != -1) {
		switch (opt) {
		case 'r': rate = atoi(optarg); break;
		case 'c': channels = atoi(optarg); break;
		case 's': sample_size = atoi(optarg); break;
		case 'i': in_fname = optarg; break;
		case 'n': name = optarg; break;

		case '?':
		default:
			die_help();
		}
	}

	if (channels <= 0 ||
	    rate <= 0 ||
	    sample_size <= 0) {
		fprintf(stderr, "ERROR: Channel count, bitrate and sample size must all be present and positive\n");
		die_help();
	}

	if (in_fname == NULL ||
	    name == NULL) {
		fprintf(stderr, "ERROR: Input filename and output name must be present\n");
		die_help();
	}

	/* Open it up */
	if ((fin = fopen(in_fname, "r")) == NULL) {
		fprintf(stderr,"Failed to open '%s': ", in_fname);
		perror("fopen");
		return -1;
	}

	track = 0;
	start_sec = get_sec();

	/* Start time can't be unspecified, only finish */
	if (start_sec < 0) {
		fprintf(stderr, "ERROR: At least one start timestamp must be specified\n");
		return -1;
	}

	/* finish_sample equals ULONG_MAX if a run was to EOF (the last track) */
	while (finish_sample != ULONG_MAX) {
		track++;
		if (construct_out_name(out_fname, sizeof(out_fname), name, track) < 0) {
			fclose(fin);
			return 1;
		}
		finish_sec = get_sec();

		printf("%s starts %f s, finishes ", out_fname, start_sec);
		if (finish_sec < 0)
			printf("EOF\n");
		else
			printf("%f s\n", finish_sec);

		/* Open output file */
		if ((fout = fopen(out_fname, "w")) == NULL) {
			fprintf(stderr,"Failed to open '%s': ", out_fname);
			perror("fopen");
			fclose(fin);
			return -1;
		}

		start_sample = start_sec * rate * channels;
		finish_sample = finish_sec * rate * channels;

		if (start_sample > finish_sample) {
			fprintf(stderr, "ERROR: Finish time can't be before start time, skipping %s\n", out_fname);
			continue;
		}

		/* Run to "infinity" if no finish time set.
		 * Of course, will actually run to EOF assuming file's small enough
		 * FIXME assumes last track has less than ULONG_MAX samples */
		if (finish_sec == -1)
			finish_sample = ULONG_MAX;

		for (i = start_sample; i < finish_sample; i += items) {
			items = fread(buffer,
			              sample_size,
			              MIN(sizeof(buffer)/sample_size, (finish_sample - i)),
			              fin);

			if (items == 0) {
				if (feof(fin))
					break;

				if (ferror(fin)) {
					perror("fread");
					break;
				}
			}

			if (fwrite(buffer, sample_size, items, fout) != items) {
				fprintf(stderr, "Write to %s failed: ", out_fname);
				perror("fwrite");
				break;
			}
		}
		fclose(fout);
		start_sec = finish_sec;
	}
	fclose(fin);
	return 0;
}
