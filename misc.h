#include <getopt.h>
#include <stdarg.h>

#include "config.h"

/*
 * Custom printf using the verbose_flag
 */
int my_printf(const char *message, ...);

/*
 * Send command line argument help to stdout
 */
void help();

/*
 * Parse the command line arguments
 */
int getArgv(int argc, char *argv[]);

/*
 * Converts a hex character to its integer value 
 * referrence; http://www.geekhideout.com/urlcode.shtml
 */
char from_hex(char ch);

/* 
 * Returns a url-decoded version of str
 * IMPORTANT: be sure to free() the returned string after use 
 * referrence; http://www.geekhideout.com/urlcode.shtml
 */
char *url_decode(char *str);

/*
 * Get the file extention of a given file (in string form)
 */
char *get_filename_ext(char *filename);