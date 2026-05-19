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
#include <vic/cbdam/base/background_thread.hpp>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

namespace cbdam {

  background_thread::background_thread(QObject * parent) : QThread(parent) {
    creation_pid_ = getpid();
  }

  background_thread::~background_thread() {

  }

  void background_thread::set_background_priority() {
    setPriority(QThread::LowPriority);

    int thread_pid = getpid();

    // set low priority only if this is a process with a pid different from creation_pid_
    if (thread_pid != creation_pid_) {
      setpriority(PRIO_PROCESS, thread_pid, 10);
    }
  }
  void background_thread::cpu_yield() {
    struct timespec ts = { 0, 10000 };		
    nanosleep(&ts,0);	
  }

  void background_thread::cpu_long_yield() {
    struct timespec ts = { 0, 10000000 };	
    nanosleep(&ts,0);				
  }

}

