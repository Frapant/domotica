#include <stdio.h>
#include <stdlib.h>
#include <mariadb/mysql.h>
#include <string.h>

#include "sql_t.h"

MYSQL *conn_t;

 void err_exit(char* s)
{
    perror(s);
    perror("\n");
    exit(1);
}

// moet worden aangeroepen ergens, 1x, dus best in de init.
int db_connect() {

    // Initialize Connection
   if (!(conn_t = mysql_init(0)))
   {
      fprintf(stderr, "unable to initialize connection struct\n");
      exit(1);
   }
   else
   {
    fprintf(stderr, "GOT it YOU ARE IN\n");
   }

   // Connect to the database
   if (!mysql_real_connect(
        conn_t,                   // Connection
        "localhost",            // Host
        "frap",                 // User account
        "1234",                 // User password
        "Watertap",             // Default database
        3306,                   // Port number
        NULL,                   // Path to socket file
        0                       // Additional options
      ))
   {
      // Report the failed-connection error & close the handle
      fprintf(stderr, "Error connecting to Server: %s\n", mysql_error(conn_t));
      mysql_close(conn_t);
      exit(1);
   }

    return 0;
}

// aanroepen als geen mqtt ontvangt, kan bij de alive check en disconnect bij geen puls van betreffende rp
int db_disconnect() {
    // Close the Connection
    mysql_close(conn_t);
    mysql_library_end();
    return 0;
}

int send_query(char *buffer){
    char * query;

    MYSQL_STMT *stmt = NULL;
    stmt = mysql_stmt_init(conn_t);

    // hier definier je de query = communicatie met database
    if (stmt) {
        query = buffer;

        // voorbereiding voor het uitvoeren van de query
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            printf("Statement prepare failed: %s\n", mysql_stmt_error(stmt));
        } else {
            puts("Statement prepare OK!");
        }
    }

    // uitvoeren van de query
    if (mysql_stmt_execute(stmt)) {
            printf("Statement execute failed: %s\n", mysql_stmt_error(stmt));
        } else {
            puts("Statement execute OK!");
        }
    return 0;
}

// insert into meeting
int insert_meting(double meetwaarde, char *type) {
    // kan ook naar keuze hier tabellen en kollomen hardcoden
    char *tabel = "meting";

    // buffer string voor de query, verwerkt ook invoer variablen
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "INSERT INTO %s (Meetwaarde, Meetdatum, sensorSensorId) VALUES ('%.2f', Now(), (SELECT SensorId FROM sensor where Metingtype = '%s'));", tabel,  meetwaarde, type);

    //verstuurt het
    send_query(buffer);

    return 0;
}