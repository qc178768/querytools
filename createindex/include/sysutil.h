#ifndef SYSUTILH
#define SYSUTILH
#include "../include/createindex.h"

int lock_file_read(int fd);
int lock_file_write(int fd);
int unlock_file(int fd);
void displayProcessTimes(const char *msg);   /* Display 'msg' and process times */



#endif /* SYSUTILH */