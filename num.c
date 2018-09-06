#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/*
 * A simple number base conversion utility to test my understanding of UTF-[8,16]
 * Lacks input validation
 * base 2,8,10,16 are handled by strtol()
 *
 * dwf -- Wed Jul 27 00:03:21 MST 2017
 *
 * dwf -- Tue Aug 28 20:32:39 MDT 2018
 *   Added -c option
 */

/*
 * Local functions
 */
int     validateUTF8 (unsigned char *);
int     UTF8CharSize (unsigned char *);
int     main         (int, char *[]  );
void    usage        (const char *   );

// returns 0 if the utf8 char is invalid
// returns a number [1,4] representing the number of bytes in the valid utf8 char
int
validateUTF8(unsigned char *ptr) {
    int n = UTF8CharSize(ptr);

    for (int i = 1; i < n; i++) {
        if ((ptr[i] & 0xc0) != 0x80) return 0;
    }
    return n;
}

// returns the number of bytes representing the utf8 char [1,4]
int
UTF8CharSize(unsigned char *ptr) {
    int num = 1;

    if (*ptr >= 0xf0) {
        num = 4;
    } else if (*ptr >= 0xe0) {
        num = 3;
    } else if (*ptr >= 0xc0) {
        num = 2;
    }

    return num;
}


int
main(int argc, char *argv[]) {
    char *progname = argv[0];
    static struct option longopts[] = {
        { "help" , no_argument       , NULL , 'h' } ,
        { "dec"  , required_argument , NULL , 'd' } ,
        { "bin"  , required_argument , NULL , 'b' } ,
        { "hex"  , required_argument , NULL , 'x' } ,
        { "oct"  , required_argument , NULL , 'o' } ,
        { "u8"   , required_argument , NULL , 'u' } ,
        { "u16"  , required_argument , NULL , 'U' } ,
        { "ch"   , required_argument , NULL , 'c' } ,
        { NULL   , 0                 , NULL ,  0  }
    };

    /*
     * each option branch parses its form of valid input and sets
     * the value of integer 'n', followed by displaying all formats
     */
    int opt = 0, outputResults = 0;
    while ((opt = getopt_long_only(argc, argv, "hd:b:x:u:U:c:", longopts, NULL)) != -1) {
        int   n   = 0;    // assigned by each option then output in all forms
        char *ptr = NULL; // optarg

        switch (opt) {
            case 0: break;

            case 'd':
            case 'b':
            case 'x':
            case 'o':
            case 'u':
            case 'U':
            case 'c':
                ptr = optarg; break;

            case 'h':
            default:
                usage(progname);
                exit(0);
        }

        //printf("opt is %c ptr is [%s]\n", opt, ptr);

        if (opt == 'c') { // Character input
            int a, b, c, d;
            int nbytes = validateUTF8((unsigned char *)ptr);
            
            switch (nbytes) {
                case 0:
                    fprintf(stderr, "Malformed UTF-8 input\n");
                    exit(0);
                case 1:
                    a = (int)ptr[0];
                    n = a;
                    break;
                case 2:
                    a = (int)ptr[0];
                    b = (int)ptr[1];
                    n = ((a & 0x1f) << 6) | (b & 0x3f);
                    break;
                case 3:
                    a = (int)ptr[0];
                    b = (int)ptr[1];
                    c = (int)ptr[2];
                    n = ((a & 0xf) << 12) | ((b & 0x3f) << 6) | (c & 0x3f);
                    break;
                case 4:
                    a = (int)ptr[0];
                    b = (int)ptr[1];
                    c = (int)ptr[2];
                    d = (int)ptr[3];
                    n = ((a & 0x7) << 18) | ((b & 0x3f) << 12) | ((c & 0x3f) << 6) | (d & 0x3f);
                    break;
                default:
                    fprintf(stderr, "Unknown error?\n");
                    exit(0);
            }
        } else if (opt == 'U') { // UTF-16 input
            int len = strlen(ptr);

            if (len <= 4) {
                n = (int)strtol(ptr, NULL, 16);
            } else if (len > 4 && len < 9) {
                char a[5], b[5];

                // break the input arg into its 2 hex components
                // 4 chars into "b", 4 chars into "a" (note, "a" can be < 4)
                strncpy(a, ptr, len - 4);
                a[len - 4] = '\0';

                strcpy(b, ptr + len - 4);

                // convert hex representations to integers and undo the encoding
                // described at: https://en.wikipedia.org/wiki/UTF-16
                //
                // Wikipedia Summary:
                // -----------------
                // Code points from the other planes (called Supplementary Planes) are encoded
                // as two 16-bit code units called surrogate pairs, by the following scheme:
                // 0x010000 is subtracted from the code point, leaving a 20-bit number in the
                // range 0x000000..0x0FFFFF.
                // The top ten bits (a number in the range 0x0000..0x03FF) are added to 0xD800
                // to give the first 16-bit code unit or high surrogate, which will be in the
                // range 0xD800..0xDBFF.
                // The low ten bits (also in the range 0x0000..0x03FF) are added to 0xDC00 to
                // give the second 16-bit code unit or low surrogate, which will be in the
                // range 0xDC00..0xDFFF.
                int h = (int)strtol(a, NULL, 16) - 0xd800;
                int l = (int)strtol(b, NULL, 16) - 0xdc00;
                n = 0x10000 + (h<<10 | l);
            } else {
                fprintf(stderr, "u16 len error\n");
                exit(1);
            }
        } else if (opt == 'u') { // UTF-8 input
            int len = strlen(ptr);
            int a, b, c, d;
            char ch[3];

            ch[2] = '\0';
            switch (len) {
                case 2:
                    a = (int)strtol(ptr, NULL, 16);
                    n = a;
                    break;
                case 4:
                    ch[0] = *ptr;
                    ch[1] = *++ptr;
                    a = (int)strtol(ch, NULL, 16);
                    b = (int)strtol(++ptr, NULL, 16);
                    n = ((a & 0x1f) << 6) | (b & 0x3f);
                    break;
                case 6:
                    ch[0] = *ptr;
                    ch[1] = *++ptr;
                    a = (int)strtol(ch, NULL, 16);
                    ch[0] = *++ptr;
                    ch[1] = *++ptr;
                    b = (int)strtol(ch, NULL, 16);
                    c = (int)strtol(++ptr, NULL, 16);
                    n = ((a & 0xf) << 12) | ((b & 0x3f) << 6) | (c & 0x3f);
                    break;
                case 8:
                    ch[0] = *ptr;
                    ch[1] = *++ptr;
                    a = (int)strtol(ch, NULL, 16);
                    ch[0] = *++ptr;
                    ch[1] = *++ptr;
                    b = (int)strtol(ch, NULL, 16);
                    ch[0] = *++ptr;
                    ch[1] = *++ptr;
                    c = (int)strtol(ch, NULL, 16);
                    d = (int)strtol(++ptr, NULL, 16);
                    n = ((a & 0x7) << 18) | ((b & 0x3f) << 12) | ((c & 0x3f) << 6) | (d & 0x3f);
                    break;
                default:
                    fprintf(stderr, "UTF-8 must be entered in 2-byte hex format: (ex. num uf09f8dba)\n");
                    exit(0);
            }
        } else { // Binary, Octal, Hex, Decimal input
            int base = 0;
            char buf[BUFSIZ];
            buf[0] = '\0';

            // Base-2 begins with [b]
            if (opt == 'b') {
                base = 2;
            } else if (opt == 'o') {
                if (*ptr != '0') {
                    strcpy(buf, "0");
                }
            } else if (opt == 'x') {
                if (strncmp(ptr, "0x", 2)) {
                    strcpy(buf, "0x");
                }
            }

            // ptr starts with "0" for Octal, "0x" for Hex
            strcat(buf, ptr);

            // these forms aren't interesting and are handled by strtol() which
            // internally converts Base-[8,10,16] with "base==0" and correct form of input
            n = (int)strtol(buf, NULL, base);
        }

        /* ********************************************************* */

        // Output Base-[10,8,16]
        printf("(Dec) %d\t(Oct) %o\t(Hex) %x", n, n, n);

        // Output Base-2
        int bits = sizeof(n)*8;
        int skippedLeadingZeros = 0;
        for (int j = bits-1; j >= 0; --j) {
            if (!skippedLeadingZeros) {
                if (n & (1<<j)) {
                    printf("\t(Bin[%d]) 1", j+1);
                    skippedLeadingZeros = 1;
                } else if (j == 0) {
                    printf("\t(Bin[1]) 0");
                }
            } else {
                printf("%d", n & (1<<j) ? 1 : 0);
            }
        }

        // Output UTF-8
        //
        //  Number  | Bits for   |    First   |    Last    |          |          |          |          |
        // of bytes | code point | code point | code point |  Byte 1  |  Byte 2  |  Byte 3  |  Byte 4  |
        // ---------------------------------------------------------------------------------------------
        //     1    |     7      |   U+0000   |  U+007F    | 0xxxxxxx |          |          |          |
        //     2    |    11      |   U+0080   |  U+07FF    | 110xxxxx | 10xxxxxx |          |          |
        //     3    |    16      |   U+0800   |  U+FFFF    | 1110xxxx | 10xxxxxx | 10xxxxxx |          |
        //     4    |    21      |   U+10000  |  U+10FFFF  | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |

        printf("\t(UTF-8) ");
        if (n >= 0 && n <= 0x7f) {
            printf("%02x", n);
        } else if (n >= 0x80 && n <= 0x7ff) {
            printf("%02x %02x", n>>6 | 0xc0, (n & 0x3f) | 0x80);
        } else if (n >= 0x800 && n <= 0xffff) {
            printf("%02x %02x %02x", n>>12 | 0xe0, (n>>6 & 0x3f) | 0x80, (n & 0x3f) | 0x80);
        } else if (n >= 0x10000 && n <= 0x10ffff) {
            printf("%02x %02x %02x %02x", n>>18 | 0xf0 ,(n>>12 & 0x3f) | 0x80 ,(n>>6 & 0x3f) | 0x80, (n & 0x3f) | 0x80);
        }

        printf("\t(UTF-8 Char) "); // may or may not be visble on output btw
        if (n >= 0 && n <= 0x7f) {
            printf("%c", n);
        } else if (n >= 0x80 && n <= 0x7ff) {
            printf("%c%c", n>>6 | 0xc0, (n & 0x3f) | 0x80);
        } else if (n >= 0x800 && n <= 0xffff) {
            printf("%c%c%c", n>>12 | 0xe0, (n>>6 & 0x3f) | 0x80, (n & 0x3f) | 0x80);
        } else if (n >= 0x10000 && n <= 0x10ffff) {
            printf("%c%c%c%c", n>>18 | 0xf0 ,(n>>12 & 0x3f) | 0x80 ,(n>>6 & 0x3f) | 0x80, (n & 0x3f) | 0x80);
        }

        // Output UTF-16
        if (n >= 0x10000 && n <= 0x10ffff) {
            n -= 0x10000;
            printf("\t(UTF-16) %04x %04x", (n>>10) + 0xd800, (n & 0x3ff) + 0xdc00);
        }

        printf("\n");
        outputResults = 1;
    }

    if (!outputResults) {
        usage(progname);
        exit(0);
    }

}



void
usage(const char *prog) {
    fprintf(stderr, "Usage: num -[dec|oct|hex|bin|u8|u16|ch] #\n");
    fprintf(stderr, "Examples:\n");

    fprintf(stderr, "  Binary:\n");
    fprintf(stderr, "\tnum -[b|bin] 1001\n");
    fprintf(stderr, "\t(Dec) 9  (Oct) 11  (Hex) 9  (Bin[4]) 1001  (UTF-8) 09\n\n");

    fprintf(stderr, "  Octal:\n");
    fprintf(stderr, "\tnum -[o|oct] 16\n");
    fprintf(stderr, "\t(Dec) 14  (Oct) 16  (Hex) e  (Bin[4]) 1110  (UTF-8) 0e\n\n");

    fprintf(stderr, "  Decimal:\n");
    fprintf(stderr, "\tnum -[d|dec] 10\n");
    fprintf(stderr, "\t(Dec) 10  (Oct) 12  (Hex) a  (Bin[4]) 1010  (UTF-8) 0a\n\n");

    fprintf(stderr, "  Hex:\n");
    fprintf(stderr, "\tnum -[x|hex] b\n");
    fprintf(stderr, "\t(Dec) 11  (Oct) 13  (Hex) b  (Bin[4]) 1011  (UTF-8) 0b\n\n");

    fprintf(stderr, "  UTF-8:\n");
    fprintf(stderr, "\tnum -[u|u8] e0a080\n");
    fprintf(stderr, "\t(Dec) 2048  (Oct) 4000  (Hex) 800  (Bin[12]) 100000000000  (UTF-8) e0 a0 80\n\n");

    fprintf(stderr, "  UTF-16:\n");
    fprintf(stderr, "\tnum -[U|u16] d83cdf7a\n");
    fprintf(stderr, "\t(Dec) 127866  (Oct) 371572  (Hex) 1f37a  (Bin[17]) 11111001101111010  (UTF-8) f0 9f 8d ba  (UTF-16) d83c df7a\n");
}

