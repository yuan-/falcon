/*
 * FALCON - The Falcon Programming Language.
 * FILE: pgsql_mod.cpp
 *
 * PgSQL Falcon service/driver
 * -------------------------------------------------------------------
 * Author: Jeremy Cowgar, Stanislas Marquis
 * Begin: Sun Dec 23 21:54:42 2007
 *
 * -------------------------------------------------------------------
 * (C) Copyright 2007: the FALCON developers (see list in AUTHORS file)
 *
 * See LICENSE file for licensing details.
 */

#include <string.h>
#include <stdio.h>

#include <falcon/engine.h>
#include "pgsql_mod.h"

/* Oid - see catalog/pg_type.h */
#define PG_TYPE_BOOL                    16
#define PG_TYPE_BYTEA                   17
#define PG_TYPE_CHAR                    18
#define PG_TYPE_NAME                    19
#define PG_TYPE_INT8                    20
#define PG_TYPE_INT2                    21
#define PG_TYPE_INT2VECTOR              22
#define PG_TYPE_INT4                    23
#define PG_TYPE_REGPROC                 24
#define PG_TYPE_TEXT                    25
#define PG_TYPE_OID                     26
#define PG_TYPE_TID                     27
#define PG_TYPE_XID                     28
#define PG_TYPE_CID                     29
#define PG_TYPE_OIDVECTOR               30
#define PG_TYPE_SET                     32
#define PG_TYPE_CHAR2                   409
#define PG_TYPE_CHAR4                   410
#define PG_TYPE_CHAR8                   411
#define PG_TYPE_POINT                   600
#define PG_TYPE_LSEG                    601
#define PG_TYPE_PATH                    602
#define PG_TYPE_BOX                     603
#define PG_TYPE_POLYGON                 604
#define PG_TYPE_FILENAME                605
#define PG_TYPE_FLOAT4                  700
#define PG_TYPE_FLOAT8                  701
#define PG_TYPE_ABSTIME                 702
#define PG_TYPE_RELTIME                 703
#define PG_TYPE_TINTERVAL               704
#define PG_TYPE_UNKNOWN                 705
#define PG_TYPE_MONEY                   790
#define PG_TYPE_OIDINT2                 810
#define PG_TYPE_OIDINT4                 910
#define PG_TYPE_OIDNAME                 911
#define PG_TYPE_BPCHAR                  1042
#define PG_TYPE_VARCHAR                 1043
#define PG_TYPE_DATE                    1082
#define PG_TYPE_TIME                    1083  /* w/o timezone */
#define PG_TYPE_TIMETZ                  1266  /* with timezone */
#define PG_TYPE_TIMESTAMP               1114  /* w/o timezone */
#define PG_TYPE_TIMESTAMPTZ             1184  /* with timezone */
#define PG_TYPE_NUMERIC                 1700


namespace Falcon
{


int32 dbi_pgsqlQuestionMarksToDollars( const String& input, String& output )
{
    output.reserve( input.size() + 32 );
    output.size( 0 );

    uint32 pos0 = 0;
    uint32 pos1 = input.find( "?" );
    int32 i = 0;

    while ( pos1 != String::npos )
    {
        output += input.subString( pos0, pos1 );
        output.A( "$" ).N( ++i );
        pos0 = pos1 + 1;
        pos1 = input.find( "?", pos0 );
    }

    output += input.subString( pos0 );
    return i;
}


/******************************************************************************
 * Recordset class
 *****************************************************************************/

DBIRecordsetPgSQL::DBIRecordsetPgSQL( DBIHandlePgSQL* dbh, PGresult* res )
    :
    DBIRecordset( dbh ),
    m_row( -1 ),
    m_res( res )
{
    m_rowCount = PQntuples( res );
    m_columnCount = PQnfields( res );
}


DBIRecordsetPgSQL::~DBIRecordsetPgSQL()
{
   if ( m_res != NULL )
      close();
}


bool DBIRecordsetPgSQL::fetchRow()
{
    return ++m_row < m_rowCount ? true : false;
}


bool DBIRecordsetPgSQL::discard( int64 ncount )
{
    for ( int64 i=0; i < ncount; ++i )
    {
        if ( !fetchRow() )
            return false;
    }
    return true;
}


int64 DBIRecordsetPgSQL::getRowIndex()
{
    return m_row;
}


int64 DBIRecordsetPgSQL::getRowCount()
{
    return m_rowCount;
}


int DBIRecordsetPgSQL::getColumnCount()
{
   return m_columnCount;
}


bool DBIRecordsetPgSQL::getColumnName( int nCol, String& name )
{
    if ( nCol < 0 || nCol >= m_columnCount )
        return false;

    name.bufferize( PQfname( m_res, nCol ) );
    return true;
}


bool DBIRecordsetPgSQL::getColumnValue( int nCol, Item& value )
{
    if ( nCol < 0 || nCol >= m_columnCount )
        return false;

    const char* v = PQgetvalue( m_res, m_row, nCol );
    if ( *v == '\0'
        && PQgetisnull( m_res, m_row, nCol ) )
    {
        value.setNil();
        return true;
    }
    else
    if ( m_dbh->options()->m_bFetchStrings )
    {
        String s( v );
        s.bufferize();
        value = s;
        return true;
    }

    switch ( PQftype( m_res, nCol ) )
    {
    case PG_TYPE_BOOL:
        value.setBoolean( *v == 't' ? true : false );
        break;

    case PG_TYPE_INT2:
    case PG_TYPE_INT4:
    case PG_TYPE_INT8:
        value.setInteger( atol( v ) );
        break;

    case PG_TYPE_FLOAT4:
    case PG_TYPE_FLOAT8:
    case PG_TYPE_NUMERIC:
        value.setNumeric( atof( v ) );
        break;

    case PG_TYPE_DATE:
        {
            VMachine* vm = VMachine::getCurrent();
            if ( vm == 0 )
                return false;

            String tv( v );
            int64 year, month, day;
            tv.subString( 0, 4 ).parseInt( year );
            tv.subString( 5, 7 ).parseInt( month );
            tv.subString( 8, 10 ).parseInt( day );
            TimeStamp* ts = new TimeStamp( year, month, day );

            CoreObject* ots = vm->findWKI( "TimeStamp" )->asClass()->createInstance();
            ots->setUserData( ts );
            value = ots;
            break;
        }

    case PG_TYPE_TIME:
    case PG_TYPE_TIMETZ: // todo: handle tz
        {
            VMachine* vm = VMachine::getCurrent();
            if ( vm == 0 )
                return false;

            String tv( v );
            int64 hour, minute, second;
            tv.subString( 0, 2 ).parseInt( hour );
            tv.subString( 3, 5 ).parseInt( minute );
            tv.subString( 6, 8 ).parseInt( second );
            TimeStamp* ts = new TimeStamp( 0, 0, 0, hour, minute, second );

            CoreObject* ots = vm->findWKI( "TimeStamp" )->asClass()->createInstance();
            ots->setUserData( ts );
            value = ots;
            break;
        }

    case PG_TYPE_TIMESTAMP:
    case PG_TYPE_TIMESTAMPTZ: // todo: handle tz
        {
            VMachine* vm = VMachine::getCurrent();
            if ( vm == 0 )
                return false;

            String tv( v );
            int64 year, month, day, hour, minute, second;
            tv.subString(  0,  4 ).parseInt( year );
            tv.subString(  5,  7 ).parseInt( month );
            tv.subString(  8, 10 ).parseInt( day );
            tv.subString( 11, 13 ).parseInt( hour );
            tv.subString( 14, 16 ).parseInt( minute );
            tv.subString( 17, 19 ).parseInt( second );
            TimeStamp* ts = new TimeStamp( year, month, day, hour, minute, second );

            CoreObject* ots = vm->findWKI( "TimeStamp" )->asClass()->createInstance();
            ots->setUserData( ts );
            value = ots;
            break;
        }

    case PG_TYPE_CHAR2:
    case PG_TYPE_CHAR4:
    case PG_TYPE_CHAR8:
    case PG_TYPE_TEXT:
    case PG_TYPE_VARCHAR:
    default:
        {
            String s( v );
            s.bufferize();
            value = s;
            break;
        }
    }
    return true;
}


void DBIRecordsetPgSQL::close()
{
    if ( m_res != NULL )
    {
        PQclear( m_res );
        m_res = NULL;
    }
}

/*
    DBIStatementPgSQL class
 */

DBIStatementPgSQL::DBIStatementPgSQL( DBIHandlePgSQL* dbh, const String& query, const String& name )
    :
    DBIStatement( dbh )
{
    String temp;
    m_nParams = dbi_pgsqlQuestionMarksToDollars( query, temp );
    AutoCString zQuery( temp );
    AutoCString zName( name );
    PGresult* res = PQprepare( dbh->getConn(), zName.c_str(), zQuery.c_str(), m_nParams, NULL );

    if ( res == NULL
        || PQresultStatus( res ) != PGRES_COMMAND_OK )
        DBIHandlePgSQL::throwError( __FILE__, __LINE__, res );

    PQclear( res );

    getExecString( m_nParams, name );
}


DBIStatementPgSQL::~DBIStatementPgSQL()
{
}


void DBIStatementPgSQL::getExecString( uint32 nParams, const String& name )
{
    fassert( name.length() );

    m_execString.reserve( 11 + name.length() + ( nParams * 2 ) );
    m_execString.size( 0 );
    m_execString = "EXECUTE " + name + "(";
    if ( nParams > 0 )
    {
        m_execString.append( "?" );
        for ( uint32 i=1; i < nParams; ++i )
            m_execString.append( ",?" );
    }
    m_execString.append( ");" );
}


int64 DBIStatementPgSQL::execute( const ItemArray& params )
{
    String output;
    if ( params.length() != m_nParams
        || !dbi_sqlExpand( m_execString, output, params ) )
    {
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_BIND_SIZE, __LINE__ ) );
    }
    AutoCString zQuery( output );

    PGresult* res = PQexec( ((DBIHandlePgSQL*)m_dbh)->getConn(), zQuery.c_str() );

    if ( res == 0 )
        DBIHandlePgSQL::throwError( __FILE__, __LINE__, res );

    ExecStatusType st = PQresultStatus( res );
    if ( st != PGRES_COMMAND_OK
        && st != PGRES_TUPLES_OK )
        DBIHandlePgSQL::throwError( __FILE__, __LINE__, res );

    PQclear( res );
    return 0;
}


void DBIStatementPgSQL::reset()
{
}


void DBIStatementPgSQL::close()
{
}


/******************************************************************************
 * DB Handler class
 *****************************************************************************/


DBIHandlePgSQL::DBIHandlePgSQL( PGconn *conn )
    :
    m_conn( conn ),
    m_bInTrans( false )
{}


DBIHandlePgSQL::~DBIHandlePgSQL()
{
    close();
}


void DBIHandlePgSQL::close()
{
    if ( m_conn != 0 )
    {
        if ( m_bInTrans )
        {
            // force rollback, skipping errors...
            PGresult* res = PQexec( m_conn, "ROLLBACK" );
            if ( res != 0 )
                PQclear( res );
        }
        PQfinish( m_conn );
        m_conn = 0;
    }
    m_bInTrans = false;
}


void DBIHandlePgSQL::options( const String& params )
{
    if ( !m_settings.parse( params ) )
    {
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_OPTPARAMS, __LINE__ )
            .extra( params ) );
    }
}


const DBISettingParams* DBIHandlePgSQL::options() const
{
    return &m_settings;
}


void DBIHandlePgSQL::throwError( const char* file, int line, PGresult* res )
{
    fassert( res );

    int code = (int) PQresultStatus( res );
    const char* err = PQresultErrorMessage( res );

    if ( err != NULL && err[0] != '\0' )
    {
        String desc = err;
        desc.remove( desc.length() - 1, 1 ); // Get rid of newline
        desc.bufferize();

        PQclear( res );

        throw new DBIError( ErrorParam( code, line )
            .extra( desc )
            .module( file ) );
    }
    else
    {
        PQclear( res );

        throw new DBIError( ErrorParam( code, line )
            .module( file ) );
    }
}


void DBIHandlePgSQL::begin()
{
    if ( m_conn == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CLOSED_DB, __LINE__ ) );

    if ( m_bInTrans )
        return;

    PGresult* res = PQexec( m_conn, "BEGIN" );
    if ( res == 0
        || PQresultStatus( res ) != PGRES_COMMAND_OK )
    {
        throwError( __FILE__, __LINE__, res );
    }

    m_bInTrans = true;
    PQclear( res );
}


void DBIHandlePgSQL::commit()
{
    if ( m_conn == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CLOSED_DB, __LINE__ ) );

    if ( !m_bInTrans )
        return;

    PGresult* res = PQexec( m_conn, "COMMIT" );
    if ( res == 0
        || PQresultStatus( res ) != PGRES_COMMAND_OK )
    {
        throwError( __FILE__, __LINE__, res );
    }

    m_bInTrans = false;
    PQclear( res );
}


void DBIHandlePgSQL::rollback()
{
    if ( m_conn == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CLOSED_DB, __LINE__ ) );

    if ( !m_bInTrans )
        return;

    PGresult* res = PQexec( m_conn, "ROLLBACK" );
    if ( res == 0
        || PQresultStatus( res ) != PGRES_COMMAND_OK )
    {
        throwError( __FILE__, __LINE__, res );
    }

    m_bInTrans = false;
    PQclear( res );
}


PGresult* DBIHandlePgSQL::internal_exec( const String& sql, int64& affectedRows )
{
    fassert( m_conn );

    AutoCString cStr( sql );
    PGresult* res = PQexec( m_conn, cStr.c_str() );

    if ( res == NULL )
        throwError( __FILE__, __LINE__, res );

    ExecStatusType st = PQresultStatus( res );
    if ( st != PGRES_TUPLES_OK
        && st != PGRES_COMMAND_OK )
        throwError( __FILE__, __LINE__, res );

    const char* num = PQcmdTuples( res );
    if ( num && num[0] != '\0' )
        affectedRows = atoi( num );
    else
        affectedRows = -1;

    if ( st == PGRES_TUPLES_OK )
        return res;
    else
    {
        PQclear( res );
        return NULL;
    }
}


DBIRecordset* DBIHandlePgSQL::query( const String &sql, int64 &affectedRows, const ItemArray& params )
{
    if ( m_conn == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CLOSED_DB, __LINE__ ) );

    String output;
    if ( params.length() != 0 )
    {
        if ( !dbi_sqlExpand( sql, output, params ) )
        {
            throw new DBIError( ErrorParam( FALCON_DBI_ERROR_QUERY, __LINE__ ) );
        }
    }
    else
        output = sql;

    PGresult* res = internal_exec( output, affectedRows );
    if ( res == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_QUERY, __LINE__ ) );

    return new DBIRecordsetPgSQL( this, res );
}


void DBIHandlePgSQL::perform( const String &sql, int64 &affectedRows, const ItemArray& params )
{
    if ( m_conn == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CLOSED_DB, __LINE__ ) );

    String output;
    if ( params.length() != 0 )
    {
        if ( !dbi_sqlExpand( sql, output, params ) )
        {
            throw new DBIError( ErrorParam( FALCON_DBI_ERROR_QUERY, __LINE__ ) );
        }
    }
    else
        output = sql;

    PGresult* res = internal_exec( output, affectedRows );
    if ( res )
        PQclear( res );
}


DBIRecordset* DBIHandlePgSQL::call( const String &sql, int64 &affectedRows, const ItemArray& params )
{
    if ( m_conn == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CLOSED_DB, __LINE__ ) );

    String output;
    if ( params.length() != 0 )
    {
        if ( !dbi_sqlExpand( sql, output, params ) )
        {
            throw new DBIError( ErrorParam( FALCON_DBI_ERROR_QUERY, __LINE__ ) );
        }
        output.prepend( "EXECUTE " );
    }
    else
        output = "EXECUTE " + sql;

    PGresult* res = internal_exec( output, affectedRows );
    return res != 0 ? new DBIRecordsetPgSQL( this, res ) : NULL;
}


DBIStatement* DBIHandlePgSQL::prepare( const String &query )
{
    if ( m_conn == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CLOSED_DB, __LINE__ ) );

    return new DBIStatementPgSQL( this, query );
}


DBIStatement* DBIHandlePgSQL::prepareNamed( const String &name, const String& query )
{
    if ( m_conn == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CLOSED_DB, __LINE__ ) );

    return new DBIStatementPgSQL( this, query, name );
}


int64 DBIHandlePgSQL::getLastInsertedId( const String& name )
{
    if ( m_conn == 0 )
        throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CLOSED_DB, __LINE__ ) );

    /* so... PQoidValue does it but takes a PGresult.
    We can retrieve a PGresult in case of prepared statements only...
    */

    AutoCString nm( name );
    PGresult* res = PQdescribePrepared( m_conn, nm.c_str() );
    int oid = -1;

    if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
        goto finish;
    else
        oid = (int) PQoidValue( res );

  finish:
    PQclear( res );
    return oid;
}


void DBIHandlePgSQL::selectLimited( const String& query, int64 nBegin, int64 nCount, String& result )
{
    String sBegin, sCount;

    if( nCount > 0 )
        sCount.A( " LIMIT " ).N( nCount );

    if ( nBegin > 0 )
        sBegin.A( " OFFSET " ).N( nBegin );

    result = "SELECT " + query + sCount + sBegin;
}


/******************************************************************************
 * Main service class
 *****************************************************************************/

void DBIServicePgSQL::init()
{
}


DBIHandle *DBIServicePgSQL::connect( const String &parameters )
{
   AutoCString connParams( parameters );
   PGconn *conn = PQconnectdb( connParams.c_str () );
   if ( conn == NULL )
   {
      throw new DBIError( ErrorParam( FALCON_DBI_ERROR_NOMEM, __LINE__ ) );
   }

   if ( PQstatus( conn ) != CONNECTION_OK )
   {
      String errorMessage = PQerrorMessage( conn );
      errorMessage.remove( errorMessage.length() - 1, 1 ); // Get rid of newline
      errorMessage.bufferize();

      PQfinish( conn );

      throw new DBIError( ErrorParam( FALCON_DBI_ERROR_CONNECT, __LINE__ )
              .extra( errorMessage ) );
   }

   return new DBIHandlePgSQL( conn );
}


CoreObject *DBIServicePgSQL::makeInstance( VMachine *vm, DBIHandle *dbh )
{
   Item *cl = vm->findWKI( "PgSQL" );
   if ( cl == 0 || ! cl->isClass() || cl->asClass()->symbol()->name() != "PgSQL" )
   {
      throw new DBIError( ErrorParam( FALCON_DBI_ERROR_INVALID_DRIVER, __LINE__ ) );
   }

   CoreObject *obj = cl->asClass()->createInstance();
   obj->setUserData( dbh );

   return obj;
}

} /* namespace Falcon */
