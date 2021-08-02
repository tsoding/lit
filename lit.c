#define SV_IMPLEMENTATION
// sv.h start //////////////////////////////
// Copyright 2021 Alexey Kutepov <reximkut@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SV_H_
#define SV_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    size_t count;
    const char *data;
} String_View;

#define SV(cstr_lit) sv_from_parts(cstr_lit, sizeof(cstr_lit) - 1)
#define SV_STATIC(cstr_lit)   \
    {                         \
        sizeof(cstr_lit) - 1, \
        (cstr_lit)            \
    }

#define SV_NULL sv_from_parts(NULL, 0)

// printf macros for String_View
#define SV_Fmt "%.*s"
#define SV_Arg(sv) (int) (sv).count, (sv).data
// USAGE:
//   String_View name = ...;
//   printf("Name: "SV_Fmt"\n", SV_Arg(name));

String_View sv_from_parts(const char *data, size_t count);
String_View sv_from_cstr(const char *cstr);
String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
String_View sv_take_left_while(String_View sv, bool (*predicate)(char x));
String_View sv_chop_by_delim(String_View *sv, char delim);
bool sv_try_chop_by_delim(String_View *sv, char delim, String_View *chunk);
String_View sv_chop_left(String_View *sv, size_t n);
String_View sv_chop_right(String_View *sv, size_t n);
String_View sv_chop_left_while(String_View *sv, bool (*predicate)(char x));
bool sv_index_of(String_View sv, char c, size_t *index);
bool sv_eq(String_View a, String_View b);
bool sv_starts_with(String_View sv, String_View prefix);
bool sv_ends_with(String_View sv, String_View suffix);
uint64_t sv_to_u64(String_View sv);

#endif  // SV_H_

#ifdef SV_IMPLEMENTATION

String_View sv_from_parts(const char *data, size_t count)
{
    String_View sv;
    sv.count = count;
    sv.data = data;
    return sv;
}

String_View sv_from_cstr(const char *cstr)
{
    return sv_from_parts(cstr, strlen(cstr));
}

String_View sv_trim_left(String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
        i += 1;
    }

    return sv_from_parts(sv.data + i, sv.count - i);
}

String_View sv_trim_right(String_View sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
        i += 1;
    }

    return sv_from_parts(sv.data, sv.count - i);
}

String_View sv_trim(String_View sv)
{
    return sv_trim_right(sv_trim_left(sv));
}

String_View sv_chop_left(String_View *sv, size_t n)
{
    if (n > sv->count) {
        n = sv->count;
    }

    String_View result = sv_from_parts(sv->data, n);

    sv->data  += n;
    sv->count -= n;

    return result;
}

String_View sv_chop_right(String_View *sv, size_t n)
{
    if (n > sv->count) {
        n = sv->count;
    }

    String_View result = sv_from_parts(sv->data + sv->count - n, n);

    sv->count -= n;

    return result;
}

bool sv_index_of(String_View sv, char c, size_t *index)
{
    size_t i = 0;
    while (i < sv.count && sv.data[i] != c) {
        i += 1;
    }

    if (i < sv.count) {
        if (index) {
            *index = i;
        }
        return true;
    } else {
        return false;
    }
}

bool sv_try_chop_by_delim(String_View *sv, char delim, String_View *chunk)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }

    String_View result = sv_from_parts(sv->data, i);

    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data  += i + 1;
        if (chunk) {
            *chunk = result;
        }
        return true;
    }

    return false;
}

String_View sv_chop_by_delim(String_View *sv, char delim)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim) {
        i += 1;
    }

    String_View result = sv_from_parts(sv->data, i);

    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data  += i + 1;
    } else {
        sv->count -= i;
        sv->data  += i;
    }

    return result;
}

bool sv_starts_with(String_View sv, String_View expected_prefix)
{
    if (expected_prefix.count <= sv.count) {
        String_View actual_prefix = sv_from_parts(sv.data, expected_prefix.count);
        return sv_eq(expected_prefix, actual_prefix);
    }

    return false;
}

bool sv_ends_with(String_View sv, String_View expected_suffix)
{
    if (expected_suffix.count <= sv.count) {
        String_View actual_suffix = sv_from_parts(sv.data + sv.count - expected_suffix.count, expected_suffix.count);
        return sv_eq(expected_suffix, actual_suffix);
    }

    return false;
}

bool sv_eq(String_View a, String_View b)
{
    if (a.count != b.count) {
        return false;
    } else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

uint64_t sv_to_u64(String_View sv)
{
    uint64_t result = 0;

    for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); ++i) {
        result = result * 10 + (uint64_t) sv.data[i] - '0';
    }

    return result;
}

String_View sv_chop_left_while(String_View *sv, bool (*predicate)(char x))
{
    size_t i = 0;
    while (i < sv->count && predicate(sv->data[i])) {
        i += 1;
    }
    return sv_chop_left(sv, i);
}

String_View sv_take_left_while(String_View sv, bool (*predicate)(char x))
{
    size_t i = 0;
    while (i < sv.count && predicate(sv.data[i])) {
        i += 1;
    }
    return sv_from_parts(sv.data, i);
}

#endif // SV_IMPLEMENTATION
// sv.h end ////////////////////////////////

#define FLAG_IMPLEMENTATION
// flag.h start ////////////////////////////
// flag.h -- command-line flag parsing
//
//   Inspired by Go's flag module: https://pkg.go.dev/flag
//
#ifndef FLAG_H_
#define FLAG_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

char *flag_name(void *val);
bool *flag_bool(const char *name, bool def, const char *desc);
uint64_t *flag_uint64(const char *name, uint64_t def, const char *desc);
char **flag_str(const char *name, const char *def, const char *desc);
bool flag_parse(int argc, char **argv);
int flag_rest_argc(void);
char **flag_rest_argv(void);
void flag_print_error(FILE *stream);
void flag_print_options(FILE *stream);

#endif // FLAG_H_

//////////////////////////////

#ifdef FLAG_IMPLEMENTATION

typedef enum {
    FLAG_BOOL,
    FLAG_UINT64,
    FLAG_STR,
} Flag_Type;

typedef union {
    char *as_str;
    uint64_t as_uint64;
    bool as_bool;
} Flag_Value;

typedef enum {
    FLAG_NO_ERROR = 0,
    FLAG_ERROR_UNKNOWN,
    FLAG_ERROR_NO_VALUE,
    FLAG_ERROR_INVALID_NUMBER,
    FLAG_ERROR_INTEGER_OVERFLOW,
} Flag_Error;

typedef struct {
    Flag_Type type;
    char *name;
    char *desc;
    Flag_Value val;
    Flag_Value def;
} Flag;

#ifndef FLAGS_CAP
#define FLAGS_CAP 256
#endif

typedef struct {
    Flag flags[FLAGS_CAP];
    size_t flags_count;

    Flag_Error flag_error;
    char *flag_error_name;

    int rest_argc;
    char **rest_argv;
} Flag_Context;

static Flag_Context flag_global_context;

Flag *flag_new(Flag_Type type, const char *name, const char *desc)
{
    Flag_Context *c = &flag_global_context;

    assert(c->flags_count < FLAGS_CAP);
    Flag *flag = &c->flags[c->flags_count++];
    memset(flag, 0, sizeof(*flag));
    flag->type = type;
    // NOTE: I won't touch them I promise Kappa
    flag->name = (char*) name;
    flag->desc = (char*) desc;
    return flag;
}

char *flag_name(void *val)
{
    Flag *flag = (Flag*) ((char*) val - offsetof(Flag, val));
    return flag->name;
}

bool *flag_bool(const char *name, bool def, const char *desc)
{
    Flag *flag = flag_new(FLAG_BOOL, name, desc);
    flag->def.as_bool = def;
    flag->val.as_bool = def;
    return &flag->val.as_bool;
}

uint64_t *flag_uint64(const char *name, uint64_t def, const char *desc)
{
    Flag *flag = flag_new(FLAG_UINT64, name, desc);
    flag->val.as_uint64 = def;
    flag->def.as_uint64 = def;
    return &flag->val.as_uint64;
}

char **flag_str(const char *name, const char *def, const char *desc)
{
    Flag *flag = flag_new(FLAG_STR, name, desc);
    flag->val.as_str = (char*) def;
    flag->def.as_str = (char*) def;
    return &flag->val.as_str;
}

static char *flag_shift_args(int *argc, char ***argv)
{
    assert(*argc > 0);
    char *result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
}

int flag_rest_argc(void)
{
    return flag_global_context.rest_argc;
}

char **flag_rest_argv(void)
{
    return flag_global_context.rest_argv;
}

bool flag_parse(int argc, char **argv)
{
    Flag_Context *c = &flag_global_context;

    flag_shift_args(&argc, &argv);

    while (argc > 0) {
        char *flag = flag_shift_args(&argc, &argv);

        if (*flag != '-') {
            // NOTE: pushing flag back into args
            c->rest_argc = argc + 1;
            c->rest_argv = argv - 1;
            return true;
        }

        if (strcmp(flag, "--") == 0) {
            // NOTE: but if it's the terminator we don't need to push it back
            c->rest_argc = argc;
            c->rest_argv = argv;
            return true;
        }

        // NOTE: remove the dash
        flag += 1;

        bool found = false;
        for (size_t i = 0; i < c->flags_count; ++i) {
            if (strcmp(c->flags[i].name, flag) == 0) {
                switch (c->flags[i].type) {
                case FLAG_BOOL: {
                    c->flags[i].val.as_bool = true;
                }
                break;

                case FLAG_STR: {
                    if (argc == 0) {
                        c->flag_error = FLAG_ERROR_NO_VALUE;
                        c->flag_error_name = flag;
                        return false;
                    }
                    char *arg = flag_shift_args(&argc, &argv);
                    c->flags[i].val.as_str = arg;
                }
                break;

                case FLAG_UINT64: {
                    if (argc == 0) {
                        c->flag_error = FLAG_ERROR_NO_VALUE;
                        c->flag_error_name = flag;
                        return false;
                    }
                    char *arg = flag_shift_args(&argc, &argv);

                    static_assert(sizeof(unsigned long long int) == sizeof(uint64_t), "The original author designed this for x86_64 machine with the compiler that expects unsigned long long int and uint64_t to be the same thing, so they could use strtoull() function to parse it. Please adjust this code for your case and maybe even send the patch to upstream to make it work on a wider range of environments.");
                    char *endptr;
                    unsigned long long int result = strtoull(arg, &endptr, 10);

                    if (arg == endptr || *endptr != '\0') {
                        c->flag_error = FLAG_ERROR_INVALID_NUMBER;
                        c->flag_error_name = flag;
                        return false;
                    }
                    if (result == ULLONG_MAX && errno == ERANGE) {
                        c->flag_error = FLAG_ERROR_INTEGER_OVERFLOW;
                        c->flag_error_name = flag;
                        return false;
                    }

                    c->flags[i].val.as_uint64 = result;
                }
                break;

                default: {
                    assert(0 && "unreachable");
                    exit(69);
                }
                }

                found = true;
            }
        }

        if (!found) {
            c->flag_error = FLAG_ERROR_UNKNOWN;
            c->flag_error_name = flag;
            return false;
        }
    }

    c->rest_argc = argc;
    c->rest_argv = argv;
    return true;
}

void flag_print_options(FILE *stream)
{
    Flag_Context *c = &flag_global_context;
    for (size_t i = 0; i < c->flags_count; ++i) {
        Flag *flag = &c->flags[i];

        fprintf(stream, "    -%s\n", flag->name);
        fprintf(stream, "        %s\n", flag->desc);
        switch (c->flags[i].type) {
        case FLAG_BOOL:
            if (flag->def.as_bool) {
                fprintf(stream, "        Default: %s\n", flag->def.as_bool ? "true" : "false");
            }
            break;
        case FLAG_UINT64:
            fprintf(stream, "        Default: %" PRIu64 "\n", flag->def.as_uint64);
            break;
        case FLAG_STR:
            if (flag->def.as_str) {
                fprintf(stream, "        Default: %s\n", flag->def.as_str);
            }
            break;
        default:
            assert(0 && "unreachable");
            exit(69);
        }
    }
}

void flag_print_error(FILE *stream)
{
    Flag_Context *c = &flag_global_context;
    switch (c->flag_error) {
    case FLAG_NO_ERROR:
        // NOTE: don't call flag_print_error() if flag_parse() didn't return false, okay? ._.
        fprintf(stream, "Operation Failed Successfully! Please tell the developer of this software that they don't know what they are doing! :)");
        break;
    case FLAG_ERROR_UNKNOWN:
        fprintf(stream, "ERROR: -%s: unknown flag\n", c->flag_error_name);
        break;
    case FLAG_ERROR_NO_VALUE:
        fprintf(stream, "ERROR: -%s: no value provided\n", c->flag_error_name);
        break;
    case FLAG_ERROR_INVALID_NUMBER:
        fprintf(stream, "ERROR: -%s: invalid number\n", c->flag_error_name);
        break;
    case FLAG_ERROR_INTEGER_OVERFLOW:
        fprintf(stream, "ERROR: -%s: integer overflow\n", c->flag_error_name);
        break;
    default:
        assert(0 && "unreachable");
        exit(69);
    }
}

#endif
// Copyright 2021 Alexey Kutepov <reximkut@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// flag.h end //////////////////////////////

// mf.c start //////////////////////////////
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#ifdef _WIN32
#error "TODO: Mapped_File API is not implemented for Windows"
#endif

typedef struct {
    void *content_data;
    size_t content_size;

    int fd;
    bool fd_open;
} Mapped_File;

void mf_unmap(Mapped_File *mf)
{
    if (mf->content_data != NULL) {
        munmap(mf->content_data, mf->content_size);
    }

    if (mf->fd_open) {
        close(mf->fd);
    }

    memset(mf, 0, sizeof(*mf));
}

bool mf_map(Mapped_File *mf, const char *file_path)
{
    mf_unmap(mf);

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
    mf_unmap(mf);
    return false;
}
// mf.c end ////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

void usage(FILE *stream)
{
    fprintf(stream, "Usage: lit [OPTIONS] [--] <INPUT-FILE>\n");
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

int main(int argc, char **argv)
{
    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0.");
    char **begin = flag_str("begin", "\\begin{code}", "Line that denotes the beginning of the code block in the markup language.");
    char **end = flag_str("end", "\\end{code}", "Line that denotes the end of the code block in the markup language.");
    char **comment = flag_str("comment", "//", "The inline comment of the programming language.");
    char **output = flag_str("o", NULL, "Output file path. If not provided, output to stdout.");

    if (!flag_parse(argc, argv)) {
        usage(stderr);
        flag_print_error(stderr);
        exit(1);
    }

    if (*help) {
        usage(stdout);
        exit(0);
    }

    int rest_argc = flag_rest_argc();
    char **rest_argv = flag_rest_argv();

    if (rest_argc <= 0) {
        usage(stderr);
        fprintf(stderr, "ERROR: no input file was provided\n");
        exit(1);
    }

    if (rest_argc > 1) {
        usage(stderr);
        fprintf(stderr, "ERROR: only one input file is supported right now\n");
        exit(1);
    }

    const char *input_file_path = rest_argv[0];

    Mapped_File mf = {0};

    if (!mf_map(&mf, input_file_path)) {
        fprintf(stderr, "ERROR: could not read file %s: %s\n",
                input_file_path, strerror(errno));
        exit(1);
    }

    String_View content = sv_from_parts(mf.content_data, mf.content_size);
    FILE *stream = stdout;
    if (*output) {
        stream = fopen(*output, "wb");
        if (stream == NULL) {
            fprintf(stderr, "ERROR: could not open file %s: %s\n", *output, strerror(errno));
            exit(1);
        }
    }

    markup_to_program(content, stream, *begin, *end, *comment);

    if (*output) {
        fclose(stream);
    }

    mf_unmap(&mf);

    return 0;
}
