/*
 * op_k2.cpp
 *
 *  Created on: 18 нояб. 2019 г.
 *      Author: sadko
 */

#include <dsp/dsp.h>
#include <test/ptest.h>
#include <core/sugar.h>

#define MIN_RANK 8
#define MAX_RANK 16

namespace native
{
    void    add_k2(float *dst, float k, size_t count);
    void    sub_k2(float *dst, float k, size_t count);
    void    mul_k2(float *dst, float k, size_t count);
    void    div_k2(float *dst, float k, size_t count);
    void    rsub_k2(float *dst, float k, size_t count);
    void    rdiv_k2(float *dst, float k, size_t count);
}

IF_ARCH_X86(
    namespace sse
    {
        void    add_k2(float *dst, float k, size_t count);
        void    sub_k2(float *dst, float k, size_t count);
        void    mul_k2(float *dst, float k, size_t count);
        void    div_k2(float *dst, float k, size_t count);
        void    rsub_k2(float *dst, float k, size_t count);
        void    rdiv_k2(float *dst, float k, size_t count);
    }
)

IF_ARCH_ARM(
    namespace neon_d32
    {
        void    add_k2(float *dst, float k, size_t count);
        void    sub_k2(float *dst, float k, size_t count);
        void    mul_k2(float *dst, float k, size_t count);
        void    div_k2(float *dst, float k, size_t count);
        void    rsub_k2(float *dst, float k, size_t count);
        void    rdiv_k2(float *dst, float k, size_t count);
    }
)

IF_ARCH_AARCH64(
    namespace asimd
    {
        void    add_k2(float *dst, float k, size_t count);
        void    sub_k2(float *dst, float k, size_t count);
        void    mul_k2(float *dst, float k, size_t count);
        void    div_k2(float *dst, float k, size_t count);
        void    rsub_k2(float *dst, float k, size_t count);
        void    rdiv_k2(float *dst, float k, size_t count);
    }
)

typedef void (* op_k2_t)(float *dst, float k, size_t count);

//-----------------------------------------------------------------------------
PTEST_BEGIN("dsp.pmath", op_k2, 5, 1000)

    void call(const char *label, float *dst, size_t count, op_k2_t func)
    {
        if (!PTEST_SUPPORTED(func))
            return;

        char buf[80];
        sprintf(buf, "%s x %d", label, int(count));
        printf("Testing %s numbers...\n", buf);

        PTEST_LOOP(buf,
            func(dst, 1.001f, count);
        );
    }

    PTEST_MAIN
    {
        size_t buf_size = 1 << MAX_RANK;
        uint8_t *data   = NULL;
        float *dst      = alloc_aligned<float>(data, buf_size * 2, 64);
        float *backup   = &dst[buf_size];

        for (size_t i=0; i < buf_size; ++i)
            dst[i]          = float(rand()) / RAND_MAX;
        dsp::copy(backup, dst, buf_size);

        #define CALL(...) \
            dsp::copy(dst, backup, buf_size); \
            call(__VA_ARGS__);

        for (size_t i=MIN_RANK; i <= MAX_RANK; ++i)
        {
            size_t count = 1 << i;

            CALL("native::add_k2", dst, count, native::add_k2);
            IF_ARCH_X86(CALL("sse::add_k2", dst, count, sse::add_k2));
            IF_ARCH_ARM(CALL("neon_d32::add_k2", dst, count, neon_d32::add_k2));
            IF_ARCH_AARCH64(CALL("asimd::add_k2", dst, count, asimd::add_k2));
            PTEST_SEPARATOR;

            CALL("native::sub_k2", dst, count, native::sub_k2);
            IF_ARCH_X86(CALL("sse::sub_k2", dst, count, sse::sub_k2));
            IF_ARCH_ARM(CALL("neon_d32::sub_k2", dst, count, neon_d32::sub_k2));
            IF_ARCH_AARCH64(CALL("asimd::sub_k2", dst, count, asimd::sub_k2));
            PTEST_SEPARATOR;

            CALL("native::rsub_k2", dst, count, native::rsub_k2);
            IF_ARCH_X86(CALL("sse::rsub_k2", dst, count, sse::rsub_k2));
            IF_ARCH_ARM(CALL("neon_d32::rsub_k2", dst, count, neon_d32::rsub_k2));
            IF_ARCH_AARCH64(CALL("asimd::rsub_k2", dst, count, asimd::rsub_k2));
            PTEST_SEPARATOR;

            CALL("native::mul_k2", dst, count, native::mul_k2);
            IF_ARCH_X86(CALL("sse::mul_k2", dst, count, sse::mul_k2));
            IF_ARCH_ARM(CALL("neon_d32::mul_k2", dst, count, neon_d32::mul_k2));
            IF_ARCH_AARCH64(CALL("asimd::mul_k2", dst, count, asimd::mul_k2));
            PTEST_SEPARATOR;

            CALL("native::div_k2", dst, count, native::div_k2);
            IF_ARCH_X86(CALL("sse::div_k2", dst, count, sse::div_k2));
            IF_ARCH_ARM(CALL("neon_d32::div_k2", dst, count, neon_d32::div_k2));
            IF_ARCH_AARCH64(CALL("asimd::div_k2", dst, count, asimd::div_k2));
            PTEST_SEPARATOR;

            CALL("native::rdiv_k2", dst, count, native::rdiv_k2);
            IF_ARCH_X86(CALL("sse::rdiv_k2", dst, count, sse::rdiv_k2));
            IF_ARCH_ARM(CALL("neon_d32::rdiv_k2", dst, count, neon_d32::rdiv_k2));
            IF_ARCH_AARCH64(CALL("asimd::rdiv_k2", dst, count, asimd::rdiv_k2));
            PTEST_SEPARATOR2;
        }

        free_aligned(data);
    }
PTEST_END
