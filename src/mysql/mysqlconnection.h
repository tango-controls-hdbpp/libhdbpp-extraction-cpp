#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

#include <connection.h>
#include <mysql/mysql.h>

class MySqlResult;

/** \brief The specific to MySql implementation of Connection interface. <em>Used internally</em>.
 *
 */
class MySqlConnection : public Connection
{
public:
    MySqlConnection();

    virtual ~MySqlConnection();

    /** \brief Connects to the mysql server specified by host and port.
     *
     * @param host the server host name
     * @param db the database name
     * @param user the database user
     * @param passwd the database password
     * @param port the database port, optional. Default is 3306.
     *
     */
    virtual bool connect(const char* host, const char *db,
                 const char* user, const char *passwd,
                 unsigned short port = 3306 );

    virtual const char* getError();

    virtual bool isConnected() const;

    virtual void close();

    int getAffectedRows() const;

    Result *query(const char *);

private:

    int mAffectedRows;

    MYSQL mMysql, *mConnection;
};

#endif // MYSQLCONNECTION_H
