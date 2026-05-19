##
##  Makefile -- Build procedure for fast3lpoad Apache module
##
##  This is a C++ module so things have to be handled a little differently.

#   the used tools

# Apache 1.0
APXS=/usr/sbin/apxs
APACHECTL=/usr/sbin/apachectl
APACHEMODS=/etc/httpd/modules

# Apache 2.0 
#APXS=/usr/sbin/apxs2
#APACHECTL=/usr/sbin/apache2ctl
#APACHEMODS=/etc/apache2/modules.d

# Get all of apxs's internal values.
APXS_CC=`$(APXS) -q CC`   
APXS_TARGET=`$(APXS) -q TARGET`   
APXS_CFLAGS=`$(APXS) -q CFLAGS`   
APXS_SBINDIR=`$(APXS) -q SBINDIR`   
APXS_CFLAGS_SHLIB=`$(APXS) -q CFLAGS_SHLIB`   
APXS_INCLUDEDIR=`$(APXS) -q INCLUDEDIR`   
APR_INCLUDEDIR=`$(APXS) -q APR_INCLUDEDIR`   
APU_INCLUDEDIR=`$(APXS) -q APU_INCLUDEDIR`   
APXS_LD_SHLIB=`$(APXS) -q LD_SHLIB`
APXS_LIBEXECDIR=`$(APXS) -q LIBEXECDIR`
APXS_LDFLAGS_SHLIB=`$(APXS) -q LDFLAGS_SHLIB`
APXS_SYSCONFDIR=`$(APXS) -q SYSCONFDIR`
APXS_LIBS_SHLIB=`$(APXS) -q LIBS_SHLIB`

#General settings
DEFINES       = -DVIC_STANDALONE=1 -DTINYXML_NAMESPACE=my_tinyxml -DTIXML_USE_STL 
CXXFLAGS      = -fPIC -Wall $(DEFINES)
INCPATH       = -iquote.


#   the default target
all: mod_victms.so

# compile the shared object file. use g++ instead of letting apxs call
# ld so we end up with the right c++ stuff. We do this in two steps,
# compile and link.

standalone:
	-cp ../base/src/vic/xml/tinyxml.h .
	-cp ../base/src/vic/xml/tinyxml.cpp .
	-cp ../base/src/vic/xml/tinyxmlerror.cpp .
	-cp ../base/src/vic/xml/tinyxmlparser.cpp .
	-cp ../base/src/vic/xml/tinystr.h .
	-cp ../base/src/vic/xml/tinystr.cpp .
	-cp ../base/src/vic/xml/document.hpp .
	-cp ../base/src/vic/xml/document.cpp .
	-cp ../ratman/src/vic/geo/base/tilemap_config.hpp .
	-cp ../ratman/src/vic/geo/base/tilemap_config.cpp .
	-cp ../ratman/src/vic/geo/base/victms_conventions.hpp .

# compile
mod_victms.o: mod_victms.cpp victms_conventions.hpp tinyxml.h tinystr.h tilemap_config.hpp document.hpp 
	g++ -c $(CXXFLAGS) $(INCPATH) -iquote$(APXS_INCLUDEDIR) -iquote$(APR_INCLUDEDIR) -iquote$(APU_INCLUDEDIR) -iquote- $(APXS_CFLAGS) $(APXS_CFLAGS_SHLIB) -o $@ $<

tinyxml.o: tinyxml.cpp tinyxml.h \
                tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxml.o tinyxml.cpp

document.o: document.cpp document.hpp \
                tinyxml.h \
                tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o document.o document.cpp

tinyxmlerror.o: tinyxmlerror.cpp tinyxml.h \
                tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxmlerror.o tinyxmlerror.cpp

tinyxmlparser.o: tinyxmlparser.cpp tinyxml.h \
                tinystr.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tinyxmlparser.o tinyxmlparser.cpp

tilemap_config.o: tilemap_config.cpp tilemap_config.hpp document.o
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tilemap_config.o tilemap_config.cpp

# link
mod_victms.so: mod_victms.o tilemap_config.o document.o tinyxml.o tinyxmlerror.o tinyxmlparser.o 
	g++ $(CXXFLAGS) -shared -o $@ $^ $(APXS_LIBS_SHLIB)

# install the shared object file into Apache 
install: all
	$(APXS) -i -n 'victms' mod_victms.so
	cp 99_mod_victms.conf ${APACHEMODS}

# display the apxs variables
check_apxs_vars:
	@echo APXS_CC $(APXS_CC);\
	echo APXS_TARGET $(APXS_TARGET);\
	echo APXS_CFLAGS $(APXS_CFLAGS);\
	echo APXS_SBINDIR $(APXS_SBINDIR);\
	echo APXS_CFLAGS_SHLIB $(APXS_CFLAGS_SHLIB);\
	echo APXS_INCLUDEDIR $(APXS_INCLUDEDIR);\
	echo APXS_LD_SHLIB $(APXS_LD_SHLIB);\
	echo APXS_LIBEXECDIR $(APXS_LIBEXECDIR);\
	echo APXS_LDFLAGS_SHLIB $(APXS_LDFLAGS_SHLIB);\
	echo APXS_SYSCONFDIR $(APXS_SYSCONFDIR);\
	echo APXS_LIBS_SHLIB $(APXS_LIBS_SHLIB);\
	echo APR_INCLUDEDIR $(APR_INCLUDEDIR);\
	echo APU_INCLUDEDIR $(APU_INCLUDEDIR);\

#   realcleanup
realclean: clean
	-rm -f tinyxml.h
	-rm -f tinyxml.cpp
	-rm -f tinyxmlerror.cpp
	-rm -f tinyxmlparser.cpp
	-rm -f tinystr.h
	-rm -f tinystr.cpp
	-rm -f document.hpp
	-rm -f document.cpp
	-rm -f tilemap_config.hpp
	-rm -f tilemap_config.cpp
	-rm -f victms_conventions.hpp

#   cleanup
clean:
	-rm -f *.so *.o *~

#   install and activate shared object by reloading Apache to
#   force a reload of the shared object file
reload: install restart

#   the general Apache start/restart/stop
#   procedures
start:
	$(APACHECTL) start
restart:
	$(APACHECTL) restart
stop:
	$(APACHECTL) stop

