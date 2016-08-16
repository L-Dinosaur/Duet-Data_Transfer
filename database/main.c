#include "sqlodbc.h"

int main()
{
	// Declaration
	SQLHENV env;
	SQLHDBC dbc;
	SQLHSTMT stmt;
	SQLRETURN ret;
	SQLCHAR outstr[CONSTRSIZE];
	SQLSMALLINT outstrlen;

	// Allocate Environment Handle
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	
	// Set Env Attribute
	SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);

	// Allocate Connection Handle
	SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
	
	// Connect
	ret = SQLDriverConnect(dbc, NULL, "DSN=PostgreSQL;", SQL_NTS, outstr, sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);
	if(SQL_SUCCEEDED(ret)) {
		printf("Connected\n");
		printf("Returned connection string was: \n\t%s\n", outstr);
		if(ret == SQL_SUCCESS_WITH_INFO) {
			printf("Driver reported the following diagnostics\n");
			extract_error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
		}
		SQLDisconnect(dbc);
	} else {
		fprintf(stderr, "Failed to connect\n");
		extract_error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
	}
	
	SQLFreeHandle(SQL_HANDLE_DBC, dbc);
	SQLFreeHandle(SQL_HANDLE_ENV, env);
	return 0;
}

