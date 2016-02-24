// TestStatement.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <Windows.h>
#include "mysql/mysql.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <thread>

#pragma comment( lib, "libmysql.lib" )

#define TESTTIME (3600)
#define NUMTHREADS (10)
#define DATASIZE (16*1024*1024)
char host[]="127.0.0.1";
int port=3306;
char username[]="root";
char password[]="123456";
char database[]="test";
std::thread* pthreads[NUMTHREADS];
unsigned long client_version=0;
unsigned long server_version=0;
unsigned long num_queries=0;
int threaddone=0;


char *databucket=NULL;

int db_query(MYSQL *dbc,char *sql,int showresults);
char* alocmem(size_t num);
int write_string(char *buf,const size_t minlen,const size_t maxlen);
/* prepared statement function declarations: 1 */
int ps_func_0000(MYSQL *db);

void worker_thread()
{
	MYSQL *dbc=NULL;
	my_bool auto_reconnect=1;
	int cancelstate=0;
	
	dbc = mysql_init(NULL);
	if(NULL == dbc)
	{
		printf("mysql_init failed\n");
		goto threadexit;
	}
	else
	{
		if(0!=mysql_options(dbc,MYSQL_OPT_RECONNECT,(char*)&auto_reconnect))
		{
			printf("mysql_options() failed to set MYSQL_OPT_RECONNECT");
		}
		if (!mysql_real_connect(dbc,host,username,password,database,port, NULL, CLIENT_FOUND_ROWS|CLIENT_MULTI_STATEMENTS|CLIENT_MULTI_RESULTS))
		{
			printf("mysql_real_connect failed: %s (%d)", mysql_error(dbc),mysql_errno(dbc));
			mysql_close(dbc);
			dbc=NULL;
		}
	}

	char shortquery[1024];
	memset(shortquery,0,1024);
	char *longquery;
	longquery=NULL;
	char *c;
	c=NULL;
	while(0==threaddone && NULL!=dbc)
	{
		if(rand()%7==0)
		{
			c=shortquery;
			c+=sprintf_s(c, 1024, "%s","flush tables");
			db_query(dbc,shortquery,1);
		}

// 		if(rand()%7==0)
// 		{
// 			longquery = alocmem(2236);
// 			c=longquery;
// 			c+=sprintf_s(c, 1024, "%s","insert ignore into `qa00` set `c1`=if(abs(");
// 			c+=sprintf_s(c, 1024, "%ld",-32768 + rand()%65535lu);
// 			c+=sprintf_s(c, 1024, "%s",")%100<20,null,'");
// 			c+=write_string(c,0,255);
// 			c+=sprintf_s(c, 1024, "%s","'),`c2`=if(abs(");
// 			c+=sprintf_s(c, 1024, "%ld",-32768 + rand()%65535lu);
// 			c+=sprintf_s(c, 1024, "%s",")%100<20,null,'");
// 			c+=write_string(c,0,255);
// 			c+=sprintf_s(c, 1024, "%s","'),`c3`=if(abs(");
// 			c+=sprintf_s(c, 1024, "%ld",-32768 + rand()%65535lu);
// 			c+=sprintf_s(c, 1024, "%s",")%100<20,null,'");
// 			c+=write_string(c,0,255);
// 			c+=sprintf_s(c, 1024, "%s","'),`c4`=if(abs(");
// 			c+=sprintf_s(c, 1024, "%ld",-32768 + rand()%65535lu);
// 			c+=sprintf_s(c, 1024, "%s",")%100<20,null,'");
// 			c+=write_string(c,0,255);
// 			c+=sprintf_s(c, 1024, "%s","')");
// 			db_query(dbc,longquery,1);
// 			free(longquery);
// 		}

		if(rand()%7==0)
		{
			//select * from `qa00` limit 5
			ps_func_0000(dbc);
		}
	}
threadexit:
	mysql_close(dbc);
	mysql_thread_end();
}

int main(int argc, const char *argv[])
{
	MYSQL *dbc=NULL;
	int i=0,err=0;

	time_t timestart=0,timenow=0;

	unsigned int counter=0;
	counter=0;
	char shortquery[1024]={0};
	char *longquery=NULL;
	longquery=NULL;
	char *c=NULL;
	mysql_library_init( 0, nullptr, nullptr );
	if (!(dbc = mysql_init(NULL)))
	{
		printf("mysql_init\n");
		dbc=NULL;
		goto threadexit;
	}
	else
	{
		if (!mysql_real_connect(dbc,host,username,password,database,port, NULL, CLIENT_FOUND_ROWS|CLIENT_MULTI_STATEMENTS|CLIENT_MULTI_RESULTS))
		{
			printf("mysql_real_connect failed: %s (%d)", mysql_error(dbc),mysql_errno(dbc));
			mysql_close(dbc);
			dbc=NULL;
			goto threadexit;
		}
	}

	printf("running initializations..\n");
	client_version=mysql_get_client_version();
	server_version=mysql_get_server_version(dbc);
	printf("client version=%lu\n",client_version);
	printf("server version=%lu\n",server_version);
	if((client_version/10000) < (server_version/10000))
	{
		printf("incompatible client and server version!  please upgrade client library!\n");
		goto threadexit;
	}

	if (!mysql_thread_safe())
	{
		printf("non-threadsafe client detected!  please rebuild and link with libmysql_r!\n");
	}

	printf("pre-generating %d bytes of random data\n",DATASIZE);
	databucket=(char*)calloc(DATASIZE,sizeof(char*));
	if(NULL == databucket)
	{
		printf("error: cannot calloc data buffer.\n");
		exit(1);
	}
	for(i=0;i<DATASIZE-1;i++)
	{
		databucket[i]=0x61+(rand()%26);
	}
// 	c=shortquery;
// 	c+=sprintf_s(c, 1024, "%s","/*!50001 set global innodb_flush_log_at_trx_commit=0 */");
// 	db_query(dbc,shortquery,1);
// 
// 	c=shortquery;
// 	c+=sprintf_s(c, 1024, "%s","/*!50001 set global innodb_flush_log_at_trx_commit=0 */");
// 	db_query(dbc,shortquery,1);

	c=shortquery;
	c+=sprintf_s(c, 1024, "%s","drop table if exists `qa00`");
	db_query(dbc,shortquery,1);

	c=shortquery;
	c+=sprintf_s(c, 1024, "%s","CREATE TABLE `qa00` (   `c1` char(1) DEFAULT NULL,   `c2` char(10) DEFAULT NULL,   `c3` char(255) DEFAULT NULL,   `c4` varchar(255) DEFAULT NULL,   KEY `c1` (`c1`),   KEY `c2` (`c2`),   KEY `c3` (`c3`),   KEY `c4` (`c4`),   KEY `c4_2` (`c4`) ) ENGINE=InnoDB DEFAULT CHARSET=latin1");
	db_query(dbc,shortquery,1);

	mysql_close(dbc);

	printf("about to spawn %d threads\n",NUMTHREADS);
	for (i=0;i<NUMTHREADS;i++)
	{
		pthreads[i] = new std::thread(worker_thread);
	}
	printf("\n");
	printf("completed spawning new database worker threads\n");

	printf("testcase is now running, so watch for error output\n");

	timestart=time(NULL);
	timenow=time(NULL);
	for(i=0;(timenow-timestart) < TESTTIME;timenow=time(NULL))
	{
		Sleep(1000);
		printf("queries: %09lu\n",num_queries);
	}
	threaddone=1;

	printf("waiting for worker threads to finish...\n");

	Sleep( ~0 );

	if(NULL!=databucket)free(databucket);
	exit(0);
threadexit:
	exit(-1);
}


int db_query(MYSQL *dbc,char *sql,int showresults)
{
	int res=0;
	MYSQL_RES *r=NULL;
	MYSQL_ROW w;
	MYSQL_FIELD *field=NULL;
	int moreresult=0;
	unsigned int i=0;
	if(NULL == dbc) return 0;
	res = mysql_query(dbc,sql);
	if(res != 0 && showresults > 0)
	{
		printf("query failed '%s' : %d (%s)\n",sql,mysql_errno(dbc),mysql_error(dbc));
		return 0;
	}

	num_queries++;
	do
	{
		r = mysql_use_result(dbc);
		if(r)
		{
			unsigned int numfields = mysql_num_fields(r);
			//unsigned int numrows=mysql_num_rows(r);
			while(0!=(field = mysql_fetch_field(r)))
			{
				//print metadata information about each field
				if(showresults > 1)
				{
					printf("%s	",field->name);
				}
			}
			if(showresults > 1)
			{
				printf("\n------------------------------------\n");
			}

			while (0!=(w = mysql_fetch_row(r)))
			{
				for(i = 0; i < numfields; i++)
				{
					//print each field here
					if(showresults > 1)
					{
						printf("%s\t",w[i]);
					}
				}

				if(showresults > 1)
				{
					printf("\n");
				}
			}
			if(showresults > 1)
			{
				printf("\n");
			}
			mysql_free_result(r);
		}
		else //no rows returned. was it a select?
		{
			if(mysql_field_count(dbc) > 0 && showresults > 0)
			{
				printf("No results for '%s'.  (%d) - %s\n",sql,mysql_errno(dbc),mysql_error(dbc));
				return 0;
			}
			else //it could have been some insert/update/delete
			{
				//this is successful query
			}
		}
		moreresult=mysql_next_result(dbc);
		if(moreresult > 0 && showresults > 0)
		{
			printf("mysql_next_result returned %d, mysql error %s, (%d)\n",moreresult,mysql_error(dbc),mysql_errno(dbc));
			return 0;
		}
	} while (0==moreresult);
	return 1;
}

char* alocmem(size_t num)
{
	char *r=(char*)calloc(num,1);
	if(NULL == r)
	{
		printf("cannot calloc %I64u bytes of memory\n",num);
		exit(1);
	}
	return r;
}

int write_string(char *buf,const size_t minlen,const size_t maxlen)
{
	if(!buf)return 0;
	if(maxlen==0 ){buf[0]=0;return 0;}
	size_t lengt=minlen + (maxlen<=minlen?0:rand() % (maxlen-minlen));
	int start_lengt = rand()%(DATASIZE - lengt - 1);
	if(lengt<0) lengt=0;
	if(start_lengt<0) start_lengt=0;
	memcpy(buf,databucket+start_lengt,lengt);
	buf[lengt]=0;
	return lengt;
}

int ps_func_0000(MYSQL *db)
{
	char *query="select * from `qa00` limit 5";
	MYSQL_STMT *stmt=NULL;
	unsigned int val=0;
	int i=0;
	int error=0;
	MYSQL_BIND bind_result[4];
	unsigned long length_result[4];
	my_bool is_null_result[4];
	my_bool error_result[4];
	memset(bind_result, 0, sizeof(bind_result));
	stmt= mysql_stmt_init(db);
	if (NULL == stmt)
	{
		printf("mysql_stmt_init failed: %s (%d)\n",mysql_error(db),mysql_errno(db));
		error=1;
		goto end;
	}

	if(mysql_stmt_prepare(stmt,query,(unsigned long)strlen(query)))
	{
		printf("mysql_stmt_prepare failed: %s (%d)\n",mysql_stmt_error(stmt),mysql_stmt_errno(stmt));
		error=1;
		goto end;
	}

//	val=1;
// 	if(mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, (void *)&val))
// 	{
// 		printf("mysql_stmt_attr_set: %s (%d)\n",mysql_stmt_error(stmt),mysql_stmt_errno(stmt));
// 		error=1;
// 		goto end;
// 	}

	val=CURSOR_TYPE_READ_ONLY;
	if(mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void *)&val))
	{
		printf("mysql_stmt_attr_set: %s (%d)\n",mysql_stmt_error(stmt),mysql_stmt_errno(stmt));
		error=1;
		goto end;
	}

// 	val=5000;
// 	if(mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS, (void *)&val))
// 	{
// 		printf("mysql_stmt_attr_set: %s (%d)\n",mysql_stmt_error(stmt),mysql_stmt_errno(stmt));
// 		error=1;
// 		goto end;
// 	}

	bind_result[0].buffer_type=MYSQL_TYPE_STRING;
	bind_result[0].buffer=alocmem(1);
	bind_result[0].buffer_length=1;
	bind_result[0].length=&length_result[0];
	bind_result[0].is_null=&is_null_result[0];
	bind_result[0].error=&error_result[0];

	bind_result[1].buffer_type=MYSQL_TYPE_STRING;
	bind_result[1].buffer=alocmem(10);
	bind_result[1].buffer_length=10;
	bind_result[1].length=&length_result[1];
	bind_result[1].is_null=&is_null_result[1];
	bind_result[1].error=&error_result[1];

	bind_result[2].buffer_type=MYSQL_TYPE_STRING;
	bind_result[2].buffer=alocmem(255);
	bind_result[2].buffer_length=255;
	bind_result[2].length=&length_result[2];
	bind_result[2].is_null=&is_null_result[2];
	bind_result[2].error=&error_result[2];

	bind_result[3].buffer_type=MYSQL_TYPE_VAR_STRING;
	bind_result[3].buffer=alocmem(255);
	bind_result[3].buffer_length=255;
	bind_result[3].length=&length_result[3];
	bind_result[3].is_null=&is_null_result[3];
	bind_result[3].error=&error_result[3];

	for(i=0;i<10;i++)
	{
		if(mysql_stmt_bind_result(stmt,bind_result))
		{
			printf("mysql_stmt_bind_result failed: %s (%d)\n",mysql_stmt_error(stmt),mysql_stmt_errno(stmt));
			error=1;
			goto end;
		}
		if(mysql_stmt_execute(stmt))
		{
			int a = mysql_stmt_execute(stmt);
			printf("mysql_stmt_execute failed: %s (%d)\n",mysql_stmt_error(stmt),mysql_stmt_errno(stmt));
			error=1;
			goto end;
		}
		num_queries++;

		if(mysql_stmt_store_result(stmt))
		{
			printf("mysql_stmt_store_result failed: %s (%d)\n",mysql_stmt_error(stmt),mysql_stmt_errno(stmt));
			error=1;
			goto end;
		}

		while (!mysql_stmt_fetch(stmt))
		{
		}
	}

	if(mysql_stmt_close(stmt))
	{
		printf("mysql_stmt_close failed: %s (%d)\n",mysql_stmt_error(stmt),mysql_stmt_errno(stmt));
		error=1;
		goto end;
	}

end:
	if(NULL!=bind_result[0].buffer) free(bind_result[0].buffer);
	if(NULL!=bind_result[1].buffer) free(bind_result[1].buffer);
	if(NULL!=bind_result[2].buffer) free(bind_result[2].buffer);
	if(NULL!=bind_result[3].buffer) free(bind_result[3].buffer);

	if(1==error) goto err;
	return 1;

err:
	return 0;
}

