#ifndef VIC_PERSISTENT_MAP_HPP
#define VIC_PERSISTENT_MAP_HPP

#include <sl/cstdint.hpp>
#include <sl/utility.hpp>
#include <sl/operators.hpp>
#include <sl/buffer_serializer.hpp>
#include <sl/os_file.hpp>
#include <cassert>
#include <db.h> // Berkeley db

namespace vic {

#define DB_EXEC_OR_FAIL(dbf) {\
  int __db_ret__ = (dbf);\
  if (__db_ret__) {\
    SL_TRACE_OUT(-1) << "DB Fatal Error: " << #dbf << " returns " << db_strerror(__db_ret__) << std::endl;\
    abort();\
  }\
}

#define DB_EXEC_OR_WARN(dbf) {\
  int __db_ret__ = (dbf);\
  if (__db_ret__) {\
    SL_TRACE_OUT(-1) << "DB Warning: " << #dbf << " returns " << db_strerror(__db_ret__) << std::endl;\
  }\
}
  
  // ========================== Forward declarations
  
  namespace persistent {
    
    template <class Key, class Data, class Compare>
    class map;

    template <class Key, class Data, class Compare>
    class map_data_reference;
    
    template <class Key, class Data, class Compare>
    class map_iterator;

    template <class Key, class Data, class Compare>
    class map_const_iterator;

    namespace detail {

      static inline DBT dbt_make() {
        DBT result;
        memset(&result, 0, sizeof(result));
        return result;
      }
        
      static inline DBT dbt_make(sl::output_buffer_serializer& s) {
        DBT result = dbt_make();
        result.size = s.buffer().size();
        result.data = s.buffer_address();
        return result;
      }

    }
  }
  

  // ========================== Reference to data
  
  namespace persistent {
      
    template <class Key, class Data, class Compare>
    class map_data_reference {
    public:
      typedef map_data_reference<Key, Data, Compare>     this_t;
      typedef std::bidirectional_iterator_tag            iterator_category; 
      
      typedef Key                                        key_type;
      typedef Data                                       data_type;
      typedef Compare                                    key_compare;
      
    protected:
      DB*  db_;
      DBC* cursor_;
      
    public:

      inline map_data_reference() :
          db_(0), cursor_(0) {
      }
      
      inline map_data_reference(DB* db, const DBC* cursor) {
        assert(db);
        assert(cursor);
        db_ = db;
        DB_EXEC_OR_FAIL(cursor->c_dup(const_cast<DBC*>(cursor), &cursor_, DB_POSITION));
      }

      inline ~map_data_reference() {
        if (cursor_) {
          DB_EXEC_OR_WARN(cursor_->c_close(cursor_));
          cursor_ = 0;
        }
        db_ = 0;
      }
      
      inline map_data_reference(const this_t& other) {
        assert(other.db_);
        assert(other.cursor_);
        db_ = other.db_;
        DB_EXEC_OR_FAIL(other.cursor_->c_dup(const_cast<DBC*>(other.cursor_), &cursor_, DB_POSITION));
        
      }

      inline this_t& operator=(const this_t& other) {
        assert(other.db_);
        assert(other.cursor_);

        if (cursor_) {
          DB_EXEC_OR_WARN(cursor_->c_close(cursor_));
          cursor_ = 0;
        }
        db_ = other.db_;
        DB_EXEC_OR_FAIL(other.cursor_->c_dup(const_cast<DBC*>(other.cursor_), &cursor_, DB_POSITION));

        return *this;
      }
      
    public: // Accessors
      
      inline data_type data() const {
        assert(db_);
        assert(cursor_);
        
        DBT db_key = detail::dbt_make();
        DBT db_data = detail::dbt_make();
        DB_EXEC_OR_FAIL(cursor_->c_get(cursor_, &db_key, &db_data, DB_CURRENT));

        // Decode and return
        sl::input_buffer_serializer codec;
        codec.buffer().resize(db_data.size);
        memcpy(codec.buffer_address(), db_data.data, db_data.size);
        data_type result;
        codec >> result;
        return result;
      }
      
      inline operator data_type() const {
        return data();
      }

    public: // Database update
      
      inline this_t& operator=(const data_type& x) {
        assert(db_);
        assert(cursor_);
        
        sl::output_buffer_serializer data_serializer;
        data_serializer << x;
        DBT db_data = detail::dbt_make(data_serializer);
        DBT db_key = detail::dbt_make(); // ignored
        
        DB_EXEC_OR_FAIL(cursor_->c_put(cursor_, &db_key, &db_data, DB_CURRENT));
        return *this;
      }
      
    }; // class map_data_reference
  }
  
  // ========================== Const Iterators

  namespace persistent {

    template <class Key, class Data, class Compare>
    class map_const_iterator {
    public:
      typedef map_const_iterator<Key, Data, Compare> this_t;
      typedef std::bidirectional_iterator_tag        iterator_category; 

      typedef Key                                    key_type;
      typedef Data                                   data_type;
      typedef Compare                                key_compare;
      typedef this_t                                 value_type;

      typedef const map<Key,Data,Compare>            map_t;
    protected:
      DB*  db_;
      DBC* cursor_;
      
    protected: 

      virtual void db_make_cursor() {
        assert(db_);
        assert(!cursor_);
        DB_EXEC_OR_FAIL(db_->cursor(db_, 0, &cursor_, 0));
      }

      virtual void db_release_cursor() {
        assert(db_);
        assert(cursor_);
        DB_EXEC_OR_WARN(cursor_->c_close(cursor_));
        cursor_ = 0;
      }
      
    public: // Creating

      inline map_const_iterator() :
          db_(0), cursor_(0) {
      }

      /// Create a cursor to begin (bgn=true) or end (bgn=false) of database
      inline map_const_iterator(DB* db, bool bgn)
          : db_(db), cursor_(0) {
        assert(db_);

        // Default is end
        if (bgn) {
          // Begin
          db_make_cursor();
          
          DBT db_key = detail::dbt_make();
          DBT db_data = detail::dbt_make();
          int ret = cursor_->c_get(cursor_, &db_key, &db_data, DB_FIRST);
          if (ret) { // Assume database empty
            db_release_cursor();
          }
        } 
      }
      
      /// Create a cursor pointing to or before key
      inline map_const_iterator(DB* db, const key_type& k, u_int32_t flag)
      : db_(db), cursor_(0) {
        assert(db_);
        
        db_make_cursor();
        
        sl::output_buffer_serializer key_serializer;
        key_serializer << k;
        DBT db_key = detail::dbt_make(key_serializer);
        DBT db_data = detail::dbt_make(); // ignored
        
        int ret = cursor_->c_get(cursor_, &db_key, &db_data, flag);
        if (ret) {
          // Assume not found - set to end()
          db_release_cursor();
        }
      }

      
      /// Destruct, releasing cursor
      inline virtual ~map_const_iterator() {
        if (cursor_) {
          db_release_cursor();
        }
        db_ = 0;
      }

      /// Copy
      inline map_const_iterator(const map_const_iterator& other) {
        db_ = other.db_;
        cursor_ = 0;
        if (other.cursor_) {
          DB_EXEC_OR_FAIL(other.cursor_->c_dup(other.cursor_, &cursor_, DB_POSITION));
        }
      }

      /// Assignment
      inline this_t& operator=(const this_t& other) {
        if (cursor_) {
          DB_EXEC_OR_WARN(cursor_->c_close(cursor_));
          cursor_ = 0;
        }
        db_ = other.db_;
        cursor_ = 0;
        if (other.cursor_) {
          DB_EXEC_OR_FAIL(other.cursor_->c_dup(other.cursor_, &cursor_, DB_POSITION));
        }
        return *this;
      }
      
    public: // Moving

      inline this_t& operator++() {
        if (cursor_) {
          DBT db_key = detail::dbt_make();
          DBT db_data = detail::dbt_make();
          int ret = cursor_->c_get(cursor_, &db_key, &db_data, DB_NEXT);
          if (ret) {
            // Assume at end
            DB_EXEC_OR_WARN(cursor_->c_close(cursor_));
            cursor_ = 0;
          }
        } 
        return *this;
      }

      inline this_t& operator--() {
        if (!cursor_) {
          // At end, create cursor and move to LAST
          db_make_cursor();
          
          DBT db_key = detail::dbt_make();
          DBT db_data = detail::dbt_make();
          int ret = cursor_->c_get(cursor_, &db_key, &db_data, DB_LAST);
          if (ret) {
            //Assume empty
            db_release_cursor();
          }              
        } else {
          // Somewhere, go to prev if not at begin
          DBT db_key = detail::dbt_make();
          DBT db_data = detail::dbt_make();
          int ret = cursor_->c_get(cursor_, &db_key, &db_data, DB_PREV);
          if (ret) {
            // Assume was at begin
            db_release_cursor();
          }
        }
        return *this;
      }

      SL_OP_INCREMENTABLE(this_t);
      SL_OP_DECREMENTABLE(this_t);
      
    public: // Pair representation, update only when dereferencing!!!

      mutable key_type  first;
      mutable data_type second;

    protected:

      virtual void dereference() const {
        assert(cursor_);
        DBT db_key = detail::dbt_make();
        DBT db_data = detail::dbt_make();
        DB_EXEC_OR_FAIL(cursor_->c_get(cursor_, &db_key, &db_data, DB_CURRENT));        
        
        sl::input_buffer_serializer codec;
        codec.buffer().resize(db_key.size+db_data.size);
        memcpy(codec.buffer_address(),             db_key.data,  db_key.size);
        memcpy(codec.buffer_address()+db_key.size, db_data.data, db_data.size);
        codec >> (this->first);
        codec >> (this->second);
      }
      
    public: // Deref

      inline const value_type& operator* () const {
        dereference();        
        return *this;
      }

      inline const value_type* operator -> () const {
        dereference();
        return this;
      }

    public: // Comparison

      inline bool operator==(const this_t& other) const {
        if (cursor_ == 0) {
          return (other.cursor_ == 0);
        } else if (other.cursor_ == 0) {
          return false;
        } else {
          // Compare keys
          this->dereference();
          other.dereference();
          
          return (this->first) == (other.first);
        }
      }
    
      inline bool operator<(const this_t& other) const {
        if (cursor_ == 0) {
          // At end, can never be < other
          return false;
        } else if (other.cursor_ == 0) {
          // Other at end
          return true;
        } else {
          // Compare keys using map's key_compare;
          this->dereference();
          other.dereference();
          
          const map_t* db_map = static_cast<const map_t*>(db_->app_private);
          assert(db_map);
          return db_map->key_comp().operator()(this->first, other.first);
        }
      }

      SL_OP_COMPARABLE1(this_t);
      SL_OP_EQUALITY_COMPARABLE1(this_t);
      
    }; // class map_const_iterator
    
  } // namespace persistent

  
  // ========================== Iterators

  namespace persistent {

    template <class Key, class Data, class Compare>
    class map_iterator: public map_const_iterator<Key, Data, Compare> {
    public:
      typedef map_iterator<Key, Data, Compare>           this_t;
      typedef map_const_iterator<Key, Data, Compare>     super_t;
      typedef std::bidirectional_iterator_tag  iterator_category; 
      
      typedef Key                                        key_type;
      typedef Data                                       data_type;
      typedef Compare                                    key_compare;
      typedef this_t                                     value_type;

      typedef map<Key,Data,Compare>                      map_t;

      typedef map_data_reference<Key,Data,Compare>       data_reference_t;
            
    protected:

      virtual void db_make_cursor() {
        assert(this->db_);
        assert(!(this->cursor_));
        DB_EXEC_OR_FAIL(this->db_->cursor(this->db_, 0, &(this->cursor_), 0/*DB_WRITECURSOR*/));
      }

      virtual void db_release_cursor() {
        assert(this->db_);
        assert(this->cursor_);
        DB_EXEC_OR_WARN(this->cursor_->c_close(this->cursor_));
        this->cursor_ = 0;
      }
      
    public: // Creating

      /// Create a cursor to begin (bgn=true) or end (bgn=false) of database
      inline map_iterator(DB* db, bool bgn) {
        this->db_ = db;
        this->cursor_ = 0;
        // Default is end
        if (bgn) {
          // Begin
          db_make_cursor();
          
          DBT db_key = detail::dbt_make();
          DBT db_data = detail::dbt_make();
          int ret = this->cursor_->c_get(this->cursor_, &db_key, &db_data, DB_FIRST);
          if (ret) { // Assume database empty
            db_release_cursor();
          }
        } 
      }
      
      /// Create a cursor pointing to or before key
      inline map_iterator(DB* db, const key_type& k, u_int32_t flag)  {
        this->db_ = db;
        this->cursor_ = 0; 

        db_make_cursor();
        
        sl::output_buffer_serializer key_serializer;
        key_serializer << k;
        DBT db_key = detail::dbt_make(key_serializer);
        DBT db_data = detail::dbt_make(); // ignored
        
        int ret = this->cursor_->c_get(this->cursor_, &db_key, &db_data, flag);
        if (ret) {
          SL_TRACE_OUT(1) << "NOT FOUND: " << db_strerror(ret) << std::endl;
          // Assume not found - set to end()
          // check DB_NOTFOUND or DB_KEYEMPTY
          db_release_cursor();
        } else {
          SL_TRACE_OUT(1) << "FOUND" << std::endl;
        }
      }

      
      /// Destruct, releasing cursor
      inline ~map_iterator() {
        if (this->cursor_) {
          db_release_cursor();
        }
      }

      /// Copy
      inline map_iterator(const map_iterator& other) :
          super_t() {
        this->db_ = other.db_;
        this->cursor_ = 0;
        if (other.cursor_) {
          DB_EXEC_OR_FAIL(other.cursor_->c_dup(other.cursor_, &(this->cursor_), DB_POSITION));
        }
      }

      /// Assignment
      inline this_t& operator=(const this_t& other) {
        if (this->cursor_) {
          DB_EXEC_OR_WARDN(this->cursor_->c_close(this->cursor_));
          this->cursor_ = 0;
        }
        this->db_ = other.db_;
        if (other.cursor_) {
          DB_EXEC_OR_FAIL(other.cursor_->c_dup(other.cursor_, &(this->cursor_), DB_POSITION));
        }
        return *this;
      }

      
    public: // Erase

      void db_erase() {
        if (this->cursor_) {
          DBC* erasing_cursor;
          DB_EXEC_OR_FAIL(this->cursor_->c_dup(this->cursor_, &erasing_cursor, DB_POSITION));
          this->operator++();
          erasing_cursor->c_del(erasing_cursor, 0);
          DB_EXEC_OR_WARN(erasing_cursor->c_close(erasing_cursor));
          erasing_cursor=0;
        }
      }

    public: // Pair representation, update only when dereferencing!!!

      mutable data_reference_t second;

    protected:

      virtual void dereference() const {
        assert(this->db_);
        assert(this->cursor_);
        DBT db_key = detail::dbt_make();
        DBT db_data = detail::dbt_make();
        DB_EXEC_OR_FAIL(this->cursor_->c_get(this->cursor_, &db_key, &db_data, DB_CURRENT));
        
        
        sl::input_buffer_serializer codec;
        codec.buffer().resize(db_key.size+db_data.size);
        memcpy(codec.buffer_address(), db_key.data, db_key.size);
        memcpy(codec.buffer_address()+db_key.size, db_data.data, db_data.size);
        codec >> this->first; // Key
        codec >> super_t::second; // Const data
        this_t::second = data_reference_t(this->db_, this->cursor_); // Assignable data
      }
      
    public: // Deref

      inline const value_type& operator* () const {
        assert(this->db_);
        assert(this->cursor_);

        dereference();        
        return *this;
      }

      const value_type* operator -> () const {
        assert(this->db_);
        assert(this->cursor_);

        dereference();
        return this;
      }
      
      inline value_type& operator* () {
        assert(this->db_);
        assert(this->cursor_);

        dereference();

        return *this;
      }
      
      inline value_type* operator -> () {
        assert(this->db_);
        assert(this->cursor_);

        dereference();
        return this;
      }
      
    }; // class map_iterator

  } // namespace persistent
  
  // ========================== Map
  
  namespace persistent {

    template <class Key,
              class Data,
              class Compare = std::less<Key> >
    class map {
    public:
      typedef map<Key,Data,Compare>                      this_t;

      typedef Key                                        key_type;
      typedef Data                                       data_type;
      typedef Compare                                    key_compare;
      typedef std::pair<Key,Data>                        value_type;

      typedef map_iterator<Key,Data,Compare>             iterator;
      typedef map_const_iterator<Key,Data,Compare>       const_iterator;

      typedef map_data_reference<Key,Data,Compare>       data_reference_t;
      typedef iterator                                   reference; // FIXME
      typedef const_iterator                             const_reference; // FIXME

      typedef sl::uint64_t                               size_type;
    protected:
      key_compare key_comp_;
      
      std::string db_fname_;
      DB*         db_;
      bool        db_creatable_;
      bool        db_writable_;
      bool        db_persistent_;
      bool        db_empty_at_creation_;
      
    protected:
            
      inline int db_compare(const DBT *db_key1, const DBT *db_key2) const {
        sl::input_buffer_serializer codec;
        codec.buffer().resize(db_key1->size+db_key2->size);

        memcpy(codec.buffer_address(),               db_key1->data, db_key1->size);
        memcpy(codec.buffer_address()+db_key1->size, db_key2->data, db_key2->size);
        key_type key1; codec >> key1;
        key_type key2; codec >> key2;
        
        if (key_comp_.operator()(key1,key2)) {
          return -1; // Key1<Key2
        } else if (key_comp_.operator()(key2,key1)) {
          return  1; // Key2>Key1
        } else {
          return 0;
        }
      }
      
      static int db_compare_cb(DB* db, const DBT *db_key1, const DBT *db_key2) {
        const this_t* db_map = static_cast<const this_t*>(db->app_private);
        assert(db_map);
        return db_map->db_compare(db_key1, db_key2);
      }
      
      void db_close() {
        if (db_) {
          if (is_persistent()) {
            DB_EXEC_OR_WARN(db_->close(db_, 0));
          } else {
#if 0
            std::cerr << "Removing database " << db_fname_ << std::endl;
            DB_EXEC_OR_WARN(db_->remove(db_, db_fname_.c_str(), NULL, 0));
#else
            std::cerr << "Removing database " << db_fname_ << std::endl;
            DB_EXEC_OR_WARN(db_->close(db_, 0));
            sl::os_file::file_delete(db_fname_.c_str());
#endif
          }
          db_ = 0;
        }
        assert(db_ == 0);
      }

      void db_open(const std::string& fname,
                   bool creatable, 
                   bool empty_at_creation,
                   bool writable,
                   bool persistent,
		   std::size_t expected_average_data_size = -1) {
        db_close();
        
        db_fname_ = fname;
        db_creatable_ = creatable;
        db_empty_at_creation_ = empty_at_creation;
        db_writable_ = writable;
        db_persistent_ = persistent;

        DB_EXEC_OR_FAIL(db_create(&db_, 0, 0));
        db_->app_private = this; 
        
        u_int32_t     flags = 0;
        int           mode = 0666;
        if (creatable) flags |= DB_CREATE;
        if (!writable) { flags |= DB_RDONLY; mode= 0444; }
        if (empty_at_creation) flags |= DB_TRUNCATE;

	if (writable && (expected_average_data_size != std::size_t(-1))) {
	  const std::size_t min_db_page_size =  4096;
	  const std::size_t max_db_page_size = 65536;

	  std::size_t db_page_size = min_db_page_size;
	  std::size_t chunk_size = (4+2) * expected_average_data_size;
	  while (db_page_size < chunk_size && db_page_size < max_db_page_size) db_page_size *= 2;
	  if (db_page_size > max_db_page_size) db_page_size = max_db_page_size;

	  SL_TRACE_OUT(1) << "Open write db '" << db_fname_ << "' with page size = " << db_page_size << std::endl;
	  db_->set_pagesize(db_, db_page_size);
	}
        
        db_->set_flags(db_, DB_RECNUM); // the count of keys will be exact
        db_->set_bt_compare(db_, db_compare_cb);

        int ret = db_->open(db_,
                            NULL,
                            db_fname_.c_str(), 
                            NULL,
                            DB_BTREE, 
                            flags,
                            mode);
        if (ret) {
          SL_TRACE_OUT(-1) << "unable to open db " << db_fname_ << ": " << db_strerror(ret) << std::endl;
          DB_EXEC_OR_WARN(db_->close(db_, 0)); // ??? FIXME is this sufficient to free created db_
          db_ = NULL;
        }
      }

      void db_truncate() {
        u_int32_t n;
        DB_EXEC_OR_WARN(db_->truncate(db_,0,&n,0));
      }

      iterator db_begin() {
        return iterator(db_, true); 
      }

      iterator db_end() {
        return iterator(db_, false); 
      }
      
      const_iterator db_begin() const {
        return const_iterator(db_, true);
      }

      const_iterator db_end() const {
        return const_iterator(db_, false);
      }

      iterator db_find_with(const key_type& k, u_int32_t flag) {
        return iterator(db_, k, flag);
      }

      const_iterator db_find_with(const key_type& k, u_int32_t flag) const {
        return iterator(db_, k, flag);
      }
      
    public:

      map(const std::string& fname,
          const std::string& mode,
	  std::size_t expected_average_data_size = -1, // Use defaults
          const key_compare& comp = key_compare()) :
          key_comp_(comp) {
        db_ = 0;
        db_fname_ = "";
        db_creatable_       = false;
        db_writable_        = false;
        db_persistent_      = false;
        db_empty_at_creation_ = false;

        bool creatable = false;
        bool writable = false;
        bool persistent = false;
        bool empty_at_creation = false;
        if (sl::matches(mode.c_str(), "r+*")) { // RW+Append, must exist
          creatable = false; writable=true; empty_at_creation=false; persistent=true;
        } else if (sl::matches(mode.c_str(), "r*")) { // RO, must exist
          creatable = false; writable=false; empty_at_creation=false; persistent=true;
        } else if (sl::matches(mode.c_str(), "w+*")) { // RW+Append
          creatable = true; writable=true; empty_at_creation=false; persistent=true;
        } else if (sl::matches(mode.c_str(), "w*")) { // RW, cleared 
          creatable = true; writable=true; empty_at_creation=true; persistent=true;
        } else if (sl::matches(mode.c_str(), "t*")) { // Temporary
          creatable = true; writable=true; empty_at_creation=true; persistent=false;
        }
        db_open(fname, creatable, empty_at_creation, writable, persistent, expected_average_data_size);
      }

      ~map() {
        db_close();
      }

      inline bool is_open() const {
        return db_ != NULL;
      }
      inline bool is_creatable() const {
        return db_creatable_;
      }

      inline bool is_writable() const {
        return db_writable_;
      }
      
      inline bool is_persistent() const {
        return db_persistent_;
      }

      inline bool is_empty_at_creation() const {
        return db_empty_at_creation_;
      }

      const std::string& file_name() const {
        return db_fname_;
      }
    
      inline void close() {
        db_close();
      }

      inline void reopen() {
        if (!is_open()) {
          db_open(file_name(),
                  is_creatable(),
                  is_empty_at_creation(),
                  is_writable(),
                  is_persistent());
        }
      }

    public: // Comparison

      inline const key_compare& key_comp() const {
        return key_comp_;
      }

    public: // Iteration

      iterator begin() {
        return db_begin();
      }

      iterator end() {
        return db_end();
      }
      
      const_iterator begin() const {
        return db_begin();
      }

      const_iterator end() const {
        return db_end();
      }

    public: // Size

      size_type max_size() const {
        return size_type(-1);
      }

      size_type size() const {
	SL_TRACE_OUT(1) << std::endl;
        DB_BTREE_STAT stats;
        DB_BTREE_STAT* statp = &stats;
#if (DB_VERSION_MAJOR>=4) ||  ((DB_VERSION_MAJOR>=4) && (DB_VERSION_MINOR>=3)) 
        db_->stat(db_, 0, &statp, DB_FAST_STAT);
#else
        db_->stat(db_, &statp, DB_FAST_STAT);
#endif
        size_type result = statp->bt_nkeys;
        return result;
      }

      bool empty() const {
        return size()==0;
      }

    public: // Searching
      
      iterator find(const key_type& k) {
        return db_find_with(k,DB_SET);
      }

      const_iterator find(const key_type& k) const {
        return db_find_with(k,DB_SET);
      }

      size_type count(const key_type& k) const {
        size_type n=0;
        for (iterator i=find(k); i->first==k; ++i) ++n; 
        return n;
      }

      iterator lower_bound(const key_type& k) {
        return db_find_with(k,DB_SET_RANGE);
      }

      const_iterator lower_bound(const key_type& k) const {
        return db_find_with(k,DB_SET_RANGE);
      }

      iterator upper_bound(const key_type& k) {
        iterator it = db_find_with(k,DB_SET_RANGE);
        return ( it==end() || it->first!=k )? it: ++it;
      }
      
      const_iterator upper_bound(const key_type& k) const {
        const_iterator it = db_find_with(k,DB_SET_RANGE);
        return ( it==end() || it->first!=k )? it: ++it;
      }
      
      std::pair<iterator,iterator> equal_range(const key_type& k) {
        return std::make_pair(lower_bound(k),upper_bound(k));
      }

      std::pair<const_iterator,const_iterator> equal_range(const key_type& k) const {
        return std::make_pair(lower_bound(k),upper_bound(k));
      }

    public: // Insert

      std::pair<iterator, bool> insert(const std::pair<key_type,data_type>& x) {
        sl::output_buffer_serializer key_serializer;
        key_serializer << x.first;
        DBT db_key = detail::dbt_make(key_serializer);

        sl::output_buffer_serializer data_serializer;
        data_serializer << x.second;
        DBT db_data = detail::dbt_make(data_serializer);

        int ret = db_->put(db_, NULL, &db_key, &db_data, DB_NOOVERWRITE);
        bool inserted = (ret == 0); // FIXME CHECK errors?

        return std::make_pair(this->find(x.first),
                              inserted);
      }
      
      iterator insert(const iterator& pos, const std::pair<key_type,data_type>& x) {
        return this->insert(x).first;
      }
      
      template <class InputIterator>
      void insert(InputIterator bgn,
                  InputIterator end) {
        for (InputIterator it=bgn; it!=end; ++it) {
          this->insert(*it);
        }
      }

      data_reference_t operator[](const key_type& k) {
        std::pair<key_type, data_type> key_default = std::make_pair(k, data_type());
        std::pair<iterator, bool> it_inserted = this->insert(key_default);
        data_reference_t result = (*(it_inserted.first)).second;
        return result;
      }
      
    public: // Erase
      
      void erase(iterator& pos) {
        pos.db_erase();
      }

      void erase(iterator& bgn, iterator& end) {
        for (iterator it=bgn; it!=end; ++it) {
          erase(it);
        }
      }

      size_type erase(const key_type& k) {
        size_type result=0;
        for (iterator it=find(k); it!=end() && it->first==k; ++it) {
          erase(it);
          ++result;
        }
        return result;
      }

      void clear() {
        // Faster than erase(begin(),end());
        db_truncate();
      }
      
      
    }; // class map
    
  } // namespace persistent
  
} // namespace vic


#endif
