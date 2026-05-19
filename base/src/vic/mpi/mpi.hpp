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
#ifndef VIC_MPI_HPP
#define VIC_MPI_HPP

#include <sl/buffer_serializer.hpp>
#include <mpi.h>

namespace vic {
  
  namespace mpi {

#define MPI_TRACE_OUT(X) SL_TRACE_OUT(X) << " ==<MPI_rank=" << vic::mpi::process_rank() << ">== "

#ifndef MPI_TRACE_LEVEL
#define MPI_TRACE_LEVEL 0
#endif

    /**
     *  Initialize the MPI execution environment
     */
    static inline void initialize(int* argc, char** argv[]) {
      MPI_Init(argc,argv);
    }
    
    /**
     *  Finalize the MPI execution environment. All  processes must call this
     *  routine before exiting.  The number of processes running after this routine is
     *  called is undefined; it is best not to perform much more than a
     *  return rc after calling finalize.
     */
    static inline void finalize() {
      MPI_Finalize();
    }

    /*
     *  Returns the rank of the calling process in the world
     */
    static inline int process_rank() {
      int result;
      MPI_Comm_rank(MPI_COMM_WORLD,&result);
      return result;
    }
    
    /*
     *  Returns the rank of the number of processes in the world
     */
    static inline int process_count() {
      int result;
      MPI_Comm_size(MPI_COMM_WORLD,&result);
      return result;
    }

    /*
     *  Returns the rank of the name of the processor (similar to gethostname)
     */
    static inline std::string processor_name() {
      char buf[MPI_MAX_PROCESSOR_NAME+1];
      int sz;
      MPI_Get_processor_name(buf,&sz);
      buf[sz] = '\0';
      return std::string(buf);
    }
  
    /*
     * Point to point communication wrapper sending the message
     * contained in a serialization object 'x' tagged with 'tag'
     * to process 'destination'.
     */  
    static inline void send_buffer(int destination, int tag, const sl::output_buffer_serializer& msg) {
#if MPI_TRACE_LEVEL>0
      std::cerr << "### MPI SEND(" << process_rank() << " on " << processor_name() << " -> " << destination << ") TAG = " << tag << std::endl;
#endif
      int info = MPI_Send((void*)(msg.buffer_address()),
                          msg.buffer_size(),
                          MPI_BYTE,
                          destination,
                          tag,
                          MPI_COMM_WORLD);
      if (info != 0) {
        SL_FAIL("FIXME - Error when sending MPI message");
      }
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of object 'x' tagged with 'tag'
     * to process 'destination'.
     */  
    template <class T>
    inline void send(int destination, int tag, const T& x) {
      sl::output_buffer_serializer msg; msg << x;
      vic::mpi::send_buffer(destination, tag, msg);
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of object 'x' tagged with 'tag'
     * to processes 'destination_lo'...'destination_hi' .
     */  
    template <class T>
    inline void send_range(int destination_lo, int destination_hi, int tag, const T& x) {
      sl::output_buffer_serializer msg; msg << x;
      for (int destination=destination_lo; destination<=destination_hi; ++destination) {
	vic::mpi::send_buffer(destination, tag, msg);
      }
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of objects 'x0,x1' tagged with 'tag'
     * to process 'destination'.
     */  
    template <class T0, class T1>
    inline void send(int destination, int tag,
                     const T0& x0,
                     const T1& x1) {
      sl::output_buffer_serializer msg;
      msg << x0 << x1;
      vic::mpi::send_buffer(destination, tag, msg);
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of objects 'x0,x1' tagged with 'tag'
     * to processes 'destination_lo'...'destination_hi' .
     */  
    template <class T0, class T1>
    inline void send_range(int destination_lo, 
			   int destination_hi, 
			   int tag,
			   const T0& x0,
			   const T1& x1) {
      sl::output_buffer_serializer msg;
      msg << x0 << x1;
      for (int destination=destination_lo; destination<=destination_hi; ++destination) {
	vic::mpi::send_buffer(destination, tag, msg);
      }
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of objects 'x0,x1,x2' tagged with 'tag'
     * to process 'destination'.
     */  
    template <class T0, class T1, class T2>
    inline void send(int destination, int tag,
                     const T0& x0,
                     const T1& x1,
                     const T2& x2) {
      sl::output_buffer_serializer msg;
      msg << x0 << x1 << x2;
      vic::mpi::send_buffer(destination, tag, msg);
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of objects 'x0,x1,x2' tagged with 'tag'
     * to processes 'destination_lo'...'destination_hi' .
     */  
    template <class T0, class T1, class T2>
    inline void send_range(int destination_lo, 
			   int destination_hi,
			   int tag,
			   const T0& x0,
			   const T1& x1,
			   const T2& x2) {
      sl::output_buffer_serializer msg;
      msg << x0 << x1 << x2;
      for (int destination=destination_lo; destination<=destination_hi; ++destination) {
	vic::mpi::send_buffer(destination, tag, msg);
      }
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of objects 'x0,x1,x2,x3' tagged with 'tag'
     * to process 'destination'.
     */  
    template <class T0, class T1, class T2, class T3>
    inline void send(int destination, int tag,
                     const T0& x0,
                     const T1& x1,
                     const T2& x2,
                     const T3& x3) {
      sl::output_buffer_serializer msg;
      msg << x0 << x1 << x2 << x3;
      vic::mpi::send_buffer(destination, tag, msg);
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of objects 'x0,x1,x2,x3' tagged with 'tag'
     * to processes 'destination_lo'...'destination_hi' .
     */  
    template <class T0, class T1, class T2, class T3>
    inline void send_range(int destination_lo, 
			   int destination_hi,
			   int tag,
			   const T0& x0,
			   const T1& x1,
			   const T2& x2,
			   const T3& x3) {
      sl::output_buffer_serializer msg;
      msg << x0 << x1 << x2 << x3;
      for (int destination=destination_lo; destination<=destination_hi; ++destination) {
	vic::mpi::send_buffer(destination, tag, msg);
      }
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of objects 'x0,x1,x2,x3,x4' tagged with 'tag'
     * to process 'destination'.
     */  
    template <class T0, class T1, class T2, class T3, class T4>
    inline void send(int destination, int tag,
                     const T0& x0,
                     const T1& x1,
                     const T2& x2,
                     const T3& x3,
                     const T4& x4) {
      sl::output_buffer_serializer msg;
      msg << x0 << x1 << x2 << x3 << x4;
      vic::mpi::send_buffer(destination, tag, msg);
    }

    /*
     * Point to point communication wrapper sending a message
     * containing a serialization of objects 'x0,x1,x2,x3,x4' tagged with 'tag'
     * to processes 'destination_lo'...'destination_hi' .
     */  
    template <class T0, class T1, class T2, class T3, class T4>
    inline void send_range(int destination_lo, 
			   int destination_hi,
			   int tag,
			   const T0& x0,
			   const T1& x1,
			   const T2& x2,
			   const T3& x3,
			   const T4& x4) {
      sl::output_buffer_serializer msg;
      msg << x0 << x1 << x2 << x3 << x4;
      for (int destination=destination_lo; destination<=destination_hi; ++destination) {
	vic::mpi::send_buffer(destination, tag, msg);
      }
    }
    
    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization object 'msg' tagged with 'tag'
     * from process 'source'.
     */  
    static inline void receive_buffer(int source, int tag, MPI_Status& status, sl::input_buffer_serializer& msg) {
      int info = MPI_Probe(source,tag,MPI_COMM_WORLD,&status);
      if (info != 0) {
        SL_FAIL("FIXME - Error when probing MPI message");
      }
      int cnt;
      MPI_Get_count(&status,MPI_BYTE,&cnt);
      msg.buffer().resize(cnt);

      info=MPI_Recv(msg.buffer_address(),
                    msg.buffer_size(),
                    MPI_BYTE,
                    status.MPI_SOURCE,
                    status.MPI_TAG,
                    MPI_COMM_WORLD,
                    &status);
#if MPI_TRACE_LEVEL>0
      std::cerr << "### MPI RECEIVE(" << process_rank() << " on " << processor_name() << " <- " << source << ") TAG = " << tag << std::endl;
#endif
      if (info != 0) {
        SL_FAIL("FIXME - Error when receiving MPI message");
      }
    }

    /*
     * Point to point communication wrapper discarding a message
     * tagged with 'tag' from process 'source'.
     */  
    inline void discard(int source, int tag, MPI_Status& status) {
      sl::input_buffer_serializer msg;
      vic::mpi::receive_buffer(source, tag, status, msg);
    }


    /*
     * Point to point communication wrapper discarding a message
     * tagged with 'tag' from process 'source'.
     */  
    inline void discard(int source, int tag) {
      MPI_Status status;
      vic::mpi::discard(source, tag, status);
    }

    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of object 'x' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T>
    inline void receive(int source, int tag, MPI_Status& status, T& x) {
      sl::input_buffer_serializer msg;
      vic::mpi::receive_buffer(source, tag, status, msg);
      msg >> x;
    }

    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of objects 'x0,x1' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T0, class T1>
    inline void receive(int source, int tag, MPI_Status& status, T0& x0, T1& x1) {
      sl::input_buffer_serializer msg;
      vic::mpi::receive_buffer(source, tag, status, msg);
      msg >> x0 >> x1;
    }

    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of objects 'x0,x1,x2' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T0, class T1, class T2>
    inline void receive(int source, int tag, MPI_Status& status, T0& x0, T1& x1, T2& x2) {
      sl::input_buffer_serializer msg;
      vic::mpi::receive_buffer(source, tag, status, msg);
      msg >> x0 >> x1 >> x2;
    }

    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of objects 'x0,x1,x2,x3' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T0, class T1, class T2, class T3>
    inline void receive(int source, int tag, MPI_Status& status, T0& x0, T1& x1, T2& x2, T3& x3) {
      sl::input_buffer_serializer msg;
      vic::mpi::receive_buffer(source, tag, status, msg);
      msg >> x0 >> x1 >> x2 >> x3;
    }

    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of objects 'x0,x1,x2,x3,x4' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T0, class T1, class T2, class T3, class T4>
    inline void receive(int source, int tag, MPI_Status& status, T0& x0, T1& x1, T2& x2, T3& x3, T4& x4) {
      sl::input_buffer_serializer msg;
      vic::mpi::receive_buffer(source, tag, status, msg);
      msg >> x0 >> x1 >> x2 >> x3 >> x4;
    }
  
    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of object 'x' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T>
    inline void receive(int source, int tag, T& x) {
      MPI_Status status;
      vic::mpi::receive(source, tag, status, x);
    }

    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of objects 'x0,x1' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T0, class T1>
    inline void receive(int source, int tag, T0& x0, T1& x1) {
      MPI_Status status;
      vic::mpi::receive(source, tag, status, x0, x1);
    }

    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of objects 'x0,x1,x2' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T0, class T1, class T2>
    inline void receive(int source, int tag, T0& x0, T1& x1, T2& x2) {
      MPI_Status status;
      vic::mpi::receive(source, tag, status, x0, x1, x2);
    }

    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of objects 'x0,x1,x2,x3' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T0, class T1, class T2, class T3>
    inline void receive(int source, int tag, T0& x0, T1& x1, T2& x2, T3& x3) {
      MPI_Status status;
      vic::mpi::receive(source, tag, status, x0, x1, x2, x3);
    }

    /*
     * Point to point communication wrapper receiving a message
     * containing a serialization of objects 'x0,x1,x2,x3,x4' tagged with 'tag'
     * from process 'source'.
     */  
    template <class T0, class T1, class T2, class T3, class T4>
    inline void receive(int source, int tag, T0& x0, T1& x1, T2& x2, T3& x3, T4& x4) {
      MPI_Status status;
      vic::mpi::receive(source, tag, status, x0, x1, x2, x3, x4);
    }

    /*
     * Nonblocking test for a message send by process 'source' and tagged with 'tag'.
     * Result.first is 1 if the message is ready to be received, 0 if it is not.
     * Result.second is a pair with the actual source and tag for the message.
     */  
    inline std::pair<bool, std::pair<int,int> > nonblocking_probe(int source = MPI_ANY_SOURCE,
                                                                  int tag = MPI_ANY_TAG) {
      MPI_Status status;
      int flag=0;
      int info = MPI_Iprobe(source,tag,MPI_COMM_WORLD,&flag, &status);
      if (info != 0) {
        SL_FAIL("FIXME - Error when probing MPI message");
      }
      if (flag) {
        return std::make_pair(true, std::make_pair(status.MPI_SOURCE, status.MPI_TAG));
      } else {
        return std::make_pair(false, std::make_pair(MPI_ANY_SOURCE, MPI_ANY_TAG));
      }
    }

    /*
     * Blocking test for a message send by process 'source' and tagged with 'tag'.
     * Result is a pair with the actual source and tag for the message.
     */  
    inline std::pair<int,int> probe(int source = MPI_ANY_SOURCE,
                                    int tag = MPI_ANY_TAG) {
#if MPI_TRACE_LEVEL>0
      std::cerr << "### MPI PROBE: " << process_rank() << " on " << processor_name() << " waiting for source " << source << " and TAG = " << tag << std::endl;
#endif
      MPI_Status status;
      int info = MPI_Probe(source,tag,MPI_COMM_WORLD,&status);
      if (info != 0) {
        SL_FAIL("FIXME - Error when probing MPI message");
      }
#if MPI_TRACE_LEVEL>0
      std::cerr << "### MPI PROBE: " << process_rank() << " on " << processor_name() << " received message from source " << source << " and TAG = " << tag << std::endl;
#endif
      return std::make_pair(status.MPI_SOURCE, status.MPI_TAG);
    }
  }
}

#endif
