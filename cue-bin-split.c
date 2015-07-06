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
#include <getopt.h>

#include "cue-bin-split.h"
#include "misc.h"

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
	char opt = 0;
	char out_fname[1024]; /* That should do it. Note: overflow IS caught */
	int track = 0;
	int items = 0;
	int i = 0;
	char buffer[BUFFER_SIZE];

	/* Timestamp building blocks */
	int mm = 0;
	int ss = 0;
	int ff = 0;

	/* Boundaries */
	double start_sec = 0;
	double finish_sec = 0;
	unsigned long start_sample = 0;
	unsigned long finish_sample = 0;
	
	while ( ( opt = getopt(argc, argv, "r:c:i:s:f:") ) != -1 )
	{
		switch (opt)
		{
			case 'r': rate = atoi(optarg); break;
			case 'c': channels = atoi(optarg); break;
			case 's': sample_size = atoi(optarg); break;
			case 'i': in_fname = optarg; break;
			case 'f': format = optarg; break;

			case '?':
			default:
				die_help();
				break;
		}
	}

	if (channels <= 0 ||
	    rate <= 0 ||
	    sample_size <= 0)
	{
		fprintf(stderr, "ERROR: Channel count, bitrate and sample size must all be present and positive\n");
		die_help();
	}

	if (in_fname == NULL ||
	    format == NULL)
	{
		fprintf(stderr, "ERROR: Input filename and output name format must be present\n");
		die_help();
	}

	/* Open it up */
	if ((fin = fopen(in_fname, "r")) == NULL) 
	{
		fprintf(stderr,"Failed to open '%s': ", in_fname);
		perror("fopen");
		return EXIT_FAILURE;
	}

	track = 0;
	items = get_stamp(&mm, &ss, &ff); /* FIXME doesn't check return value */
	start_sec = (double)mm*60 + (double)ss + ((double)ff)/FRAMES_PER_SEC;

	while ( ( items = get_stamp(&mm, &ss, &ff) ) )
	{
		track++;

		construct_out_name(out_fname, sizeof(out_fname), format, track);

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
			finish_sec = -1;
			printf("%s starts %f s, finished EOF\n", out_fname, start_sec);
		} else {
			/* Following track starts at end of last */
			if (items != 3)
			{
				fprintf(stderr, "Timestamp #%d malformed\n", track);
				break;
			}
			finish_sec = (double)mm*60 + (double)ss + ((double)ff)/75;
			printf("%s starts %f s, finishes %f s\n", out_fname, start_sec, finish_sec);
		}

		start_sample = start_sec * rate * channels;
		finish_sample = finish_sec * rate * channels;

		if (start_s > finish_s)
		{
			fprintf(stderr, "ERROR: Finish time can't be before start time, skipping %s", out_fname);
			continue;
		}


		/* FIXME this copies whole buffers, potentially going over end of track */
		for (i = (int)start_sample; i != (int)finish_sample; i += items)
		{
			if ((items = fread(buffer, sample_size, sizeof(buffer)/sample_size, fin)))
			{
				if (feof(fin))
					break;
				fwrite(buffer, items, sample_size, fout);
			}
		}

		fclose(fout);
		start_sec = finish_sec;

		/* FIXME ick */
		if (finish_sec < 0)
			break;
	}
	return 0;
}

