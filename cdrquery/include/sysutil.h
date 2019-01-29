#ifndef _SYS_UTIL_H_
#define _SYS_UTIL_H_

int day_of_year(int year, int month, int day);
void month_day(int year, int yearday, int *pmonth, int *pda);
int lock_file_read(int fd);
int lock_file_write(int fd);
int unlock_file(int fd);




#endif /* _SYS_UTIL_H_ */