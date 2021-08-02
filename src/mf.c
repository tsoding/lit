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

