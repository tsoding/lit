#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define SV_IMPLEMENTATION
#include "./sv.h"

#define FLAG_IMPLEMENTATION
#include "./flag.h"

typedef struct {
    void *content_data;
    size_t content_size;

    int fd;
    bool fd_open;
} Mapped_File;

void unmap_file(Mapped_File *mf)
{
    if (mf->content_data != NULL) {
        munmap(mf->content_data, mf->content_size);
    }

    if (mf->fd_open) {
        close(mf->fd);
    }

    memset(mf, 0, sizeof(*mf));
}

bool map_file(Mapped_File *mf, const char *file_path)
{
    unmap_file(mf);

    mf->fd = open(file_path, O_RDONLY);
    if (mf->fd < 0) {
        goto error;
    }
    mf->fd_open = true;

    struct stat statbuf = {0};
    if (fstat(mf->fd, &statbuf) < 0) {
        goto error;
    }

    mf->content_size = statbuf.st_size;
    mf->content_data = mmap(NULL, mf->content_size, PROT_READ, MAP_PRIVATE, mf->fd, 0);
    if (mf->content_data == NULL) {
        goto error;
    }

    return true;
error:
    unmap_file(mf);
    return false;
}

void usage(FILE *stream)
{
    fprintf(stream, "Usage: lit [OPTIONS]\n");
    flag_print_options(stream);
}

int main(int argc, char **argv)
{
    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0");
    char **input = flag_str("input", NULL, "Path to the input file");
    char **begin = flag_str("begin", "\\begin{code}", "Line that denotes the beginning of the code block in the markup language");
    char **end = flag_str("end", "\\end{code}", "Line that denotes the end of the code block in the markup language");
    char **comment = flag_str("comment", "//", "The inline comment of the programming language");

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
        fprintf(stderr, "ERROR: No input file is provided\n");
        exit(1);
    }

    const char *input_file_path = *input;

    Mapped_File mf = {0};

    if (!map_file(&mf, input_file_path)) {
        fprintf(stderr, "ERROR: could not read file %s: %s\n",
                input_file_path, strerror(errno));
        exit(1);
    }

    bool code_mode = false;

    String_View content = sv_from_parts(mf.content_data, mf.content_size);
    while (content.count > 0) {
        String_View line = sv_chop_by_delim(&content, '\n');

        if (code_mode) {
            if (sv_eq(sv_trim(line), sv_from_cstr(*end))) {
                printf("%s " SV_Fmt"\n", *comment, SV_Arg(line));
                code_mode = false;
            } else {
                printf(SV_Fmt"\n", SV_Arg(line));
            }
        } else {
            printf("%s " SV_Fmt"\n", *comment, SV_Arg(line));
            if (sv_eq(sv_trim(line), sv_from_cstr(*begin))) {
                code_mode = true;
            }
        }
    }

    unmap_file(&mf);

    return 0;
}
