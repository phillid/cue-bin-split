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
#include <limits.h>
#include <unistd.h>

#include "cue-bin-split.h"
#include "misc.h"

void args_collect(int *argc, char ***argv, int *rate, int *channels, int *sample_size, char **in_fname, char **format)
{
	char opt = '\0';
	while ( ( opt = getopt(*argc, *argv, "r:c:i:s:f:") ) != -1 )
	{
		switch (opt)
		{
			case 'r': *rate = atoi(optarg); break;
			case 'c': *channels = atoi(optarg); break;
			case 's': *sample_size = atoi(optarg); break;
			case 'i': *in_fname = optarg; break;
			case 'f': *format = optarg; break;

			case '?':
			default:
				die_help();
		}
	}

	if (*channels <= 0 ||
	    *rate <= 0 ||
	    *sample_size <= 0)
	{
		fprintf(stderr, "ERROR: Channel count, bitrate and sample size must all be present and positive\n");
		die_help();
	}

	if (*in_fname == NULL ||
	    *format == NULL)
	{
		fprintf(stderr, "ERROR: Input filename and output name format must be present\n");
		die_help();
	}
}

int main(int argc, char **argv)
{
	/* File handles */
	FILE *fin = NULL;
	FILE *fout = NULL;

	/* Command line options */
	char *format = NULL;
	char *in_fname = NULL;
	int channels = 0;
	int rate = 0;
	int sample_size = 0;

	/* Misc */
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

	args_collect(&argc, &argv, &rate, &channels, &sample_size, &in_fname, &format);

	/* Open it up */
	if ((fin = fopen(in_fname, "r")) == NULL)
	{
		fprintf(stderr,"Failed to open '%s': ", in_fname);
		perror("fopen");
		return -1;
	}

	track = 0;
	start_sec = get_sec();

	/* Start time can't be unspecified, only finish */
	if (start_sec < 0)
	{
		fprintf(stderr, "ERROR: At least one start timestamp must be specified\n");
		return -1;
	}

	/* finish_sample equals ULONG_MAX if a run was to EOF (the last track) */
	while ( finish_sample != ULONG_MAX )
	{
		track++;
		construct_out_name(out_fname, sizeof(out_fname), format, track);
		finish_sec = get_sec();

		printf("%s starts %f s, finishes ", out_fname, start_sec);
		if (finish_sec < 0)
			printf("EOF\n");
		else
			printf("%f s\n", finish_sec);

		/* Open output file */
		if ((fout = fopen(out_fname, "w")) == NULL)
		{
			fprintf(stderr,"Failed to open '%s': ", out_fname);
			perror("fopen");
			fclose(fin);
			return -1;
		}

		start_sample = start_sec * rate * channels;
		finish_sample = finish_sec * rate * channels;

		if (start_sample > finish_sample)
		{
			fprintf(stderr, "ERROR: Finish time can't be before start time, skipping %s", out_fname);
			continue;
		}

		/* Run to "infinity" if no finish time set.
		 * Of course, will actually run to EOF assuming file's small enough
		 * FIXME assumes last track has less than ULONG_MAX samples */
		if (finish_sec == -1)
			finish_sample = ULONG_MAX;

		for (i = start_sample; i < finish_sample; i += items)
		{
			items = fread(buffer,
			              sample_size,
			              MIN(sizeof(buffer)/sample_size, (finish_sample - i)),
			              fin);

			if (items == 0)
			{
				if (feof(fin))
					break;

				if (ferror(fin))
				{
					perror("fread");
					break;
				}
			}

			if (fwrite(buffer, sample_size, items, fout) != items)
			{
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
