#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>

#include "sql.h"

 void err_exit(char* s)
{
    perror(s);
    perror("\n");
    exit(1);
}

// moet worden aangeroepen ergens, 1x, dus best in de init.
int db_connect() {
    // Initialize Connection
    MYSQL *conn;

    if (!(conn = mysql_init(0)))
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
        conn,                   // Connection
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
      fprintf(stderr, "Error connecting to Server: %s\n", mysql_error(conn));
      mysql_close(conn);
      exit(1);
   }

    return 0;
}

// aanroepen als geen mqtt ontvangt, kan bij de alive check en disconnect bij geen puls van betreffende rp
int db_disconnect() {
    // Close the Connection
    mysql_close(conn);
    mysql_library_end();
    return 0;
}

int send_query(char *buffer){
    char * query;

    stmt = mysql_stmt_init(conn);

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

}

// insert, pas functie aan naar wens voor nodige hoeveel data kan dus naar wens als ik er niet aan toekom
int insert(char *tabel, char *d1, char *d2, char *d3) {
    // kan ook naar keuze hier tabellen en kollomen hardcoden
        // ruimte


    // buffer string voor de query, verwerkt ook invoer variablen
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "INSERT INTO %s VALUES ('%s','%s');", tabel,  d1, d2);

    //verstuurt het 
    send_query(buffer);
    
    return 0;
}

//voor specifiek in meting inserten in mijn (Timo's) geval en Gijs ook (als het goed is)
int insert_meting(double meetwaarde) {
    // kan ook naar keuze hier tabellen en kollomen hardcoden
    char *tabel = "meting";
    int sensorId = 3; // MOET ZELFDE nmr zijn als sensorID IN TABEL sensor
    

    // buffer string voor de query, verwerkt ook invoer variablen
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "INSERT INTO %s (Meetwaarde, Meetdatum, sensorSensorId ) VALUES ('%.2f', Now(), '%d');", tabel,  meetwaarde, sensorID);

    //verstuurt het 
    send_query(buffer);
    
    return 0;
}


// nieuwe transactie (reamon)
int new_tran() {
    char *query = "INSERT INTO Transactie (DeviceDeviceId,GebruikerGebruikerId,TransactieStart) VALUES ("","",Now())"
    send_query(query);
    return 0;
}

// select functie, kan tabel naam en kolom naam meegeven
int select(char *tabel, char *kolom) {
    MYSQL_RES *query_result;
    MYSQL_ROW row;

    int res;
    char *query;
    int number_of_cols, i;

    char buffer[128];

    snprintf(buffer, sizeof(buffer), "SELECT %s FROM %s;", kolom, tabel);
    printf("%s\n\n", buffer);
    // a select query where you want to work with the output    
    query = buffer;
      
    res = mysql_real_query(conn,query, strlen(query));
    printf("Query out: %d\n", res);
    if(res != 0)
    {
        printf("Query failed: %s\n", mysql_error(conn));
    }
    else
    {
        puts("Query OK");
    }

    // geeft aan dat je terugkoppeling wil
    query_result = mysql_use_result(conn);
    
    if(query_result==NULL) {
        printf("No records or error: %s\n", mysql_error(conn));
    } else {
        puts("succesful selected query rows");
        number_of_cols = mysql_num_fields(query_result);
        printf("Number of query col: %d", number_of_cols);
        putchar('\n');
    }
    // collecting the rows one by one
    
    // is dus wat je terugkrijgt 
    // print elke row, elke row komt in array, formaat is aantal teruggegeven rows in number_of_cols

    row = mysql_fetch_row(query_result);
    while(row !=NULL)
    {
    for (i = 0; i < number_of_cols; i++) {
        printf("%s ", row[i]);
        }
    putchar('\n');
    
    row = mysql_fetch_row(query_result);
    }
      
    mysql_free_result(query_result);

    return 0;
}