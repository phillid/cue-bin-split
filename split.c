#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Split.c
 * Splits a raw audio file into segments bounded by the time specified on stdin */

int main(int argc, char **argv)
{
	FILE *fin = NULL;
	FILE *fout = NULL;
	char *in_fname = NULL;
	char out_fname[] = "track-0000";
	int m = 0;
	int s = 0;
	int frame = 0;
	int index = 0;
	int items = 0;
	int channels = 0;
	int rate = 0;
	int sample_size = 0;
	int i = 0;
	double start = 0;
	double finish = 0;
	unsigned long start_sample = 0;
	unsigned long finish_sample = 0;


	/* FIXME Use getopt */
	/* FIXME Replace assertions with useful checks+messages */
	assert(argc == 5);

	in_fname = argv[1];
	channels = atoi(argv[2]);
	rate = atoi(argv[3]);
	sample_size = atoi(argv[4]);

	assert(channels > 0);
	assert(rate > 0);
	assert(sample_size > 0);
	assert(in_fname != NULL);

	/* Open it up */
	if ((fin = fopen(in_fname, "r")) == NULL)
	{
		fprintf(stderr,"Failed to open '%s': ", in_fname);
		perror("fopen");
		return EXIT_FAILURE;
	}
	printf("Opened input file '%s'\n", in_fname);


	index = 0;
	finish = 0; /* FIXME we assume first track starts at beginning of file */
	while ( ( items = fscanf(stdin, "%d:%d:%d\n", &m, &s, &frame) ) != EOF )
	{
		index++;
		/* Following track starts at end of last */
		if (items != 3)
		{
			fprintf(stderr, "Timestamp #%d malformed\n", index);
			break;
		}
		start = finish;
		finish = (double)m*60 + (double)s + ((double)frame)/75;

		fprintf(stderr, "Track %d starts %f s, finishes %f s\n", index, start, finish);

		sprintf(out_fname, "track_%04d", index);


		/* Open it up */
		if ((fout = fopen(out_fname, "w")) == NULL)
		{
			fprintf(stderr,"Failed to open '%s': ", out_fname);
			perror("fopen");
			return EXIT_FAILURE;
		}

		start_sample = start * (double)rate * channels;
		finish_sample = finish * (double)rate * channels;


		/* Seek to first sample and write until last sample */
		fseek(fin, start_sample*sample_size, SEEK_SET);

		/* FIXME put this declaration at top */
		char buffer[10240];
		for (i = (int)start_sample; i < (int)finish_sample; )
		{
			/* FIXME unnecessary call to fwrite with items == 0 possible */
			if ((items = fread(buffer, sample_size, sizeof(buffer)/sample_size, fin)))
				fwrite(buffer, items, sample_size, fout);

			i += items;
		}

		fclose(fout);
	}
}

