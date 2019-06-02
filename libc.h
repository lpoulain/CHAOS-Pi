#ifndef __LIBC_H
#define __LIBC_H

typedef unsigned char  uint8_t;
typedef unsigned int   uint;
typedef unsigned int   uint32;
typedef unsigned int   uint32_t;
typedef unsigned int   size_t;
typedef          int   sint32;
typedef          int   int32_t;
typedef          int   int32;
typedef unsigned short uint16;
typedef          short sint16;
typedef unsigned char  uint8;
typedef          char  sint8;
typedef unsigned long long uint64;
typedef unsigned long long uint64_t;
typedef 		long long int64;

extern void delay ( unsigned long);
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );
extern int get_el ( void );
extern char ascii[256];
extern void *get_sp();

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, int n);
int strcmp(const char *s1, const char *s2);
int strlen(const char *s);
int strnlen (const char *s, uint maxlen);
int strncmp(const char *s1, const char *s2, uint n);

uint atoi_hex(char *str);
int atoi(char *str);
void itoa(int, char*);
void itoa_right(int nb, char *number);
void itoa_hex(uint64 nb, unsigned char *str);
void itoa_hex_0x(uint nb, unsigned char *str);
void itoa_hex_0x64(uint64 nb, unsigned char *str);
void ctoa_hex(char c, unsigned char *str);
void ctoa_hex_0x(char c, unsigned char *str);

uint umin(uint nb1, uint nb2);
uint umax(uint nb1, uint nb2);
//int min(int nb1, int nb2);
//int max(int nb1, int nb2);
uint rand(int nb);

void memcpy(void *dest, const void *src, uint len);
void memset(void *dest, uint8 val, uint len);
void lmemcpy(uint *dest, const uint *src, uint len);
void lmemset(uint *dest, uint val, uint len);
void crash();

void EnableInterrupts();

uint8_t CoreExecute (uint8_t coreNum, void (*func) (void));

void wait_msec(unsigned int n);
uint get_timer();
void init_sync_cores();
void core_complete(uint i);
void sync_cores();

#endif
