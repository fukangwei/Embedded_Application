#ifndef _UTIL_H
#define _UTIL_H
#include "types.h"

uint16 ATOI ( char *str, uint16 base );
uint32 ATOI32 ( char *str, uint16 base );
void itoa ( uint16 n, uint8 *str, uint8 len );
int ValidATOI ( char *str, int base, int *ret );
char C2D ( u_char c );
uint16 swaps ( uint16 i );
uint32 swapl ( uint32 l );
void replacetochar ( char *str, char oldchar, char newchar );
void mid ( int8 *src, int8 *s1, int8 *s2, int8 *sub );
void inet_addr_ ( unsigned char *addr, unsigned char *ip );
#endif