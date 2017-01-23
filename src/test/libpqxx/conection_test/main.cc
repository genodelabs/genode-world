/**
*  This example was taken of the article "PostgreSQL - C/C++ Interface" on the url next.
*  URL=http://www.tutorialspoint.com/postgresql/postgresql_c_cpp.htm
*/

#include <iostream>
#include <pqxx/pqxx>

#include <timer_session/connection.h>

using namespace std;
using namespace pqxx;

int main()
{

 try{

      connection C("dbname=postgres user=postgres password=ok \
      hostaddr=10.12.119.178 port=5432");

      if (C.is_open()) {
         cout << "Opened database successfully: " << C.dbname() << endl;
      } else {
         cout << "Can't open database" << endl;
         return 1;
      }
      Timer::Connection timer;
      timer.usleep(100000);


      C.disconnect ();

   }catch (const std::exception &e){
      cerr << e.what() << std::endl;
      return 1;
   }

 return 0;
}
