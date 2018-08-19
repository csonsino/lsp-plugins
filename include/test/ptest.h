/*
 * ptest.h
 *
 *  Created on: 16 авг. 2018 г.
 *      Author: sadko
 */

#ifndef INCLUDE_TEST_PTEST_H_
#define INCLUDE_TEST_PTEST_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <core/types.h>
#include <data/cstorage.h>

#define PTEST_BEGIN(group, name, time, iterations) \
        namespace test { \
        namespace ptest { \
        namespace name { \
            \
            using namespace ::test; \
            \
            class ptest_ ## name: public PerformanceTest { \
                public: \
                    explicit ptest_ ## name() : PerformanceTest(group, #name, time, iterations) {} \
                    \
                    virtual ~ptest_ ## name() {}

#define PTEST_IGNORE \
        virtual bool ignore() const { return true; }

#define PTEST_MAIN \
        virtual void execute()

#define PTEST_LOOP(__key, ...) { \
        double __start = clock(); \
        double __time = 0.0f; \
        wsize_t __iterations = 0; \
        \
        do { \
            for (size_t i=0; i<__test_iterations; ++i) { \
                __VA_ARGS__; \
            } \
            /* Calculate statistics */ \
            __iterations   += __test_iterations; \
            __time          = (clock() - __start) / CLOCKS_PER_SEC; \
        } while (__time < __test_time); \
        \
        gather_stats(__key, __time, __iterations); \
        if (__verbose) { \
            printf("  time [s]:                 %.2f/%.2f\n", __time, __test_time); \
            printf("  iterations:               %ld/%ld\n", long(__iterations), long((__iterations * __test_time) / __time)); \
            printf("  performance [i/s]:        %.2f\n", __iterations / __time); \
            printf("  iteration time [us/i]:    %.4f\n\n", (1000000.0 * __time) / __iterations); \
        } \
    }

#define PTEST_FAIL(code)    \
        fprintf(stderr, "Performance test '%s' group '%s' has failed at file %s, line %d", \
                __test_name, __test_group, __FILE__, __LINE__); \
        exit(code));

#define PTEST_END \
        } performance_test;  /* ptest class */ \
        } /* namespace name */ \
        } /* namespace ptest */ \
        } /* namespace test */

namespace test
{
    class PerformanceTest
    {
        private:
            friend PerformanceTest *ptest_init();

        private:
            static PerformanceTest    *__root;
            PerformanceTest           *__next;

        protected:
            typedef struct stats_t
            {
                char       *key;            /* The loop indicator */
                char       *time;           /* Actual time [seconds] */
                char       *n_time;         /* Normalized time [seconds] */
                char       *iterations;     /* Number of iterations */
                char       *n_iterations;   /* Normalized number of iterations */
                char       *performance;    /* The performance of test [iterations per second] */
                char       *time_cost;      /* The amount of time spent per iteration [milliseconds per iteration] */
            } stats_t;

        protected:
            const char                         *__test_group;
            const char                         *__test_name;
            bool                                __verbose;
            size_t                              __test_iterations;
            double                              __test_time;
            mutable lsp::cstorage<stats_t>      __test_stats;

        protected:
            void gather_stats(const char *key, double time, wsize_t iterations);
            static void destroy_stats(stats_t *stats);
            static void estimate(size_t *len, const char *text);
            static void out_text(size_t length, const char *text, int align, const char *padding, const char *tail);

        public:
            explicit PerformanceTest(const char *group, const char *name, float time, size_t iterations);
            virtual ~PerformanceTest();

        public:
            inline const char *name() const     { return __test_name; }
            inline const char *group() const    { return __test_group; }
            inline PerformanceTest *next()      { return __next; }
            void dump_stats() const;
            void free_stats();
            inline void set_verbose(bool verbose)      { __verbose = verbose; }

        public:
            virtual void execute() = 0;

            virtual bool ignore() const;
    };


    /**
     * Initialize set of performance tests (validate duplicates, etc)
     * @return valid set of performance tests
     */
    PerformanceTest *ptest_init();
}

#endif /* INCLUDE_TEST_PTEST_H_ */
