// -*-c++-*-
#ifndef VIC_QXML_DATABASE_HPP_
#define VIC_QXML_DATABASE_HPP_

#include <QDomDocument>
#include <QDomNode>
#include <QMutex>

#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>


#include <set>
#include <assert.h>

#include <iostream>
#include <stdio.h>

namespace vic {
  namespace qxml {

    class database;

    class database_callback {
    public:
      database_callback(const QString& name) {
	name_ = name;
      }
      virtual ~database_callback() {}
  
      virtual void  execute(const database* db)  = 0;
      virtual const void* receiver() const  = 0;
      const QString& name() const { return name_;}

    protected:
      QString name_;
    };


    class database_callback_functor : public database_callback {
    public:
      typedef  void (*functor_t)(void* receiver, const database* db);

    public:
      database_callback_functor(const QString& name, void* receiver, functor_t fun) : database_callback(name) {
	receiver_ = receiver;
	function_ = fun;
      }
      virtual  void   execute(const database *db) { (*function_)(receiver_, db); }
      const void*     receiver() const { return receiver_; }
      const functor_t function() const { return function_; }

    protected:
      void* receiver_;
      functor_t function_;
    };


    /*

    Supported PATHS
 
    /data/section/velocity        
    /data/section/velocity/@units 
    /data/section/velocity[last()]/foo
    /data/section/velocity[0]/foo
    /data/section/velocity[10]/bar
    /data/section/velocity[@units='_s']
    /data/section/velocity[@set]
    */

    enum SelType { NO_SELECTOR, INDEX_DIRECT, INDEX_REVERSE, 
		   NAME_ONLY, NAME_VALUE, 
		   UNKNOWN };

    // next time we factor out the generic ParameterDataBase.
    class database {
    public:
      typedef database_callback callback_t;
  
    public: // Construction methods

      database(const char* name, const char* filename)  {
	dom_ = new QDomDocument(name);
	load_document(filename);
      }

      ~database() {
	delete dom_;
      }

      void reload_from_file()  {
	reload_from_file(filename().toLatin1());
      }

      void reload_from_file(const char* filename)  {
	mutex_.lock();
	load_document(filename);
	mutex_.unlock();
      }

    public: // Save methods
      void save(const char* filename);

    public: // Queries

      const QString& filename() const {
	return filename_;
      }
  
      int count(const char *key) const; 

      int get_int(const char *path) const {
	QString key = QString::fromLatin1(path);
	return get_int( key );
      }

      int get_int(const QString& key) const;

      float get_float(const char* path) const {
	QString key = QString::fromLatin1(path);
	return get_float( key );
      }

      float get_float(const QString& key) const;

      void get_string(const char *key, char *buffer) const;


      /// Get qstring associated to key (isNull() if not found)
      QString  get_qstring(const char* key) const {
	QString path = QString::fromLatin1(key);
	return get_qstring( path );
      }

      QString get_qstring(const QString& path) const;

      bool  expected_units(QString path, QString required_units) const {
	return  get_qstring(path + "/@units") == required_units  ;
      }

      float get_value_with_units(QString path, QString required_units) const;

    public:

      inline void set_int(const char *key, int v ) {
	set_int( key, v, "%d");
      }

      inline void set_float(const char *key, float v ) {
	set_float( key, v, "%g");
      }
      inline void set_string(const char *key, const char *v) {
	QString value(v);
	set_qstring(key, value);
      }
  
      void set_qstring(const char *key, const QString& value);


    public:

      void register_callback(callback_t *callback) {
	mutex_.lock();
	callbacks_.insert(callback);
	mutex_.unlock();
      }

      void unregister_callback(callback_t *callback) {
	mutex_.lock();
	callbacks_.erase(callback);
	mutex_.unlock();
      }

      void unregister_receiver_callbacks(const void* receiver);

      void delete_receiver_callbacks(const void* receiver);

      void propagate_changes(void *activator = 0);

  
    protected:
      void load_document(const char* filename);

      // Protected function
      inline void set_int(const char *key, int v, const char * format ) {
	char  number[32];
	sprintf( number, format, v);
	set_qstring(key, QString( number ));
      }


      inline void set_float(const char *key, float v, const char * format) {
	char  number[32];
	sprintf( number, format, v);
	set_qstring(key, QString( number) );
      }

    protected:
      mutable QMutex mutex_;

      QDomDocument*         dom_;
      std::set<callback_t*> callbacks_;
      QString filename_;
    };

  }
}



#endif //VIC_QXML_DATABASE_HPP
