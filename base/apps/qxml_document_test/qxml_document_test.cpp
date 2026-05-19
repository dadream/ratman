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
#include <vic/qxml/database.hpp>
#include <string>
#include <iostream>
#include <fstream>


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

int main() {

  vic::qxml::database db("test", "test.xml");

  int people_count = db.count("FIGLIO");
  QString path = QString("FIGLIO[@sesso='F']/NOME");
  std::cerr << "Number of guys == " << people_count << std::endl;

  QString      girl_name =  db.get_qstring(path);
  std::cerr << "Girl name is " << ( girl_name.toLatin1().data() ) << std::endl; 

  path = QString("FIGLIO[@sesso='F']/DESCRIZIONE[2]/NUMERO[2]");
  std::cerr << " Numero vale " << ( db.get_int( path ) ) << std::endl;

  return 0;
}
