#include "sysutil.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


void  displayProcessTimes(const char *msg)
{
    struct tms t;
    clock_t clockTime;
    static long clockTicks = 0;

    if (msg != NULL)
        printf("%s", msg);

    if (clockTicks == 0) {      /* Fetch clock ticks on first call */
        clockTicks = sysconf(_SC_CLK_TCK);
        if (clockTicks == -1)
            ERR_EXIT("sysconf");
    }

    clockTime = clock();
    if (clockTime == -1)
        ERR_EXIT("clock");

    printf("        clock() returns: %ld clocks-per-sec (%.2f secs)\n",
            (long) clockTime, (double) clockTime / CLOCKS_PER_SEC);

    if (times(&t) == -1)
        ERR_EXIT("times");
    printf("        times() yields: user CPU=%.2f; system CPU: %.2f\n",
            (double) t.tms_utime / clockTicks,
            (double) t.tms_stime / clockTicks);
}

static int lock_internal(int fd, int lock_type)
{
	int ret;
	struct flock the_lock;
	memset(&the_lock, 0, sizeof(the_lock));
	the_lock.l_type = lock_type;
	the_lock.l_whence = SEEK_SET;
	the_lock.l_start = 0;
	the_lock.l_len = 0;
	do {
		ret = fcntl(fd, F_SETLKW, &the_lock);
	}
	while (ret < 0 && errno == EINTR);

	return ret;
}

int lock_file_read(int fd)
{
	return lock_internal(fd, F_RDLCK);
}


int lock_file_write(int fd)
{
	return lock_internal(fd, F_WRLCK);
}


int unlock_file(int fd)
{
	int ret;
	struct flock the_lock;
	memset(&the_lock, 0, sizeof(the_lock));
	the_lock.l_type = F_UNLCK;
	the_lock.l_whence = SEEK_SET;
	the_lock.l_start = 0;
	the_lock.l_len = 0;

	ret = fcntl(fd, F_SETLK, &the_lock);

	return ret;
}