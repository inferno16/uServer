#pragma once
class Statistics {
public:
	Statistics() : max_conn(0), curr_conn(0), total_conn(0) {}
	size_t max_connections() const { return max_conn; }
	size_t current_connections() const { return curr_conn; }
	size_t total_connections() const { return total_conn; }
	void new_connection() { curr_conn++; total_conn++; if (curr_conn > max_conn) { max_conn = curr_conn; } }
	void new_disconnection() { curr_conn--; if (curr_conn < 0) { throw ""; /*ToDo: handle this case*/ } }

	friend std::ostream &operator << (std::ostream &stream, const Statistics &obj) {
		stream << "Current connections: " << obj.curr_conn << ", Total: " << obj.total_conn << ", Peak: " << obj.max_conn << std::endl;
		return stream;
	}
private:
	size_t max_conn;
	size_t curr_conn;
	size_t total_conn;
};