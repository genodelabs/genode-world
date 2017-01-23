/**
*  This example was taken of the article "PostgreSQL - C/C++ Interface" on the url next.
*  URL=http://www.tutorialspoint.com/postgresql/postgresql_c_cpp.htm
*/

#include <iostream>
#include <pqxx/pqxx> 

using namespace std;
using namespace pqxx;

int main(int argc, char* argv[])
{
   char * sql;
   
   try{
      connection C("dbname=testdb user=postgres password=ok \
      hostaddr=10.12.119.178 port=5432");

      if (C.is_open()) {
         cout << "Opened database successfully: " << C.dbname() << endl;
      } else {
         cout << "Can't open database" << endl;
         return 1;
      }

      /* Create SQL statement */
      sql = "CREATE TABLE COMPANY("  \
      "ID INT PRIMARY KEY     NOT NULL," \
      "NAME           TEXT    NOT NULL," \
      "AGE            INT     NOT NULL," \
      "ADDRESS        CHAR(50)," \
      "SALARY         REAL );";

      /* Create a transactional object. */
      work W(C);
      
      /* Execute SQL query */
      W.exec( sql );
      W.commit();
      cout << "Table created successfully" << endl;

      C.disconnect ();

   }catch (const std::exception &e){
      cerr << e.what() << std::endl;
      return 1;
   }

   return 0;

}
