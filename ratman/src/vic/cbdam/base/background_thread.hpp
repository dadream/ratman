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
#ifndef CBDAM_BACKGROUND_THREAD_HPP
#define CBDAM_BACKGROUND_THREAD_HPP

#include <vic/cbdam/base/config.hpp>
#include <QThread>

namespace cbdam {
  
  /**
   * Thread with background priority.
   * Derived classes simply need to reimplement run which must call background_thread::run
   * before doing other stuff.
   * This class provides also 2 yield static functions to make a process sleep.
   */
  class background_thread : public QThread {
  public:
    //    typedef

  protected:
    int creation_pid_;

  public:
    background_thread(QObject * parent = 0);

    virtual ~background_thread();

    /// derived classes must call this run, then do their stuff.
    virtual void run() {
      set_background_priority();
    }

    /// sleep 1 ms
    static void cpu_yield();

    /// sleep 10 ms
    static void cpu_long_yield();

  protected:
    void set_background_priority();
  
  };


} // namespace cbdam 

#endif // CBDAM_BACKGROUND_THREAD_HPP

#ifndef CBDAM_BACKGROUND_THREAD_IPP
#define CBDAM_BACKGROUND_THREAD_IPP

namespace cbdam {

} // namespace cbdam 

#endif // CBDAM_BACKGROUND_THREAD_IPP
