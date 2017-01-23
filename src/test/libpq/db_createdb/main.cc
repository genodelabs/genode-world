/**
*  This example was taken of the article "PostgreSQL C tutorial" on the url next.
*  URL=http://zetcode.com/db/postgresqlc/
*/

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

void do_exit(PGconn *conn, PGresult *res) {
    
    fprintf(stderr, "%s\n", PQerrorMessage(conn));    

    PQclear(res);
    PQfinish(conn);    
    
    exit(1);
}

int main(int argc, char **argv) {
    const char *conninfo;

    if (argc > 0)
        conninfo = argv[0];
    else
        conninfo = "dbname=postgres";

    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) == CONNECTION_BAD) {
        
        fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
            
        PQfinish(conn);
        exit(1);
    }

    PGresult *res = PQexec(conn, "CREATE DATABASE baseDBTest;");
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        do_exit(conn, res);
    }
    
    PQclear(res);  
    PQfinish(conn);

    return 0;
}
