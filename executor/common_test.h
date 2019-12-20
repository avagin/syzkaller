// Copyright 2018 syzkaller project authors. All rights reserved.
// Use of this source code is governed by Apache 2 LICENSE that can be found in the LICENSE file.

// This file is shared between executor and csource package.

#include <stdlib.h>
#include <unistd.h>

#if SYZ_EXECUTOR || __NR_syz_mmap
#include <sys/mman.h>

// syz_mmap(addr vma, len len[addr])
static long syz_mmap(volatile long a0, volatile long a1)
{
	return (long)mmap((void*)a0, a1, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_FIXED, -1, 0);
}
#endif

#if SYZ_EXECUTOR || __NR_syz_errno
#include <errno.h>

// syz_errno(v int32)
static long syz_errno(volatile long v)
{
	errno = v;
	return v == 0 ? 0 : -1;
}
#endif

#if SYZ_EXECUTOR || __NR_syz_exit
// syz_exit(status int32)
static long syz_exit(volatile long status)
{
	_exit(status);
	return 0;
}
#endif

#if SYZ_EXECUTOR || __NR_syz_compare
#include <errno.h>
#include <string.h>

// syz_compare(want ptr[in, string], want_len len[want], got ptr[in, compare_data], got_len len[got])
static long syz_compare(volatile long want, volatile long want_len, volatile long got, volatile long got_len)
{
	if (want_len != got_len) {
		debug("syz_compare: want_len=%lu got_len=%lu data:\n", want_len, got_len);
		debug_dump_data((char*)got, got_len);
		errno = EBADF;
		return -1;
	}
	if (memcmp((void*)want, (void*)got, want_len)) {
		debug("syz_compare: data differs, want:\n");
		debug_dump_data((char*)want, want_len);
		debug("got:\n");
		debug_dump_data((char*)got, got_len);
		errno = EINVAL;
		return -1;
	}
	return 0;
}
#endif

#if SYZ_EXECUTOR || __NR_syz_compare_int
#include <errno.h>
#include <stdarg.h>

// syz_compare_int$4(n const[2], v0 intptr, v1 intptr, v2 intptr, v3 intptr)
static long syz_compare_int(volatile long n, ...)
{
	va_list args;
	va_start(args, n);
	long v0 = va_arg(args, long);
	long v1 = va_arg(args, long);
	long v2 = va_arg(args, long);
	long v3 = va_arg(args, long);
	va_end(args);
	if (n < 2 || n > 4)
		return errno = E2BIG, -1;
	if (n <= 2 && v2 != 0)
		return errno = EFAULT, -1;
	if (n <= 3 && v3 != 0)
		return errno = EFAULT, -1;
	if (v0 != v1)
		return errno = EINVAL, -1;
	if (n > 2 && v0 != v2)
		return errno = EINVAL, -1;
	if (n > 3 && v0 != v3)
		return errno = EINVAL, -1;
	return 0;
}
#endif

#if SYZ_EXECUTOR || SYZ_SANDBOX_NONE
static void loop();
static int do_sandbox_none(void)
{
	loop();
	return 0;
}
#endif
