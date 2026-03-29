#ifndef _SQL_T_
#define _SQL_T_

int db_connect();
int db_disconnect();

int insert_meting(double meetwaarde, char *type);

#endif