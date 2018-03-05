//
// Created by sjw on 2018/2/28.
//

#include "hdtd.h"

int
hd_open_dev(hd_context *ctx, const char *diskname)
{
    int dev_fd;
    dev_fd = open(diskname, O_RDWR | O_BINARY);
    if (dev_fd < 0)
        hd_throw(ctx, HD_ERROR_GENERIC, "failed to open %s, errno %d", diskname, errno);
    return dev_fd;
}
void
hd_read_write_device(hd_context *ctx, int fd, bool bwrite, unsigned char *buf, uint64_t start, size_t size)
{
    ssize_t r;

    r = lseek(fd, start, SEEK_SET);
    if (r == -1) {
        hd_throw(ctx, HD_ERROR_GENERIC, "failed to seek to offset %d: %lu, errno %d", size, start, errno);
    }

    if(bwrite)
        r = write(fd, buf, size);
    else
        r = read(fd, buf, size);

    if (r < 0 || r != (ssize_t)size) {
        if (!errno)
            errno = EINVAL;	// probably too small file/device
        hd_throw(ctx, HD_ERROR_GENERIC, "failed to %s device from offset %u: %ju, errno %d",
                 bwrite?"write":"read", size, start, errno);
    }

}