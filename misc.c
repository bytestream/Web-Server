#include "misc.h"

// Global variables
int verbose_flag;
unsigned int port;

/*
 * Custom printf using the verbose_flag
 */
int my_printf(const char *message, ...) {
	if (verbose_flag) {
		va_list arg;
		int len = 0;
		va_start(arg, message);
		len = vprintf(message, arg);
		va_end(arg);
		fflush(stdout);
		return len;
	} else {
		return 0;
	}
}

/*
 * Send command line argument help to stdout
 */
void help() {
        puts("Usage:");
        puts("\t./server [-p port] [-v]");
        puts("\t[-p port] the listening/connect to port");
        puts("\t[-v] enable output");
}

/*
 * Parse the command line arguments
 */
int getArgv(int argc, char *argv[]) {
	int c;
	while (1) {
		static struct option options[] = {
			{ "verbose", no_argument, 	&verbose_flag, 1 },
			{ "port", required_argument, 	0, 'p' },
			{ 0, 0, 0, 0 }
		};
		int option_index = 0;
		c = getopt_long(argc, argv, "vp:", options, &option_index);

		if (c == -1)
			break;
		switch (c) {
			case 'v':
				verbose_flag = 1;
				break;
			case 'p':
				port = atoi(optarg);
				if (port < 1 || port > 65556) {
					help();
					return -1;
				}
				break;
			case '?':
				break;
			default:
				help();
				return -1;
		}
	}

	if (optind < argc) {
		while (optind < argc);
	}

	return 0;
}

/*
 * Converts a hex character to its integer value 
 * referrence; http://www.geekhideout.com/urlcode.shtml
 */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* 
 * Returns a url-decoded version of str
 * IMPORTANT: be sure to free() the returned string after use 
 * referrence; http://www.geekhideout.com/urlcode.shtml
 */
char *url_decode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') { 
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

/*
 * Get the file extention of a given file (in string form)
 */
char *get_filename_ext(char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}