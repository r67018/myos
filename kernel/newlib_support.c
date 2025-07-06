#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>

void _exit(void)
{
    while (1) __asm__("hlt");
}

caddr_t sbrk(int incr)
{
    errno = ENOMEM;
    return (caddr_t)-1;
}

int getpid(void)
{
    return 1;
}

int kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int close(int file)
{
    return -1;
}

off_t lseek(int file, off_t ptr, int dir)
{
    return 0;
}

ssize_t read(int file, void* ptr, size_t len)
{
    return 0;
}

ssize_t write(int file, const void* ptr, size_t len)
{
    return len;
}

int fstat(int file, struct stat* st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int isatty(int file)
{
    return 1;
}
