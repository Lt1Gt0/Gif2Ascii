#include "errorhandler.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace ErrorHandler
{
    void err_n_die(const char* fmt, ...)
    {
        int errnoSave;
        va_list ap;

        errnoSave = errno;
        va_start(ap, fmt);
        vfprintf(stdout, fmt, ap);
        fprintf(stdout, "\n");
        fflush(stdout);

        if (errnoSave != 0) {
            fprintf(stdout, "(errno = %d) : %s\n", errnoSave, strerror(errnoSave));
            fprintf(stdout, "\n");
            fflush(stdout);
        }

        va_end(ap);
        exit(1);
    }
}