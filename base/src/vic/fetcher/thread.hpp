//+++HDR+++
//======================================================================
//   This file is part of the RATMAN software framework.
//   Copyright (C) 2009 by CRS4, Pula, Italy.
//
//   For more information, visit the CRS4 Visual Computing Group
//   web pages at http://www.crs4.it/vic/
//
//   This file may be used under the terms of the GNU General Public
//   License as published by the Free Software Foundation and appearing
//   in the file LICENSE included in the packaging of this file.
//
//   CRS4 reserves all rights not expressly granted herein.
//  
//   This file is provided AS IS with NO WARRANTY OF ANY KIND, 
//   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS 
//   FOR A PARTICULAR PURPOSE.
//
//======================================================================
//---HDR---//
#ifndef FETCHER_THREAD_HPP
#define FETCHER_THREAD_HPP

// Operating system dependent calls for thread utility

#if _WIN32
#include <windows.h>

#  define FETCHER_SET_THREAD_PRIORITY_IDLE \
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL)

#  define FETCHER_CPU_YIELD \
  Sleep(1)

#else

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

#define FETCHER_SET_THREAD_PRIORITY_IDLE \
    setpriority(PRIO_PROCESS, getpid(), 10)

#  define FETCHER_CPU_YIELD { \
    struct timespec ts = { 0, 10000 };	\
    nanosleep(&ts,0); \
  }

#endif

#endif
