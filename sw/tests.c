/* Stage 3: self-checking C test suite for the RV32I pipeline.
 * No multiply/divide/modulo is used (RV32I has none). Each test prints
 * PASS/FAIL; main() returns the number of failures (shown as EXIT code). */
#ifdef HOST
#include <stdio.h>
static void putch(char c) { putchar(c); }
#define LEDS_SET(v) ((void)(v))
#else
#define CONSOLE (*(volatile unsigned int *)0xFFFFFFF4u)
#define LEDS    (*(volatile unsigned int *)0xFFFFFFF0u)
#define LEDS_SET(v) (LEDS = (v))
static void putch(char c) { CONSOLE = (unsigned char)c; }
#endif
#define NOINL __attribute__((noinline))

static void puts_(const char *s) { while (*s) putch(*s++); }

static void print_hex(unsigned v)
{
    for (int i = 28; i >= 0; i -= 4) {
        unsigned n = (v >> i) & 15u;
        putch(n < 10 ? (char)('0' + n) : (char)('a' + n - 10));
    }
}
static void print_uint(unsigned v)
{
    static const unsigned p10[] = {1000000000u,100000000u,10000000u,1000000u,
                                   100000u,10000u,1000u,100u,10u,1u};
    int started = 0;
    for (int i = 0; i < 10; i++) {
        char d = '0';
        while (v >= p10[i]) { v -= p10[i]; d++; }
        if (d != '0' || started || i == 9) { putch(d); started = 1; }
    }
}

static int fails = 0;
static void check(const char *name, unsigned got, unsigned exp)
{
    if (got == exp) { puts_("PASS "); puts_(name); putch('\n'); }
    else {
        fails++;
        puts_("FAIL "); puts_(name);
        puts_(" got 0x"); print_hex(got);
        puts_(" exp 0x"); print_hex(exp); putch('\n');
    }
}

/* ---------- 1. ALU ---------- */
static void t_alu(void)
{
    volatile unsigned a = 0x12345678u, b = 0x0F0F0F0Fu, sh = 8;
    volatile int n = (int)0x87654321u, s = -5;
    check("add",  a + b, 0x21436587u);
    check("sub",  a - b, 0x03254769u);
    check("and",  a & b, 0x02040608u);
    check("or",   a | b, 0x1F3F5F7Fu);
    check("xor",  a ^ b, 0x1D3B5977u);
    check("slli", a << 4, 0x23456780u);
    check("srli", a >> 4, 0x01234567u);
    check("srai", (unsigned)(n >> 4), 0xF8765432u);
    check("sll",  a << sh, 0x34567800u);
    check("srl",  a >> sh, 0x00123456u);
    check("sra",  (unsigned)(n >> sh), 0xFF876543u);
    check("slt",  (unsigned)(s < 3), 1);
    check("sltu", (unsigned)((unsigned)s < 3u), 0);
    check("lui-imm", 0xABCDE000u + 0x123u, 0xABCDE123u);
}

/* ---------- 2. byte / halfword memory ---------- */
static unsigned int buf[2];
static void t_mem(void)
{
    volatile unsigned int   *pw = buf;
    volatile unsigned char  *pb = (volatile unsigned char  *)buf;
    volatile unsigned short *ph = (volatile unsigned short *)buf;
    pw[0] = 0x80FF7F01u; pw[1] = 0;
    check("lbu0", pb[0], 0x01);
    check("lbu1", pb[1], 0x7F);
    check("lbu2", pb[2], 0xFF);
    check("lb-sign", (unsigned)(int)(signed char)pb[3], 0xFFFFFF80u);
    check("lhu0", ph[0], 0x7F01);
    check("lh-sign", (unsigned)(int)(short)ph[1], 0xFFFF80FFu);
    pb[4] = 0xAA;
    check("sb-lane0", pw[1], 0x000000AAu);
    pb[5] = 0xBB;
    check("sb-lane1", pw[1], 0x0000BBAAu);
    ph[3] = 0xCCDD;
    check("sh-hi", pw[1], 0xCCDDBBAAu);
    check("sb-no-clobber", pw[0], 0x80FF7F01u);
}

/* ---------- 3. every branch type ---------- */
static void t_branch(void)
{
    volatile int x = 5, y = 7, n = -3;
    unsigned c = 0;
    if (x == 5)                c |= 1;     /* beq  */
    if (x != y)                c |= 2;     /* bne  */
    if (x < y)                 c |= 4;     /* blt  */
    if (y >= x)                c |= 8;     /* bge  */
    if ((unsigned)-1 > (unsigned)x) c |= 16;  /* bltu */
    if ((unsigned)x <= (unsigned)-1) c |= 32; /* bgeu */
    if (n < x)                 c |= 64;    /* signed neg < pos */
    if ((unsigned)n < (unsigned)x) c |= 128;  /* must NOT be taken */
    check("branches", c, 0x7Fu);
}

/* ---------- 4. recursion / stack ---------- */
static NOINL unsigned fib(unsigned n) { return n < 2 ? n : fib(n - 1) + fib(n - 2); }
static NOINL unsigned sumto(unsigned n) { return n == 0 ? 0 : n + sumto(n - 1); }
static NOINL unsigned ack(unsigned m, unsigned n)
{
    if (m == 0) return n + 1;
    if (n == 0) return ack(m - 1, 1);
    return ack(m - 1, ack(m, n - 1));
}
static void t_recursion(void)
{
    volatile unsigned f = 18, d = 50, am = 2, an = 3;
    check("fib18",  fib(f), 2584);
    check("sumto50", sumto(d), 1275);
    check("ack2_3", ack(am, an), 9);
}

/* ---------- 5. sorting ---------- */
static unsigned xs = 0x2545F491u;
static unsigned rnd(void) { xs ^= xs << 13; xs ^= xs >> 17; xs ^= xs << 5; return xs; }
static unsigned arr[16];
static void t_sort(void)
{
    unsigned sum0 = 0, sum1 = 0, ok = 1;
    for (int i = 0; i < 16; i++) { arr[i] = rnd() & 0xFFFFu; sum0 += arr[i]; }
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15 - i; j++)
            if (arr[j] > arr[j + 1]) { unsigned t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t; }
    for (int i = 0; i < 16; i++) sum1 += arr[i];
    for (int i = 0; i < 15; i++) if (arr[i] > arr[i + 1]) ok = 0;
    check("sort-ordered", ok, 1);
    check("sort-sum", sum1, sum0);
}

/* ---------- 6. software mul / gcd / collatz (shift-add loops) ---------- */
static NOINL unsigned mul_sw(unsigned a, unsigned b)
{
    unsigned r = 0;
    while (b) { if (b & 1u) r += a; a <<= 1; b >>= 1; }
    return r;
}
static NOINL unsigned gcd_sw(unsigned a, unsigned b)
{
    while (a != b) { if (a > b) a -= b; else b -= a; }
    return a;
}
static NOINL unsigned collatz(unsigned n)
{
    unsigned steps = 0;
    while (n != 1) { n = (n & 1u) ? (n + n + n + 1u) : (n >> 1); steps++; }
    return steps;
}
static void t_math(void)
{
    volatile unsigned a = 123, b = 456, m = 0xFFFFu, g1 = 1071, g2 = 462, c = 27;
    check("mul_sw",  mul_sw(a, b), 56088u);
    check("mul_big", mul_sw(m, m), 0xFFFE0001u);
    check("gcd",     gcd_sw(g1, g2), 21u);
    check("collatz27", collatz(c), 111u);
}

/* ---------- 7. load-use hazard chain (pointer chasing) ---------- */
static unsigned char nxt[32];
static void t_hazard(void)
{
    for (int i = 0; i < 32; i++) nxt[i] = (unsigned char)((i + 5) & 31);
    unsigned idx = 0;
    for (int k = 0; k < 100; k++) idx = nxt[idx];   /* every load feeds the next */
    check("ptr-chase", idx, 20u);
}

/* ---------- 8. function pointers / .rodata code pointers ---------- */
static NOINL int f_add(int a, int b) { return a + b; }
static NOINL int f_sub(int a, int b) { return a - b; }
static NOINL int f_xor(int a, int b) { return a ^ b; }
static int (*const ops[3])(int, int) = { f_add, f_sub, f_xor };
static void t_fptr(void)
{
    volatile int i0 = 0, i1 = 1, i2 = 2;
    check("fptr-add", (unsigned)ops[i0](10, 3), 13u);
    check("fptr-sub", (unsigned)ops[i1](10, 3), 7u);
    check("fptr-xor", (unsigned)ops[i2](10, 3), 9u);
}

/* ---------- 9. .data / .rodata / .bss initialisation ---------- */
static int gdata[4] = { 1, 2, 3, 4 };
static const unsigned rotab[4] = { 0xDEADBEEFu, 0x01234567u, 0x89ABCDEFu, 0x0u };
static int gbss[8];
static void t_globals(void)
{
    unsigned z = 0;
    for (int i = 0; i < 8; i++) z |= (unsigned)gbss[i];
    check("bss-zero", z, 0u);
    check("data-init", (unsigned)(gdata[0] + gdata[1] + gdata[2] + gdata[3]), 10u);
    gdata[2] = 100;
    check("data-write", (unsigned)(gdata[0] + gdata[1] + gdata[2] + gdata[3]), 107u);
    check("rodata0", rotab[0], 0xDEADBEEFu);
    check("rodata2", rotab[2], 0x89ABCDEFu);
}

/* ---------- 10. switch (compare chain, no jump table) ---------- */
static NOINL int sw(int v)
{
    switch (v) { case 0: return 11; case 1: return 22; case 2: return 33;
                 case 7: return 77; default: return -1; }
}
static void t_switch(void)
{
    volatile int a = 0, b = 2, c = 7, d = 9;
    check("switch", (unsigned)(sw(a) + sw(b) + sw(c) + sw(d)), (unsigned)(11 + 33 + 77 - 1));
}

int main(void)
{
    LEDS_SET(0x01);
    t_alu();       LEDS_SET(0x03);
    t_mem();       LEDS_SET(0x07);
    t_branch();    LEDS_SET(0x0F);
    t_recursion(); LEDS_SET(0x1F);
    t_sort();      LEDS_SET(0x3F);
    t_math();      LEDS_SET(0x7F);
    t_hazard();
    t_fptr();
    t_globals();
    t_switch();
    if (fails == 0) { puts_("ALL TESTS PASSED\n"); LEDS_SET(0xFFFF); }
    else { print_uint((unsigned)fails); puts_(" TEST(S) FAILED\n"); }
    return fails;
}
