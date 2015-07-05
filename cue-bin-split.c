/*
 * cue-bin-split - Split raw PCM files on time boundaries
 * Copyright (c) 2015 David Phillips <dbphillipsnz@gmail.com>
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char **argv)
{
	FILE *fin = NULL;
	FILE *fout = NULL;
	char *format = NULL;
	char *in_fname = NULL;
	char out_fname[] = "track-000000000000"; /* That should do it */
	int m = 0;
	int s = 0;
	int frame = 0;
	int index = 0;
	int items = 0;
	int channels = 0;
	int rate = 0;
	int sample_size = 0;
	int i = 0;
	char buffer[10240];
	double start = 0;
	double finish = 0;
	unsigned long start_sample = 0;
	unsigned long finish_sample = 0;


	/* FIXME Use getopt */
	/* FIXME Replace assertions with useful checks+messages */
	assert(argc == 6);

	in_fname = argv[1];
	channels = atoi(argv[2]);
	rate = atoi(argv[3]);
	sample_size = atoi(argv[4]);
	format = argv[5];

	assert(channels > 0);
	assert(rate > 0);
	assert(sample_size > 0);
	assert(in_fname != NULL);
	assert(format != NULL);

	/* Open it up */
	if ((fin = fopen(in_fname, "r")) == NULL)
	{
		fprintf(stderr,"Failed to open '%s': ", in_fname);
		perror("fopen");
		return EXIT_FAILURE;
	}
	printf("Opened input file '%s'\n", in_fname);


	index = 0;
	items = fscanf(stdin, "%d:%d:%d\n", &m, &s, &frame); /* FIXME doesn't check return value */
	start = (double)m*60 + (double)s + ((double)frame)/75;

	/* FIXME duplication of code to get  mm:ss:ff */
	while ( ( items = fscanf(stdin, "%d:%d:%d\n", &m, &s, &frame) ) )
	{
		index++;

		i = snprintf(out_fname, sizeof(out_fname), format, index);
		if (i == sizeof(out_fname))
		{
			fprintf(stderr, "Filename too large for buffer\n");
			fclose(fin);
			return EXIT_FAILURE;
		}


		/* Open output file */
		if ((fout = fopen(out_fname, "w")) == NULL)
		{
			fprintf(stderr,"Failed to open '%s': ", out_fname);
			perror("fopen");
			fclose(fin);
			return EXIT_FAILURE;
		}


		/* EOF means this is the last track; run it to end of input file */
		if (items == EOF)
		{
			finish = -1;
		} else {
			/* Following track starts at end of last */
			if (items != 3)
			{
				fprintf(stderr, "Timestamp #%d malformed\n", index);
				break;
			}
			finish = (double)m*60 + (double)s + ((double)frame)/75;
		}
		fprintf(stderr, "Track %d starts %f s, finishes %f s\n", index, start, finish);

		start_sample = start * rate * channels;
		finish_sample = finish * rate * channels;


		/* Seek to first sample of track */
		fseek(fin, (start_sample * sample_size), SEEK_SET);

		if (finish >= 0)
		{
			for (i = (int)start_sample; i < (int)finish_sample; i += items)
			{
				/* FIXME unnecessary call to fwrite with items == 0 possible */
				/* FIXME Handle EOF properly, silly! */
				if ((items = fread(buffer, sample_size, sizeof(buffer)/sample_size, fin)))
				{
					if (feof(fin))
						break;
					fwrite(buffer, items, sample_size, fout);
				}
			}
		} else {
			for (i = (int)start_sample; ; i += items)
			{
				/* FIXME unnecessary call to fwrite with items == 0 possible */
				if ((items = fread(buffer, sample_size, sizeof(buffer)/sample_size, fin)))
				{
					if (feof(fin))
						break;
					fwrite(buffer, items, sample_size, fout);
				}
			}

		}
		fclose(fout);
		start = finish;

		if (finish < 0)
			break;
	}
	return 0;
}

