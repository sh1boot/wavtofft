#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <math.h>
#include <sndfile.h>
#include <fftw3.h>

int main(int argc, char *argv[])
{
    char const *inname = NULL;
    char const *outname = NULL;
    char const *initstring = NULL;
    char const *preamble = NULL;
    char const *postamble = NULL;
    uint32_t length = 16384;
    sf_count_t seek = 0;
    double seek_sec = 0;
    sf_count_t step = 0;
    double step_sec = 0;
    SNDFILE *infile = NULL;
    FILE *outfile = NULL;
    int decimate = 1;
    SF_INFO sfinfo;
    double *in = NULL;
    fftw_complex *out = NULL;
    fftw_plan plan = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "i:o:l:s:S:t:T:w:d:I:p:P:")) != -1)
        switch (opt)
        {
        case 'i': inname = optarg;                          break;
        case 'o': outname = optarg;                         break;
        case 'l': length = atoi(optarg);                    break;
        case 's': seek_sec = atof(optarg);                  break;
        case 'S': seek = atoll(optarg);                     break;
        case 't': step_sec = atof(optarg);                  break;
        case 'T': step = atoll(optarg);                     break;
        case 'w':
            if (strcmp(optarg, "rectangular") && strcmp(optarg, "boxcar"))
                fprintf(stderr, "only rectangular and boxcar window functions supported.\n");
            break;
        case 'd': decimate = atoi(optarg);                  break;
        case 'I': initstring = optarg;                      break;
        case 'p': preamble = optarg;                        break;
        case 'P': postamble = optarg;                       break;
        default:
            fprintf(stderr, "unknown option '%c'\n", opt);
            exit(EXIT_FAILURE);
        }

    if ((infile = sf_open(inname, SFM_READ, &sfinfo)) == NULL)
    {
        fprintf(stderr, "couldn't open input outfile '%s'\n", inname);
        exit(EXIT_FAILURE);
    }

    if (outname == NULL)
        outfile = stdout;
    else if ((outfile = fopen(outname, "wt")) == NULL)
    {
        fprintf(stderr, "couldn't open output outfile '%s'\n", outname);
        exit(EXIT_FAILURE);
    }

    if (initstring) fprintf(outfile, "%s\n", initstring);

    in = fftw_malloc(sizeof(*in) * sfinfo.channels * length);
    out = fftw_malloc(sizeof(*out) * sfinfo.channels * length);
    if (in && out)
    {
        int n[1] = { length };

        plan = fftw_plan_many_dft_r2c(1, n, sfinfo.channels,
                                    in, NULL, sfinfo.channels, 1,
                                    out, NULL, sfinfo.channels, 1,
                                    FFTW_ESTIMATE | FFTW_DESTROY_INPUT);
    }
    if (plan == NULL)
    {
        fprintf(stderr, "couldn't initialise fftw.\n");
        exit(EXIT_FAILURE);
    }

    seek += rint(seek_sec * sfinfo.samplerate);
    step += rint(step_sec * sfinfo.samplerate);

    do
    {
        int r, c;

        sf_seek(infile, (sf_count_t)rint(seek), SEEK_SET);
        r = sf_readf_double(infile, in, length) * sfinfo.channels;
        if (r <= 0)
            break;
        if (r < length * sfinfo.channels)
            step = 0;
        while (r < length * sfinfo.channels)
            in[r++] = 0.0;

        fftw_execute(plan);

        if (preamble) fprintf(outfile, "%s\n", preamble);

        for (r = 0; r * 2 < length; r += decimate)
        {
            double f = (double)r * sfinfo.samplerate / length;

            fprintf(outfile, "%lf", f);

            for (c = 0; c < sfinfo.channels; c++)
            {
                double x = 0.0;
                int i;
                for (i = 0; i < decimate; i++)
                {
                    fftw_complex *p = &out[(r + i) * sfinfo.channels + c];
                    double y = p[0][0] * p[0][0] + p[0][1] * p[0][1];
                    if (x < y)
                        x = y;
                }
                x = log10(x * 2.0) / 2.0 * 20.0 - log10(length) * 20.0;
                fprintf(outfile, " %lf", x);
            }
            fprintf(outfile, "\n");
        }

        if (postamble) fprintf(outfile, "%s\n", postamble);

        seek += step;
    } while (step > 0);

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    sf_close(infile);
    if (outname != NULL)
        fclose(outfile);

    return EXIT_SUCCESS;
}
