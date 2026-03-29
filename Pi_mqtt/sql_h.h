#ifndef _SQL_H_
#define _SQL_H_

int db_connect();
int db_disconnect();

int insert_meting(double meetwaarde, char *type);

#endif