/*
 * fmop_k4.cpp
 *
 *  Created on: 23 авг. 2018 г.
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
    void    fmadd_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
    void    fmsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
    void    fmrsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
    void    fmmul_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
    void    fmdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
    void    fmrdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
}

IF_ARCH_X86(
    namespace sse
    {
        void    fmadd_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmrsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmmul_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmrdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
    }

    namespace avx
    {
        void    x64_fmadd_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    x64_fmsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    x64_fmrsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    x64_fmmul_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    x64_fmdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    x64_fmrdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);

        void    x64_fmadd_k4_fma3(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    x64_fmsub_k4_fma3(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    x64_fmrsub_k4_fma3(float *dst, const float *src1, const float *src2, float k, size_t count);
    }
)

IF_ARCH_ARM(
    namespace neon_d32
    {
        void    fmadd_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmrsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmmul_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmrdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
    }
)

IF_ARCH_AARCH64(
    namespace asimd
    {
        void    fmadd_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmrsub_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmmul_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
        void    fmrdiv_k4(float *dst, const float *src1, const float *src2, float k, size_t count);
    }
)

typedef void (* fmop_k4_t)(float *dst, const float *src1, const float *src2, float k, size_t count);

UTEST_BEGIN("dsp.pmath", fmop_k4)

    void call(const char *label, size_t align, fmop_k4_t func1, fmop_k4_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                32, 64, 65, 100, 999, 0xfff)
        {
            for (size_t mask=0; mask <= 0x07; ++mask)
            {
                printf("Testing %s on input buffer of %d numbers, mask=0x%x...\n", label, int(count), int(mask));

                FloatBuffer src1(count, align, mask & 0x01);
                FloatBuffer src2(count, align, mask & 0x02);
                FloatBuffer dst1(count, align, mask & 0x04);
                FloatBuffer dst2(count, align, mask & 0x04);

                // Call functions
                src1.randomize_sign();
                src2.randomize_sign();
                func1(dst1, src1, src2, 0.4f, count);
                func2(dst2, src1, src2, 0.4f, count);

                UTEST_ASSERT_MSG(src1.valid(), "Source buffer 1 corrupted");
                UTEST_ASSERT_MSG(src2.valid(), "Source buffer 2 corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst1.equals_adaptive(dst2, 1e-4))
                {
                    src1.dump("src1");
                    src2.dump("src2");
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
        IF_ARCH_X86(call("sse::fmadd_k4", 16, native::fmadd_k4, sse::fmadd_k4));
        IF_ARCH_X86(call("sse::fmsub_k4", 16, native::fmsub_k4, sse::fmsub_k4));
        IF_ARCH_X86(call("sse::fmrsub_k4", 16, native::fmrsub_k4, sse::fmrsub_k4));
        IF_ARCH_X86(call("sse::fmmul_k4", 16, native::fmmul_k4, sse::fmmul_k4));
        IF_ARCH_X86(call("sse::fmdiv_k4", 16, native::fmdiv_k4, sse::fmdiv_k4));
        IF_ARCH_X86(call("sse::fmrdiv_k4", 16, native::fmrdiv_k4, sse::fmrdiv_k4));

        IF_ARCH_X86(call("avx::x64_fmadd_k4", 32, native::fmadd_k4, avx::x64_fmadd_k4));
        IF_ARCH_X86(call("avx::x64_fmsub_k4", 32, native::fmsub_k4, avx::x64_fmsub_k4));
        IF_ARCH_X86(call("avx::x64_fmrsub_k4", 32, native::fmrsub_k4, avx::x64_fmrsub_k4));
        IF_ARCH_X86(call("avx::x64_fmmul_k4", 32, native::fmmul_k4, avx::x64_fmmul_k4));
        IF_ARCH_X86(call("avx::x64_fmdiv_k4", 32, native::fmdiv_k4, avx::x64_fmdiv_k4));
        IF_ARCH_X86(call("avx::x64_fmrdiv_k4", 32, native::fmrdiv_k4, avx::x64_fmrdiv_k4));
        IF_ARCH_X86(call("avx::x64_fmadd_k4_fma3", 32, native::fmadd_k4, avx::x64_fmadd_k4_fma3));
        IF_ARCH_X86(call("avx::x64_fmsub_k4_fma3", 32, native::fmsub_k4, avx::x64_fmsub_k4_fma3));
        IF_ARCH_X86(call("avx::x64_fmrsub_k4_fma3", 32, native::fmrsub_k4, avx::x64_fmrsub_k4_fma3));

        IF_ARCH_ARM(call("neon_d32::fmadd_k4", 16, native::fmadd_k4, neon_d32::fmadd_k4));
        IF_ARCH_ARM(call("neon_d32::fmsub_k4", 16, native::fmsub_k4, neon_d32::fmsub_k4));
        IF_ARCH_ARM(call("neon_d32::fmrsub_k4", 16, native::fmrsub_k4, neon_d32::fmrsub_k4));
        IF_ARCH_ARM(call("neon_d32::fmmul_k4", 16, native::fmmul_k4, neon_d32::fmmul_k4));
        IF_ARCH_ARM(call("neon_d32::fmdiv_k4", 16, native::fmdiv_k4, neon_d32::fmdiv_k4));
        IF_ARCH_ARM(call("neon_d32::fmrdiv_k4", 16, native::fmrdiv_k4, neon_d32::fmrdiv_k4));

        IF_ARCH_AARCH64(call("asimd::fmadd_k4", 16, native::fmadd_k4, asimd::fmadd_k4));
        IF_ARCH_AARCH64(call("asimd::fmsub_k4", 16, native::fmsub_k4, asimd::fmsub_k4));
        IF_ARCH_AARCH64(call("asimd::fmrsub_k4", 16, native::fmrsub_k4, asimd::fmrsub_k4));
        IF_ARCH_AARCH64(call("asimd::fmmul_k4", 16, native::fmmul_k4, asimd::fmmul_k4));
        IF_ARCH_AARCH64(call("asimd::fmdiv_k4", 16, native::fmdiv_k4, asimd::fmdiv_k4));
        IF_ARCH_AARCH64(call("asimd::fmrdiv_k4", 16, native::fmrdiv_k4, asimd::fmrdiv_k4));
    }
UTEST_END


