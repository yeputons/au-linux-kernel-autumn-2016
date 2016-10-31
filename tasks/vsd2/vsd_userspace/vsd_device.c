#include <sys/errno.h>
#include <fcntl.h>

#include "vsd_ioctl.h"
#include "vsd_device.h"

static int fd;

int vsd_init()
{
    if (fd) {
        errno = -EINVAL;
        return -1;
    }
    fd = open("/dev/vsd", O_RDWR);
    return (fd < 0) ? fd : 0;
}

int vsd_deinit()
{
    return close(fd);
}

int vsd_get_size(size_t *out_size)
{
    struct vsd_ioctl_get_size_arg arg;
    if (ioctl(fd, VSD_IOCTL_GET_SIZE, &arg)) {
        return errno;
    }
    *out_size = arg.size;
    return 0;
}

int vsd_set_size(size_t size)
{
    struct vsd_ioctl_set_size_arg arg;
    arg.size = size;
    return ioctl(fd, VSD_IOCTL_SET_SIZE, &arg);
}

ssize_t vsd_read(char* dst, off_t offset, size_t size)
{
    if (lseek(fd, offset, SEEK_SET) < 0) {
        return -1;
    }
    return read(fd, dst, size);
}

ssize_t vsd_write(const char* src, off_t offset, size_t size)
{
    if (lseek(fd, offset, SEEK_SET) < 0) {
        return -1;
    }
    return write(fd, src, size);
}

void* vsd_mmap(size_t offset)
{
    size_t size, pagesize;
    pagesize = getpagesize();
    if (vsd_get_size(&size)) {
        return NULL;
    }
    size = (size - offset) & ~(pagesize - 1);
    void* result = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if (result == MAP_FAILED) {
        return NULL;
    } else {
        return result;
    }
}

int vsd_munmap(void* addr, size_t offset)
{
    return munmap(addr, offset);
}
