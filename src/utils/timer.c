/*
  Copyright (c) 2013-2014 Dong Fang. All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include <time.h>
#include <sys/time.h>
#include "timer.h"

i64 gettimeofms()
{
	struct timeval tv;
	i64 ct;

	gettimeofday (&tv, NULL);
	ct = (i64) tv.tv_sec * 1000 + tv.tv_usec / 1000;

	return ct;
}

i64 gettimeofus()
{
	struct timeval tv;
	i64 ct;

	gettimeofday (&tv, NULL);
	ct = (i64) tv.tv_sec * 1000000 + tv.tv_usec;

	return ct;
}

i64 gettimeofns()
{
	struct timeval tv;
	i64 ct;

	gettimeofday (&tv, NULL);
	ct = (i64) tv.tv_sec * 1000000000 + (i64) tv.tv_usec * 1000;

	return ct;
}
