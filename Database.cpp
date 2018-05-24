#include "Database.h"

Database::Database(db_strc &host, const unsigned &port, db_strc &user, db_strc &pass, db_strc &schema)
	: __started_threads(0), __finished_threads(0)
{
	std::string conn_str = host.asStdString();
	if (conn_str.substr(0, 6) != "tcp://") { // If it is just a host name/address
		conn_str = "tcp://" + conn_str; // Add the tcp protocol in the beginning
	}
	conn_str += ':' + std::to_string(port); // Add the port
	// Store the data
	_driver = sql::mysql::get_driver_instance();
	_conn_str = conn_str;
	_user = user;
	_pass = pass;
	_schema = schema;
	
	if (!TestConnection()) {
		std::cerr << "Database connection test failed!\n";
		exit(EXIT_FAILURE);
	}
}

Database::~Database()
{
	// Wait for database daemons
	bool first = true;
	while (!this->__all_threads_finished()) {
		if (first) {
			std::cout << "Not all of the Database daemons are done. The program will close once all the daemons are closed.\n";
			first = false;
		}
		std::cout << "Waiting for 5 more seconds\n";
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

bool Database::RoomExists(std::string roomID)
{
	bool exists = false;
	sql::Connection *conn = nullptr;
	sql::PreparedStatement *pstmt = nullptr;
	sql::ResultSet *rs = nullptr;
	try
	{
		if (CreateConnection(&conn)) {
			pstmt = conn->prepareStatement("SELECT * FROM rooms WHERE stream_key=?");
			pstmt->setString(1, roomID);
			rs = pstmt->executeQuery();
			exists = (rs->rowsCount() > 0);
		}
	}
	catch (sql::SQLException e)
	{
		std::cerr << e.what() << std::endl;
	}
	CleanUp(&conn, &rs, &pstmt);
	return exists;
}

void Database::__call_wrapper(void (Database::*func)(sql::Connection*))
{
	this->__new_thread_started();
	std::thread t([this, func]() {
		_driver->threadInit();
		sql::Connection *conn = nullptr;

		try {
			if (CreateConnection(&conn)) {
				(this->*func)(conn); // Call the function;
			}
		} 
		catch (sql::SQLException e) {
			std::cerr << e.what() << std::endl;
		}

		CleanUp(&conn);
		_driver->threadEnd();
		this->__new_thread_finished();
	});
	t.detach(); // Detaching the thread, thus making it a daemon
}

void Database::__new_thread_started()
{
	std::lock_guard<std::mutex> g(this->__mutex_started);
	this->__started_threads++;
}

void Database::__new_thread_finished()
{
	std::lock_guard<std::mutex> g(this->__mutex_finished);
	this->__finished_threads++;
}

bool Database::__all_threads_finished()
{
	std::lock_guard<std::mutex> g_s(this->__mutex_started);
	std::lock_guard<std::mutex> g_f(this->__mutex_finished);
	return (this->__finished_threads == this->__started_threads);
}

bool Database::TestConnection()
{
	bool success = false;
	sql::Connection *conn = nullptr;
	sql::Statement *stmt = nullptr;
	sql::ResultSet *rs = nullptr;
	try
	{
		if (CreateConnection(&conn, &stmt)) {
			rs = stmt->executeQuery("SHOW TABLES");
			success = (rs->rowsCount() > 0);
		}
	}
	catch (sql::SQLException e)
	{
		std::cerr << e.what() << std::endl;
		success = false;
	}
	CleanUp(&conn, &stmt, &rs);
	return success;
}

bool Database::CreateConnection(sql::Connection **conn, sql::Statement **stmt)
{
	if (*conn != nullptr || *stmt != nullptr) {
		std::cerr << "Can't create connection on non-nullptr variables. Please use CleanUp first.\n";
		return false;
	}
	*conn = _driver->connect(_conn_str, _user, _pass);
	(*conn)->setSchema(_schema);
	*stmt = (*conn)->createStatement();
	return true;
}

bool Database::CreateConnection(sql::Connection ** conn)
{
	if (*conn != nullptr) {
		std::cerr << "Can't create connection on non-nullptr variables. Please use CleanUp first.\n";
		return false;
	}
	*conn = _driver->connect(_conn_str, _user, _pass);
	(*conn)->setSchema(_schema);
	return true;
}
