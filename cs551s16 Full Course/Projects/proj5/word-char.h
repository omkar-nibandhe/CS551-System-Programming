#ifndef _WORD_CHAR_H
#define _WORD_CHAR_H

/** type for is-word-char function which returns non-zero iff c is a
 *  word char
 */
typedef int is_word_char_f(int c);

#define IS_WORD_CHAR_F isWordChar
#define STR(s) #s
#define STRINGIFY(s) STR(s)
#define IS_WORD_CHAR_F_NAME STRINGIFY(IS_WORD_CHAR_F)

/** Return non-zero iff c is a word char */
int WORD_CHAR_F(int c);

#endif //ifndef _WORD_CHAR_H
