#define CONSOLE (*(volatile unsigned int *)0xFFFFFFF4u)
#define LEDS    (*(volatile unsigned int *)0xFFFFFFF0u)

static void putch(char c) { CONSOLE = (unsigned char)c; }

static void puts_(const char *s) { while (*s) putch(*s++); }

/* no divide (RV32I has none): repeated subtraction */
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

int main(void)
{
    int a = 10, b = 20;
    LEDS = 0x55;
    puts_("Hello from my RV32I CPU!\n");
    print_uint(a); puts_(" + "); print_uint(b); puts_(" = ");
    print_uint(a + b); putch('\n');
    LEDS = 0xAA;
    return 0;            /* startup writes this to the EXIT register */
}
