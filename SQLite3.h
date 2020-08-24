#ifndef SQLITE3_H_INCLUDED
#define SQLITE3_H_INCLUDED


#define SQLITE_OK   0x00
#define SQLITE_ROW  0x64

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

typedef int (_cdecl *Fsqlite3_open) (const char *filename, sqlite3 **ppDb);
typedef int (_cdecl *Fsqlite3_prepare_v2) (sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
typedef int (_cdecl *Fsqlite3_close) (sqlite3 *);
typedef int (_cdecl *Fsqlite3_step) (sqlite3_stmt *);
typedef const unsigned char * (_cdecl *Fsqlite3_column_text)(sqlite3_stmt *, int iCol);

#endif //SQLITE3_H_INCLUDED
