#pragma once

extern char _ctype[];

#define CT_UP	0x01	/* upper case */
#define CT_LOW	0x02	/* lower case */
#define CT_DIG	0x04	/* digit */
#define CT_CTL	0x08	/* control */
#define CT_PUN	0x10	/* punctuation */
#define CT_WHT	0x20	/* white space (space/cr/lf/tab) */
#define CT_HEX	0x40	/* hex digit */
#define CT_SP	0x80	/* hard space (0x20) */
 
#define isAlnum(c)	((_ctype + 1)[(unsigned)(c)] & (CT_UP | CT_LOW | CT_DIG))
#define isAlpha(c)	((_ctype + 1)[(unsigned)(c)] & (CT_UP | CT_LOW))
#define isCntrl(c)	((_ctype + 1)[(unsigned)(c)] & (CT_CTL))
#define isDigit(c)	((_ctype + 1)[(unsigned)(c)] & (CT_DIG))
#define isGraph(c)	((_ctype + 1)[(unsigned)(c)] & (CT_PUN | CT_UP | CT_LOW | CT_DIG))
#define isLower(c)	((_ctype + 1)[(unsigned)(c)] & (CT_LOW))
#define isPrint(c)	((_ctype + 1)[(unsigned)(c)] & (CT_PUN | CT_UP | CT_LOW | CT_DIG | CT_SP))
#define isPunct(c)	((_ctype + 1)[(unsigned)(c)] & (CT_PUN))
#define isSpace(c)	((_ctype + 1)[(unsigned)(c)] & (CT_WHT))
#define isUpper(c)	((_ctype + 1)[(unsigned)(c)] & (CT_UP))
#define isXdigit(c)	((_ctype + 1)[(unsigned)(c)] & (CT_DIG | CT_HEX))
#define isAscii(c)	((unsigned)(c) <= 0x7F)
#define toAscii(c)	((unsigned)(c) & 0x7F)
#define toLower(c)	(isupper(c) ? c + 'a' - 'A' : c)
#define toUpper(c)	(islower(c) ? c + 'A' - 'a' : c)
