// Start-up CPU check for the amd64 builds. Compiled for baseline x86-64 (see CMakeLists.txt) and
// run as the first constructor, so a v3 binary on a CPU without AVX2 exits with a message
// instead of dying on an illegal instruction.
#if defined(__x86_64__)
#include <cpuid.h>
#include <unistd.h>

#include <cstring>

namespace
{
    void fail(const char *need, const char *missing)
    {
        char msg[256] = "fsf: thi sbuild (" FSF_TARGET_NAME ") needs ";
        std::strcat(msg, need);
        std::strcat(msg, "; this CPU lacks ");
        std::strcat(msg, missing);
        std::strcat(msg, ".");
#if FSF_CPU_LEVEL >= 3
        std::strcat(msg, " Use fsf-linux-amd64-v2 on this machine.");
#endif
        std::strcat(msg, "\n");
        ssize_t r = write(2, msg, std::strlen(msg));
        (void)r;
        _exit(1);
    }

/*
This block compiles xgetbv0() only for builds targeting CPU level 3 or higher. The function reads extended control register 0 (XCR0), which reports which processor state components the operating system has enabled for saving and restoring.

The inline assembly executes the xgetbv instruction with ECX set to 0. It receives the 64-bit result in EDX:EAX format, then combines those halves into an unsigned long long. The caller can inspect the resulting bits to determine whether the OS has enabled the state needed by features such as AVX. Because xgetbv is not safe to execute on every CPU, the surrounding startup check must first establish that it is supported.
*/
#if FSF_CPU_LEVEL >= 3
    unsigned long long xgetbv0()
    {
        unsigned eax, edx;
        __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
        return (static_cast<unsigned long long>(edx) << 32) | eax;
    }
#endif

    __attribute__((constructor(101), used)) void fsf_cpu_check()
    {
        unsigned a, b, c, d;
        if (!__get_cpuid(1, &a, &b, &c, &d))
            fail("x86-64-v2", "CPUID leaf 1");
        unsigned c1 = c;
        unsigned ext_c = 0;
        if (__get_cpuid(0x80000001, &a, &b, &c, &d))
            ext_c = c;

        // x86-64-v2: CMPXCHG16B, LAHF-SAHF, POPCNT, SSE3, SSE4.1, SSE4.2, SSSE3
        struct Bit
        {
            unsigned reg;
            unsigned bit;
            const char *name;
        };
        const Bit v2[] = {{c1, bit_SSE3, "SSE3"}, {c1, bit_SSSE3, "SSSE3"}, {c1, bit_SSE4_1, "SSE4.1"}, {c1, bit_SSE4_2, "SSE4.2"}, {c1, bit_POPCNT, "POPCNT"}, {c1, bit_CMPXCHG16B, "CMPXCHG16B"}, {ext_c, bit_LAHF_LM, "LAHF-SAHF"}};
        for (const Bit &x : v2)
            if (!(x.reg & x.bit))
                fail("x86-64-v2", x.name);

#if FSF_CPU_LEVEL >= 3
        // x86-64-v3: AVX, AVX2, BMI1, BMI2, F16C, FMA, LZCNT, MOVBE, OSXSAVE, with the OS saving
        // the SSE and AVX register state (XCR0 bits 1 and 2).
        if (!(c1 & bit_OSXSAVE))
            fail("x86-64-v3", "OSXSAVE");
        if ((xgetbv0() & 0x6) != 0x6)
            fail("x86-64-v3", "OS support for AVX state");
        unsigned b7 = 0;
        if (__get_cpuid_count(7, 0, &a, &b, &c, &d))
            b7 = b;
        const Bit v3[] = {{c1, bit_AVX, "AVX"}, {b7, bit_AVX2, "AVX2"}, {b7, bit_BMI, "BMI1"}, {b7, bit_BMI2, "BMI2"}, {c1, bit_F16C, "F16C"}, {c1, bit_FMA, "FMA"}, {ext_c, bit_LZCNT, "LZCNT"}, {c1, bit_MOVBE, "MOVBE"}};
        for (const Bit &x : v3)
            if (!(x.reg & x.bit))
                fail("x86-64-v3", x.name);
#endif
    }

} // namespace
#endif