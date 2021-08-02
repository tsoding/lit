#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define SV_IMPLEMENTATION
#include "./sv.h"

#define FLAG_IMPLEMENTATION
#include "./flag.h"

#include "./mf.c"

void usage(FILE *stream)
{
    fprintf(stream, "Usage: lit [OPTIONS]\n");
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
}

void markup_to_program(String_View content, FILE *stream, char *begin, char *end, char *comment)
{
    bool code_mode = false;
    while (content.count > 0) {
        String_View line = sv_chop_by_delim(&content, '\n');

        if (code_mode) {
            if (sv_eq(sv_trim(line), sv_from_cstr(end))) {
                fprintf(stream, "%s " SV_Fmt"\n", comment, SV_Arg(line));
                code_mode = false;
            } else {
                fprintf(stream, SV_Fmt"\n", SV_Arg(line));
            }
        } else {
            fprintf(stream, "%s " SV_Fmt"\n", comment, SV_Arg(line));
            if (sv_eq(sv_trim(line), sv_from_cstr(begin))) {
                code_mode = true;
            }
        }
    }
}

void program_to_markup(String_View content, FILE *stream, char *begin, char *end, char *comment)
{
    (void) content;
    (void) stream;
    (void) begin;
    (void) end;
    (void) comment;
    assert(0 && "TODO: implement program_to_markup mode");
}

int main(int argc, char **argv)
{
    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0");
    char **input = flag_str("input", NULL, "Path to the input file");
    char **begin = flag_str("begin", "\\begin{code}", "Line that denotes the beginning of the code block in the markup language");
    char **end = flag_str("end", "\\end{code}", "Line that denotes the end of the code block in the markup language");
    char **comment = flag_str("comment", "//", "The inline comment of the programming language");
    char **mode = flag_str("mode", "m2p", "Conversion mode. m2p -- markup to program. p2m -- program to markup");

    if (!flag_parse(argc, argv)) {
        usage(stderr);
        flag_print_error(stderr);
        exit(1);
    }

    if (*help) {
        usage(stdout);
        exit(0);
    }

    if (*input == NULL) {
        usage(stderr);
        fprintf(stderr, "ERROR: No -input is provided\n");
        exit(1);
    }

    const char *input_file_path = *input;

    Mapped_File mf = {0};

    if (!mf_map(&mf, input_file_path)) {
        fprintf(stderr, "ERROR: could not read file %s: %s\n",
                input_file_path, strerror(errno));
        exit(1);
    }


    String_View content = sv_from_parts(mf.content_data, mf.content_size);
    if (strcmp(*mode, "m2p") == 0) {
        markup_to_program(content, stdout, *begin, *end, *comment);
    } else if (strcmp(*mode, "p2m") == 0) {
        program_to_markup(content, stdout, *begin, *end, *comment);
    } else {
        usage(stderr);
        fprintf(stderr, "ERROR: unknown mode %s\n", *mode);
        exit(1);
    }

    mf_unmap(&mf);

    return 0;
}
