# mpool

sandbox to play with memory allocation

# TODO
* add scratch memory to alloc things bigger than a page.
* use free ring instead of list => lockless

# xmalloc test

Taken and adapted from [mimalloc-bench](https://github.com/daanx/mimalloc-bench/tree/master/bench/xmalloc-test)

xmalloc-test.c: is also known as "malloc-test"

Originally by C. Lever and D. Boreham, Christian Eder ( ederc@mathematik.uni-kl.de) for [xmalloc](https://github.com/ederc/xmalloc),
and modified by Bradley C. Kuzmaul for SuperMalloc <https://github.com/kuszmaul/SuperMalloc>

```
* \author C. Lever and D. Boreham, Christian Eder ( ederc@mathematik.uni-kl.de )
* \date   2000
* \brief  Test file for xmalloc. This is a multi-threaded test system by
*         Lever and Boreham. It is first noted in their paper "malloc()
*         Performance in a Multithreaded Linux Environment", appeared at the
*         USENIX 2000 Annual Technical Conference: FREENIX Track.
*         This file is part of XMALLOC, licensed under the GNU General
*         Public License version 3. See COPYING for more information.
```

Random.h is based on lran2.h by Wolfram Gloger 1996
