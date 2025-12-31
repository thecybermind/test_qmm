/*
TEST_QMM - QMM plugin for testing QMM features in development
Copyright 2004-2025
https://github.com/thecybermind/test_qmm/
3-clause BSD license: https://opensource.org/license/bsd-3-clause

Created By:
    Kevin Masterson < k.m.masterson@gmail.com >

*/

#ifndef __TEST_QMM_VERSION_H__
#define __TEST_QMM_VERSION_H__

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

#define TEST_QMM_VERSION_MAJOR	2
#define TEST_QMM_VERSION_MINOR	4
#define TEST_QMM_VERSION_REV	1

#define TEST_QMM_VERSION		STRINGIFY(TEST_QMM_VERSION_MAJOR) "." STRINGIFY(TEST_QMM_VERSION_MINOR) "." STRINGIFY(TEST_QMM_VERSION_REV)

#if defined(_WIN32)
#define TEST_QMM_OS             "Windows"
#ifdef _WIN64
#define TEST_QMM_ARCH           "x86_64"
#else
#define TEST_QMM_ARCH           "x86"
#endif
#elif defined(__linux__)
#define TEST_QMM_OS             "Linux"
#ifdef __LP64__
#define TEST_QMM_ARCH           "x86_64"
#else
#define TEST_QMM_ARCH           "x86"
#endif
#endif

#define TEST_QMM_VERSION_DWORD	TEST_QMM_VERSION_MAJOR , TEST_QMM_VERSION_MINOR , TEST_QMM_VERSION_REV , 0
#define TEST_QMM_COMPILE		__TIME__ " " __DATE__
#define TEST_QMM_BUILDER		"Kevin Masterson"

#endif // __TEST_QMM_VERSION_H__
