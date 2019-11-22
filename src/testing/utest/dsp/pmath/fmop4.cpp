/*
 * fmop4.cpp
 *
 *  Created on: 22 нояб. 2019 г.
 *      Author: sadko
 */


#include <dsp/dsp.h>
#include <test/utest.h>
#include <test/FloatBuffer.h>
#include <core/sugar.h>

#define MIN_RANK 8
#define MAX_RANK 16

namespace native
{
    void    fmadd4(float *dst, const float *a, const float *b, const float *c, size_t count);
    void    fmsub4(float *dst, const float *a, const float *b, const float *c, size_t count);
    void    fmrsub4(float *dst, const float *a, const float *b, const float *c, size_t count);
    void    fmmul4(float *dst, const float *a, const float *b, const float *c, size_t count);
    void    fmdiv4(float *dst, const float *a, const float *b, const float *c, size_t count);
    void    fmrdiv4(float *dst, const float *a, const float *b, const float *c, size_t count);
}

IF_ARCH_X86(
    namespace sse
    {
        void    fmadd4(float *dst, const float *a, const float *b, const float *c, size_t count);
        void    fmsub4(float *dst, const float *a, const float *b, const float *c, size_t count);
        void    fmrsub4(float *dst, const float *a, const float *b, const float *c, size_t count);
        void    fmmul4(float *dst, const float *a, const float *b, const float *c, size_t count);
        void    fmdiv4(float *dst, const float *a, const float *b, const float *c, size_t count);
        void    fmrdiv4(float *dst, const float *a, const float *b, const float *c, size_t count);
    }
)

typedef void (* fmop4_t)(float *dst, const float *a, const float *b, const float *c, size_t count);

UTEST_BEGIN("dsp.pmath", fmop4)

    void call(const char *label, size_t align, fmop4_t func1, fmop4_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                32, 64, 65, 100, 999, 0xfff)
        {
            for (size_t mask=0; mask <= 0x0f; ++mask)
            {
                printf("Testing %s on input buffer of %d numbers, mask=0x%x...\n", label, int(count), int(mask));

                FloatBuffer a(count, align, mask & 0x01);
                FloatBuffer b(count, align, mask & 0x02);
                FloatBuffer c(count, align, mask & 0x04);
                FloatBuffer dst1(count, align, mask & 0x08);
                FloatBuffer dst2(count, align, mask & 0x08);

                // Call functions
                a.randomize_sign();
                b.randomize_sign();
                func1(dst1, a, b, c, count);
                func2(dst2, a, b, c, count);

                UTEST_ASSERT_MSG(a.valid(), "Buffer A corrupted");
                UTEST_ASSERT_MSG(b.valid(), "Buffer B corrupted");
                UTEST_ASSERT_MSG(c.valid(), "Buffer C corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst1.equals_adaptive(dst2, 1e-4))
                {
                    a.dump("a   ");
                    a.dump("b   ");
                    a.dump("c   ");
                    dst1.dump("dst1");
                    dst2.dump("dst2");
                    printf("index=%d, %.6f vs %.6f\n", dst1.last_diff(), dst1.get_diff(), dst2.get_diff());
                    UTEST_FAIL_MSG("Output of functions for test '%s' differs", label);
                }
            }
        }
    }

    UTEST_MAIN
    {
        #define CALL(ns, func, align) call(#ns "::" #func, align, native::func, ns::func)

        IF_ARCH_X86(CALL(sse, fmadd4, 16));
        IF_ARCH_X86(CALL(sse, fmsub4, 16));
        IF_ARCH_X86(CALL(sse, fmrsub4, 16));
        IF_ARCH_X86(CALL(sse, fmmul4, 16));
        IF_ARCH_X86(CALL(sse, fmdiv4, 16));
        IF_ARCH_X86(CALL(sse, fmrdiv4, 16));
    }
UTEST_END






