#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

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
  "usage: cnfdd [-h] src dst cmd\n" \
  "\n" \
  "  src   file name of an existing CNF in DIMACS format\n" \
  "  dst   file name of generated minimized CNF\n" \
  "  cmd   command to debug (expects a CNF file as argument)\n" \
  "\n" \
  "The delta debugger copies 'src' to 'dst' and tries to remove\n" \
  "as many clauses and literals without changing the exit code\n" \
  "of 'cmd dst'.  Then unused variables are removed, as long the\n" \
  "exit code does not change.\n"

static int ** clauses;
static int size_clauses;
static int maxidx;
static int * movedto;

static void
parse (void)
{
  int i, ch, * clause, lit, sign, size_clause, count_clause, count_clauses;
  FILE * file = fopen (src, "r");

  if (!file)
    die ("can not read from '%s'", src);

SKIP:
  ch = getc (file);
  if (isspace (ch))
    goto SKIP;

  if (ch == 'c')
    {
      while ((ch = getc (file)) != '\n' && ch != EOF)
	;
      goto SKIP;
    }

  if (ch != 'p' || fscanf (file, " cnf %d %d", &maxidx, &size_clauses) != 2)
    die ("expected 'p cnf ...' header");

  movedto = malloc ((maxidx + 1) * sizeof (movedto[0]));
  for (i = 1; i <= maxidx; i++)
    movedto[i] = i;

  clauses = malloc (size_clauses * sizeof (clauses[0]));

  clause = 0;
  size_clause = count_clause = count_clauses = 0;

NEXT:

  ch = getc (file);
  if (isspace (ch))
    goto NEXT;

  if (ch == 'c')
    {
      while ((ch = getc (file)) != '\n' && ch != EOF)
	;
      goto NEXT;
    }

  if (ch == '-')
    {
      sign = -1;
      ch = getc (file);
      if (ch == EOF)
	die ("EOF after '-'");
    }
  else
    sign = 1;

  if (ch != EOF && !isdigit (ch))
    die ("invalid character %02x", ch);

  if (isdigit (ch))
    {
      lit = ch - '0';
      while (isdigit (ch = getc (file)))
	lit = 10 * lit + (ch - '0');

      lit *= sign;

      if (count_clause == size_clause)
	{
	  size_clause = size_clause ? 2 * size_clause : 1;
	  clause = realloc (clause, size_clause * sizeof (clause[0]));
	}

      clause[count_clause++] = lit;

      if (!lit)
	{
	  if (count_clauses == size_clauses)
	    die ("too many clauses");

	  clauses[count_clauses++] = clause;
	  count_clause = size_clause = 0;
	  clause = 0;
	}

      goto NEXT;
    }

  assert (ch == EOF);

  if (count_clause)
    die ("missing '0' after clause");

  if (count_clauses < size_clauses)
    die ("%d clauses missing", size_clauses - count_clauses);

  assert (!clause);
  fclose (file);
}

static void
print (void)
{
  int i, j, lit, idx, count, movedtomaxidx;
  FILE * file = fopen (dst, "w");

  if (!file)
    die ("can not write to '%s'", dst);

  movedtomaxidx = 0;
  count = 0;

  for (i = 0; i < size_clauses; i++)
    {
      if (!clauses[i])
	continue;

      j = 0;
      while ((idx = abs (clauses[i][j++])))
	{
	  if (idx == INT_MAX)
	    continue;

	  idx = movedto[idx];
	  if (idx > movedtomaxidx)
	    movedtomaxidx = idx;
	}

      count++;
    }

  fprintf (file, "p cnf %d %d\n", movedtomaxidx, count);

  for (i = 0; i < size_clauses; i++)
    {
      if (!clauses[i])
	continue;

      j = 0;
      while ((lit = clauses[i][j++]))
	{
	  if (lit == INT_MAX)
	    continue;

	  idx = abs (lit);
	  idx = movedto[idx];
	  lit = (lit < 0) ? -idx : idx;

	  fprintf (file, "%d ", lit);
	}

      fputs ("0\n", file);
    }

  fclose (file);
}

static void
reduce (void)
{
}

static void
move (void)
{
}

static void
reset (void)
{
  int i;
  for (i = 0; i < size_clauses; i++)
    free (clauses[i]);
  free (clauses);
  free (movedto);
}

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

      if (cmd)
	die ("too many command line options");
      else if (dst)
	cmd = argv[i];
      else if (src)
	dst = argv[i];
      else
	src = argv[i];
    }

  if (!src)
    die ("'src' missing");

  if (!dst)
    die ("'dst' missing");

  if (!cmd)
    die ("'cmd' missing");

  parse ();
  reduce ();
  move ();
  print ();
  reset ();

  return 0;
}
