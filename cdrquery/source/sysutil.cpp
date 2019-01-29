#include "sysutil.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

static char daytab[2][13] = {
	{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};
/* day_of_year: 将某月某日的日期表示形式转换为某年中第几天的表示形式*/
int day_of_year(int year, int month, int day)
{
	int i, leap;
	
	leap = year%4 == 0 && year%100 != 0 || year%400 == 0;
	for (i = 1; i < month; i++)
	day += daytab[leap][i];
	return day;
}
/* month_day: 将某年中第几天的日期表示形式转换为某月某日的表示形式*/
void month_day(int year, int yearday, int *pmonth, int *pday)
{
	int i, leap;
	leap = year%4 == 0 && year%100 != 0 || year%400 ==0;
	for (i = 1; yearday > daytab[leap][i]; i++)
	yearday -= daytab[leap][i];
	*pmonth = i;
	*pday = yearday;
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