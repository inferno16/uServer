#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include "mysql_connection.h"
#include "cppconn\driver.h"
#include "mysql_driver.h"
#include "cppconn\resultset.h"
#include "cppconn\statement.h"
#include "cppconn\prepared_statement.h"
#include "mysql_error.h"

typedef const sql::SQLString db_strc;
typedef sql::SQLString db_str;

class Database
{
	// Public functions
public:
	Database(db_strc &host, const unsigned &port, db_strc &user, db_strc &pass, db_strc &schema);
	~Database();
	bool RoomExists(const std::string &roomID);
	std::string GetUsername(const std::string &address);
	// Private functions
private:
	// Synchronization functions
	template <typename T>
	void __call_wrapper(void (Database::* func)(sql::Connection*, T), T param); // This SHOULD be used to make async database calls
	void __new_thread_started();
	void __new_thread_finished();
	bool __all_threads_finished();
	// Regular functions
	template <typename T>
	void CleanUp(T t);
	template <typename T, typename... Args>
	void CleanUp(T t, Args... args);
	bool TestConnection();
	bool CreateConnection(sql::Connection** conn, sql::Statement** stmt);
	bool CreateConnection(sql::Connection** conn);
	void DeleteFromTempUsers(sql::Connection* conn, unsigned id);

	// Private variables
private:
	// Synchronization variables
	unsigned __started_threads;
	unsigned __finished_threads;
	std::mutex __mutex_started;
	std::mutex __mutex_finished;
	// Regular varables
	sql::Driver *_driver;
	db_str _conn_str;
	db_str _user;
	db_str _pass;
	db_str _schema;
};

template<typename T>
void Database::CleanUp(T t)
{
	if (*t != nullptr) {
		delete *t;
		*t = nullptr;
	}
}

template<typename T, typename ...Args>
void Database::CleanUp(T t, Args ...args)
{
	if (*t != nullptr) {
		delete *t;
		*t = nullptr;
	}

	CleanUp(args...);
}