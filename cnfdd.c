#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

static const char * src;
static const char * dst;
static const char * cmd;

static void
die (const char * fmt, ...)
{
  va_list ap;
  fputs ("*** cnfdd: ", stderr);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fputc ('\n', stderr);
  fflush (stderr);
  exit (1);
}

#define USAGE \
  "usage: cnfdd [-h] src.cnf dst.cnf command\n"

int
main (int argc, char ** argv)
{
  int i;

  for (i = 1; i < argc; i++)
    {
      if (!strcmp (argv[i], "-h"))
	{
	  printf ("%s", USAGE);
	  exit (0);
	}

      if (argv[i][0] == '-')
	die ("invalid command line option '%s'", argv[i]);
    }

  return 0;
}
