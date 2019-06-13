#include "libc.h"

char *strcpy(char *dest, const char *src) {
    char *save = dest;
    while ((*dest++ = *src++));
    return save;	
}

char *strncpy(char *dest, const char *src, int n) {
    while (n-- && (*dest++ = *src++));
    return dest;
}

int strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2; s1++, s2++)
  	if (*s1 == '\0') return 0;
    return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1);
}

int strlen(const char *s) {
    const char *p = s;
    while (*s) ++s;
    return s - p;
}

int strnlen (const char *s, uint maxlen) {
  const char *e = s;
  uint n;

  for (n = 0; *e && n++ < maxlen; e++);
  return n;
}

int strncmp(const char *s1, const char *s2, uint n)
{
    for ( ; n > 0; s1++, s2++, --n)
    if (*s1 != *s2)
        return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1);
    else if (*s1 == '\0')
        return 0;
    return 0;
}

// Copy len bytes from src to dest.
void memcpy(void *dest, const void *src, uint len)
{
    const uint8 *sp = (const uint8 *)src;
    uint8 *dp = (uint8 *)dest;
    for(; len != 0; len--) *dp++ = *sp++;
}

// Write len copies of val into dest.
void memset(void *dest, uint8 val, uint len)
{
    uint8 *temp = (uint8 *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

void lmemcpy(uint *dest, const uint *src, uint len) {
    for (; len != 0; len--) *dest++ = *src++;
}

void lmemset(uint *dest, uint val, uint len) {
    for (; len != 0; len--) *dest++ = val;
}

void clear_pages(void *dest, int nb_pages) {
    uint64 *ptr = (uint64*)dest;

    for (int i=0; i<nb_pages * 4096 / 8; i++) {
        *ptr = 0x0;
        ptr++;
    }
}

int atoi(char *str) {
    int res = 0; // Initialize result
  
    // Iterate through all characters of input string and
    // update result
    for (int i = 0; str[i] != '\0'; ++i)
        res = res*10 + str[i] - '0';
  
    // return result.
    return res;
}

void itoa(int nb, char *str) {
    if (nb < 0) {
        *str++ = '-';
        nb = -nb;
    }
    uint nb_ref = 1000000000;
    uint leading_zero = 1;
    uint digit;

    for (int i=0; i<=9; i++) {
        if (nb >= nb_ref) {
            digit = nb / nb_ref;
            *str++ = '0' + digit;
            nb -= nb_ref * digit;

            leading_zero = 0;
        } else {
            if (!leading_zero) *str++ = '0';
        }
        nb_ref /= 10;
    }

    if (leading_zero) *str++ = '0';

    *str = 0;
}

void itoa_right(int nb, char *number) {
    uint negative = 0;
    number[11] = 0;
    number[0] = ' ';

    if (nb < 0) {
        negative = 1;
        nb = -nb;
    }
    uint nb_ref = 1000000000;
    uint leading_zero = 1;
    uint digit;

    for (int i=0; i<=9; i++) {
        if (nb >= nb_ref) {
            digit = nb / nb_ref;
            number[i+1] = '0' + digit;
            nb -= nb_ref * digit;

            if (!leading_zero && negative) number[i] = '-';
            leading_zero = 0;
        } else {
            if (!leading_zero) number[i+1] = '0';
            else number[i+1] = ' ';
        }
        nb_ref /= 10;
    }

    if (leading_zero) number[10] = '0';
}

char key[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void _itoa_hex(uint64 nb, int nb_bytes, unsigned char *str) {

    unsigned char *addr = (unsigned char *)&nb + (nb_bytes -1);

    for (int i=0; i<nb_bytes; i++) {
        unsigned char b = *addr--;
        int u = ((b & 0xf0) >> 4);
        int l = (b & 0x0f);
        *str++ = key[u];
        *str++ = key[l];
    }

    *str = 0;
}

void itoa_hex(uint nb, unsigned char *str) {
    _itoa_hex(nb, 4, str);
}

void itoa_hex_64(uint64 nb, unsigned char *str) {
    _itoa_hex(nb, 8, str);
}

void itoa_hex_0x(uint nb, unsigned char *str) {
    *str++ = '0';
    *str++ = 'x';
    _itoa_hex(nb, 4, str);
}

void itoa_hex_0x64(uint64 nb, unsigned char *str) {
    *str++ = '0';
    *str++ = 'x';
    _itoa_hex(nb, 8, str);
}

void ctoa_hex(char c, unsigned char *str) {
    _itoa_hex((uint)c, 1, str);
}

void ctoa_hex_0x(char c, unsigned char *str) {
    *str++ = '0';
    *str++ = 'x';
    _itoa_hex((uint)c, 1, str);
}

uint rand(int nb) // RAND_MAX assumed to be 32767
{
    uint rand_next = (nb + 5) * 1103515245 + 12345;
    return (unsigned int)(rand_next / 65536) % 32768;
}

char ascii[256] = {
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.',
    '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'
};

void crash() {
    unsigned int r;
    r=*((volatile unsigned int*)0xFFFFFFFFFF000000);
    // make gcc happy about unused variables :-)
    r++;
}

/////////////////////////////////////////
// Access of packed structures attributes
/////////////////////////////////////////

uint8 *get_ptr(void *mem, uint offset) {
    return (uint8*)((uint8*)mem + offset);
}

uint8 get_uint8(void *mem, uint offset) {
    uint64 addr = (uint64)mem + offset;
    uint64 addr_aligned = addr & 0xFFFFFFFFFFFFFFF8;
    uint64 nb = *(uint64*)addr_aligned;
    nb = nb >> ((addr & 0x7) * 8);
    return (uint8)nb; // *((uint8*)&nb);
}

uint16 get_uint16(void *mem, uint offset) {
    uint64 addr = (uint64)mem + offset;
    uint64 addr_aligned = addr & 0xFFFFFFFFFFFFFFF8;
    uint64 nb = *(uint64*)addr_aligned;
    nb = nb >> ((addr & 0x7) * 8);
    return (uint16)nb; // *((uint16*)&nb);
}

uint32 get_uint32(void *mem, uint offset) {
    uint64 addr = (uint64)mem + offset;
    uint64 addr_aligned = addr & 0xFFFFFFFFFFFFFFF8;
    uint64 nb = *(uint64*)addr_aligned;
    nb = nb >> ((addr & 0x7) * 8);
    return (uint32)nb; // *((uint32*)&nb);
}

/////////////////////////////////////////
// Synchronization
/////////////////////////////////////////

void wait_msec(unsigned int n)
{
    register unsigned long f, t, r;
    // get the current counter frequency
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    // calculate expire value for counter
    t+=((f/1000)*n)/1000;
    do{asm volatile ("mrs %0, cntpct_el0" : "=r"(r));}while(r<t);
}

void wait_cycles(unsigned int n)
{
    if(n) while(n--) { asm volatile("nop"); }
}

uint get_timer() {
    register unsigned long t;
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    return t;
}

volatile uint core[4];

void init_sync_cores() {
	core[0] = 0;
	core[1] = 0;
	core[2] = 0;
	core[3] = 0;
}

void core_complete(uint i) {
    if (i > 3) return;
    core[i] = 1;
}

void sync_cores() {
    while (1) {
        if (core[0] != 0 &&
            core[1] != 0 &&
            core[2] != 0 &&
            core[3] != 0) return;

        wait_msec(8000);
    }
}
