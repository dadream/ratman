#include <vic/qxml/database.hpp>
#include <cstdlib>

namespace vic {
  namespace qxml {

    // Helpers for database class

    static inline void parse_path_forwards(const QString& path,
					   QString& tag,
					   QString& remainder,
					   QString& selector){
      assert (! path.startsWith("/")) ;
      tag  = path.section('/', 0, 0);
      remainder = path.section('/', 1);

      selector = tag.section('[', 1);
      tag = tag.section('[', 0, 0);
      assert(! tag.isEmpty());
      selector.truncate(selector.length() - 1);
    }

    static inline void parse_path_backwards(const QString& path, 
					    QString& prefix, 
					    QString& att_name){
      if (path.section('/', -1, -1).startsWith("@")) {
	prefix = path.section('/', 0, -2);
	att_name = path.section('/', -1, -1);
	att_name = att_name.remove(0, 1);
      } else {
	prefix = path;
      }
    }


    static enum SelType parse_selector(const QString& selector,
				       int& sel_idx,
				       QString& att_name,
				       QString& att_value){
      if(selector.isEmpty()) 
	return(NO_SELECTOR);

      if(selector == "last()") {
	sel_idx = 0;
	return(INDEX_REVERSE);
      }

      bool ok;
      int index = selector.toInt(&ok);
      if (ok) {
	sel_idx = index;
	if (index > -1) {
	  return(INDEX_DIRECT);
	} else {
	  index = -1 - index;
	  return(INDEX_REVERSE);
	}
      }
      // @foo='bar' or @foo="bar" 
      att_name  = selector.section('=', 0,0);
      att_value = selector.section('=', 1);
      att_value = att_value.remove(0,1);
      att_value.truncate(att_value.length() - 1);

      assert(att_name.isEmpty() || att_name.startsWith("@"));
      att_name = att_name.remove(0,1);

      if(att_value.isEmpty()) {
	return NAME_ONLY;
      } else {
	return NAME_VALUE;
      }
      // will never get here.
      assert(0);
      return UNKNOWN;
    }
			      
    // FIXME why do I have to remove the const?
    static inline bool compatible_node(/* const */ QDomNode& node,
				       const QString& tag,
				       int sel_type,
				       int sel_idx,
				       int counter,
				       const QString& att_name,
				       const QString& att_value){
      bool res = ((node.isElement() && node.nodeName() == tag) 
		  &&
		  (sel_type == NO_SELECTOR
		   ||((sel_type == INDEX_DIRECT || sel_type == INDEX_REVERSE)
		      && sel_idx == counter)
		   ||(sel_type == NAME_ONLY 
		      && node.toElement().hasAttribute(att_name))
		   ||(sel_type == NAME_VALUE 
		      && (node.toElement().hasAttribute(att_name)
			  &&
			  att_value == node.toElement().attribute(att_name)))));
      return res;
    }

    static inline QDomNode find_node_at_xpath(const QDomNode& root, 
					      const QString& path, 
					      bool& found, 
					      QString &path_left,
					      bool do_count, 
					      int& counts) {
      if (path.isEmpty()) {
	found = true;
	return root;
      } 
  
      QString tag, remainder, selector;
      parse_path_forwards(path, tag, remainder, selector);

      QString att_name, att_value;
      int sel_idx = 0;
      enum SelType sel_type = parse_selector(selector, sel_idx, 
					     att_name, att_value);

      bool count_mode_enabled = do_count && remainder.isEmpty();
      bool reverse = (sel_type == INDEX_REVERSE) ;
      int element_counter  = 0;
      int selected_counter = 0;

      for (QDomNode child = (reverse)? root.lastChild() : root.firstChild();
	   !child.isNull();
	   child = (reverse)? child.previousSibling() : child.nextSibling()){
	if (compatible_node(child, tag, sel_type, sel_idx, element_counter, att_name, att_value)){
	  if (count_mode_enabled) {
	    selected_counter++;
	  } else {
	    return find_node_at_xpath(child, remainder, found, path_left, do_count, counts);
	  }
	}
	element_counter++;
      }
      found = count_mode_enabled;
      counts = selected_counter;
      path_left = path;
      return root;
    }

    static inline QDomNode find_node_at_xpath(const QDomNode& root, 
					      const QString& path, 
					      bool& found) {
      int counts = 0;
      QString dummy;
      return find_node_at_xpath(root, path, found, dummy, false, counts);
    }

    static inline QDomNode build_descendent(QDomNode& node, 
					    const QString& path){
      if (path.isEmpty()) {
	return node;
      }
      QString tag, remainder, selector;
      parse_path_forwards(path, tag, remainder, selector);

      QString att_name, att_value;
      int sel_idx = 0;
      enum SelType sel_type = parse_selector(selector, sel_idx, 
					     att_name, att_value);

      QDomDocument doc = node.ownerDocument();
      QDomNode child = doc.createElement(tag);
      node.appendChild(child);

      switch(sel_type) {
      case  NO_SELECTOR:
	break;
      case NAME_ONLY:
      case NAME_VALUE:
	node.toElement().setAttribute(att_name, att_value);
	break;
      case INDEX_DIRECT:
	{
	  for(int i = 0 ; i < sel_idx ; i++) {
	    child = doc.createElement(tag);
	    node.appendChild(child);
	  }
	}
	break;
      case INDEX_REVERSE:
      case UNKNOWN:
      default:
	assert(0);
      }
      return build_descendent(child, remainder);
    }

    static inline QDomNode find_create_node_at_xpath(const QDomNode& root,
						     const QString& path){
      bool found;
      int counts = 0;
      QString remainder;
      QDomNode node = find_node_at_xpath(root, path, found, remainder, 
					 false, counts);

      return (found)? node : build_descendent(node, remainder);
    }


    // Class vi::qxml::database implementation

    void database::save(const char* filename) {
      QFile f(filename);
      if ( f.open(QIODevice::WriteOnly)) {
	mutex_.lock();
	QString xml = dom_->toString();
	mutex_.unlock();
	QTextStream stream(&f);
	stream << xml;
	f.close();
      } else {
	std::cerr << "database[ERROR]: Cannot write to file " << filename << std::endl;
      }
    }

    int database::count(const char *key) const {
      int counts = 0;
      const QDomNode& root = dom_->documentElement();
      QString path(key);
      QString path_left;
      bool found;
      find_node_at_xpath(root, path, found, path_left, true, counts);
      return counts;
    }

    int database::get_int(const QString& key)  const{
      QString svalue = get_qstring(key);
      bool ok;
      int ival = svalue.toInt(&ok);
      if (!ok) {
	std::cerr << "database[ERROR]: Cannot get integer at " << ( key.toLatin1().data() ) << std::endl;
      }
      return ival;
    }



    float database::get_float(const QString& key) const {
      QString svalue = get_qstring(key);
      bool ok;
      float fval = svalue.toFloat(&ok);
      if (!ok) {
	std::cerr << "database[ERROR]: Cannot get float at " << key.data() << std::endl;
      }
      return fval;
    }

    /// Get qstring associated to key (isNull() if not found)
    QString database::get_qstring(const QString& path) const {
      QString prefix;
      QString att_name;
      QString value;
      bool found;
      parse_path_backwards(path, prefix, att_name);

      const QDomNode& root = dom_->documentElement();

      mutex_.lock();
      QDomNode node = find_node_at_xpath(root, prefix, found);
      if (found) {
	if (!att_name.isEmpty()) {
	  value = node.toElement().attribute(att_name);
	} else {
	  value =  node.toElement().text();    
	}
      } else {
	std::cerr << "database[ERROR]: Cannot get string at >" << path.data() << "<." << std::endl;
      }
      mutex_.unlock();
      return value;
    }


    float database::get_value_with_units(QString path, QString required_units) const {
    
      if   ( !expected_units(path, required_units) ) {
	std::cerr << "Wrong units for " << path .toStdString()
		  << " it should be " << required_units.toStdString()
		  << " found " << get_qstring(path + "/@units").toStdString() << std::endl;
	abort();
      }
      return get_float( path );
    }
  
    void database::set_qstring(const char *key, const QString& value) {
      QString path(key);
      QString prefix;
      QString att_name;
      parse_path_backwards(path, prefix, att_name);

      const QDomNode& root = dom_->documentElement();

      mutex_.lock();
      QDomNode node = find_create_node_at_xpath(root, prefix);

      if (!att_name.isEmpty()) {
	node.toElement().setAttribute(att_name, value);
      } else {
	if(node.hasChildNodes()) {
	  QDomNode child = node.firstChild();
	  assert(child.isText());
	  child.setNodeValue(value);
	  assert(child.nextSibling().isNull());
	} else {
	  QDomDocument doc = node.ownerDocument();
	  QDomNode child = doc.createTextNode(value);
	  node.appendChild(child);
	}
      }
      mutex_.unlock();
    }

    void database::unregister_receiver_callbacks(const void* receiver) {
      mutex_.lock();
      for (std::set<callback_t*>::iterator it = callbacks_.begin(); it != callbacks_.end() ;) {
	if ((*it)->receiver() == receiver) {
	  callbacks_.erase(it);
	} else {
	  ++it;
	}
      }
      mutex_.unlock();
    }

    void database::delete_receiver_callbacks(const void* receiver) {
      mutex_.lock();
      for (std::set<callback_t*>::iterator it = callbacks_.begin(); it != callbacks_.end() ;) {
	if ((*it)->receiver() == receiver) {
	  callback_t* objectp = *it;
	  callbacks_.erase(it);
	  delete(objectp);
	} else {
	  ++it;
	}
      }
      mutex_.unlock();
    }

    void database::propagate_changes(void *activator) {
      for (std::set<callback_t*>::iterator it = callbacks_.begin(); it != callbacks_.end() ; ++it) {
	if ((*it)->receiver() != activator) {
	     // assert(!mutex_.locked());
	  
	  (*it)->execute(this);
	}
      }
    }

    void database::load_document(const char* filename) {
      filename_ = QString(filename);

      QFile f(filename);
      if ( !f.open(QIODevice::ReadOnly)) {
	std::cerr << "database[ERROR]: Cannot open file " << filename << " for reading " << std::endl;
	abort();
      }
      if ( !dom_->setContent(&f)) {
	std::cerr << "database[ERROR]: Cannot read file " << filename << std::endl;
	abort();
      }
      f.close();
    }


  }
}
