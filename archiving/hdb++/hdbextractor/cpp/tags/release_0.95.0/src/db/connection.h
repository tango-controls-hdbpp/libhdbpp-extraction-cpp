#ifndef CONNECTION_H
#define CONNECTION_H

class Result;
class XVariant;

/** \brief The interface that represents a server database connection. <em>Used internally</em>.
 *
 *  You can implement this interface to define a database connection specific to a particular
 *  database server, i.e. a mysql server.
 *
 *  @see MySqlConnection
 *  @see DbSchema
 */
class Connection
{
public:

    /** \brief Object constructor
     *
     */
    Connection() {}

    virtual ~Connection() {}

    /** \brief Try to establish a connection to the database server with the specified parameters
     *
     * @return true if the connection succeeded, false otherwise.
     *
     * @param host the host name of the database server
     * @param db the name of the database
     * @param user the user allowed to query the database
     * @param passwd the password of the user specified before
     * @param port the database server port number
     *
     * @see MySqlConnection
     */
    virtual bool connect(const char* host, const char *db,
                 const char* user, const char *passwd,
                 unsigned short port ) = 0;

    /** \brief returns the last error that occurred during the connection setup
     *
     */
    virtual const char* getError() = 0;

    /** \brief Tells if the coneection to the database is established or not
     *
     * @return true database connection established
     * @return false database connection not established
     */
    virtual bool isConnected() const = 0;

    /** \brief Closes the database connection, if open
     */
    virtual void close() = 0;

    /** \brief Executes a query and returns a Result object
     *
     * @see Result
     */
    virtual Result* query(const char *) = 0;

};

#endif // CONNECTION_H
