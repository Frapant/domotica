#ifndef _SQL_H_
#define _SQL_H_

int db_connect();
int db_disconnect();

//verschillende inserts
int insert(char* tabel, char* d1, char* d2, char* d3);
int insert_meting(double meetwaarde);

// select functies
int select(char *tabel, char *kolom);

#endif