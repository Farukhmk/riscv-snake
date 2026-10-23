/* Self-playing terminal Snake for the RV32I core (no mul/div/mod, no libc).
 * Each move: BFS path to the food, safety check by flood fill, fallback to the
 * neighbour with the most reachable space. Frames go out through the MMIO
 * console at 0xFFFFFFF4 with ANSI colours. Score is mirrored on the LEDs. */
#ifdef HOST
#include <stdio.h>
static void putch(char c) { putchar(c); }
#define LEDS_SET(v) ((void)(v))
#define FRAME_SYNC() ((void)0)
#else
#define CONSOLE (*(volatile unsigned int *)0xFFFFFFF4u)
#define LEDS    (*(volatile unsigned int *)0xFFFFFFF0u)
#define LEDS_SET(v) (LEDS = (v))
#define FRAME_SYNC() (*(volatile unsigned int *)0xFFFFFFFCu = 1u)  /* testbench pacing only */
static void putch(char c) { CONSOLE = (unsigned char)c; }
#endif

#ifndef MAX_MOVES
#define MAX_MOVES   700
#endif
#ifndef FRAME_EVERY
#define FRAME_EVERY 1          /* draw every Nth move (1 = every move) */
#endif

#define GW    16               /* grid width  (power of two: idx = y*16 + x) */
#define GH    12
#define CELLS (GW * GH)        /* 192 */

static void puts_(const char *s) { while (*s) putch(*s++); }
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

/* ---- state ---- */
static unsigned char cell[CELLS];   /* 0 empty, 1 snake, 2 food */
static unsigned char body[256];     /* circular buffer of cell indices */
static unsigned char q[CELLS], par[CELLS], vis[CELLS], gen;
static int hd, tl, len, food, score, moves;
static unsigned xs = 0x2545F491u;

static unsigned rnd(void) { xs ^= xs << 13; xs ^= xs >> 17; xs ^= xs << 5; return xs; }

/* neighbour of idx in direction d, or -1 outside the grid */
static int nb(int idx, int d)
{
    switch (d) {
    case 0: return idx >= GW ? idx - GW : -1;
    case 1: return idx < CELLS - GW ? idx + GW : -1;
    case 2: return (idx & (GW - 1)) != 0 ? idx - 1 : -1;
    default: return (idx & (GW - 1)) != GW - 1 ? idx + 1 : -1;
    }
}

static void new_gen(void)
{
    gen++;
    if (gen == 0) { for (int i = 0; i < CELLS; i++) vis[i] = 0; gen = 1; }
}

/* cells reachable from start; the tail cell counts as free (it will move) */
static int flood(int start, int tail)
{
    int qh = 0, qt = 0, cnt = 0;
    new_gen();
    q[qt++] = (unsigned char)start; vis[start] = gen;
    while (qh < qt) {
        int c = q[qh++];
        cnt++;
        for (int d = 0; d < 4; d++) {
            int n = nb(c, d);
            if (n >= 0 && vis[n] != gen && (cell[n] != 1 || n == tail)) {
                vis[n] = gen; q[qt++] = (unsigned char)n;
            }
        }
    }
    return cnt;
}

/* first step of a shortest path head -> food, or -1 if unreachable */
static int bfs_step(int head, int target, int tail)
{
    int qh = 0, qt = 0;
    new_gen();
    q[qt++] = (unsigned char)head; vis[head] = gen; par[head] = (unsigned char)head;
    while (qh < qt) {
        int c = q[qh++];
        if (c == target) {
            int n = c;
            while (par[n] != head) n = par[n];
            return n;
        }
        for (int d = 0; d < 4; d++) {
            int n = nb(c, d);
            if (n >= 0 && vis[n] != gen && (cell[n] != 1 || n == tail)) {
                vis[n] = gen; par[n] = (unsigned char)c; q[qt++] = (unsigned char)n;
            }
        }
    }
    return -1;
}

static int place_food(void)
{
    if (len >= CELLS) return 0;
    for (;;) {
        unsigned r = (rnd() >> 8) & 255u;
        if (r < CELLS && cell[r] == 0) { cell[r] = 2; food = (int)r; return 1; }
    }
}

static void render(void)
{
    int head = body[hd];
    puts_("\x1b[H+");
    for (int x = 0; x < GW * 2; x++) putch('-');
    puts_("+\n");
    for (int y = 0; y < GH; y++) {
        putch('|');
        for (int x = 0; x < GW; x++) {
            int idx = (y << 4) + x;
            if (idx == head)        puts_("\x1b[93mOO\x1b[0m");
            else if (cell[idx] == 1) puts_("\x1b[92m@@\x1b[0m");
            else if (cell[idx] == 2) puts_("\x1b[91m**\x1b[0m");
            else                     puts_("  ");
        }
        puts_("|\n");
    }
    putch('+');
    for (int x = 0; x < GW * 2; x++) putch('-');
    puts_("+\nScore: "); print_uint((unsigned)score);
    puts_("  Moves: ");  print_uint((unsigned)moves);
    puts_("  Length: "); print_uint((unsigned)len);
    puts_("   \n");
    FRAME_SYNC();   /* lets the testbench pause here (+delay_ms=N); costs 1 store */
}

int main(void)
{
    /* start: length 3, row 6, heading right */
    for (int i = 0; i < 3; i++) {
        int idx = (6 << 4) + 4 + i;
        body[i] = (unsigned char)idx; cell[idx] = 1;
    }
    tl = 0; hd = 2; len = 3; score = 0; moves = 0;
    place_food();
    puts_("\x1b[2J");
    render();

    const char *result = "MOVE LIMIT";
    int fc = 0;
    while (moves < MAX_MOVES) {
        int head = body[hd], tail = body[tl], step = -1;

        int s = bfs_step(head, food, tail);
        if (s >= 0 && flood(s, tail) >= len) step = s;

        if (step < 0) {                       /* fallback: roomiest neighbour */
            int best = -1;
            for (int d = 0; d < 4; d++) {
                int n = nb(head, d);
                if (n >= 0 && (cell[n] != 1 || n == tail)) {
                    int a = flood(n, tail);
                    if (a > best) { best = a; step = n; }
                }
            }
        }
        if (step < 0) { result = "SNAKE TRAPPED"; break; }

        if (cell[step] == 2) {                /* eat: grow */
            score++; len++;
            LEDS_SET((unsigned)score);
            cell[step] = 1; hd = (hd + 1) & 255; body[hd] = (unsigned char)step;
            if (!place_food()) { moves++; result = "BOARD FULL - YOU WIN"; break; }
        } else {                              /* move: drop tail, add head */
            cell[body[tl]] = 0; tl = (tl + 1) & 255;
            cell[step] = 1; hd = (hd + 1) & 255; body[hd] = (unsigned char)step;
        }
        moves++;
        if (++fc >= FRAME_EVERY) { fc = 0; render(); }   /* no modulo: RV32I has no divide */
    }
    render();
    puts_("\n*** "); puts_(result);
    puts_(" ***  score "); print_uint((unsigned)score);
    puts_(" in ");        print_uint((unsigned)moves); puts_(" moves\n");
    return 0;
}
