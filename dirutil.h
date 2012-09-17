#include <sys/stat.h>

int mkpath(const char*, mode_t);
static int do_mkdir(const char*, mode_t);

