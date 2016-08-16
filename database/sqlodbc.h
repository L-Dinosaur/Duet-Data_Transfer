#ifndef _SQLODBC_H
#define	_SQLODBC_H
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>

#define	CONSTRSIZE	1024

void extract_error(char *fn, SQLHANDLE handle, SQLSMALLINT type);

#endif /* _SQLODBC_H */
