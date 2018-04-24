#include <uWS.h>
#include <iostream>
#include <string>
#include <map>

//#define MAX_ROOMID_LEN 10

//typedef char room_id[MAX_ROOMID_LEN];
typedef std::map<std::string, uWS::Group<uWS::SERVER>*> rooms_map;

enum RoomStatus { RS_ACTIVE, RS_PENDING, RS_INVALID };

class Statistics {
public:
	Statistics() : max_conn(0), curr_conn(0), total_conn(0) {}
	size_t max_connections() const { return max_conn; }
	size_t current_connections() const { return curr_conn; }
	size_t total_connections() const { return total_conn; }
	void new_connection() { curr_conn++; total_conn++; if (curr_conn > max_conn) { max_conn = curr_conn; } }
	void new_disconnection() { curr_conn--; if (curr_conn < 0) { std::cerr << "Stats show less than zero connections!\n"; } }
private:
	size_t max_conn;
	size_t curr_conn;
	size_t total_conn;
};

std::string getRoomID(const char *header_val) {
	size_t i = 0;

	while (header_val[i] != '\0') { // ToDo: This doesn't work this way. PLS FIX
		if (header_val[i] == ' ' && i > 0) {
			return std::string(&header_val[1], i - 1);
		}
		i++;
	}
	return "";
}

RoomStatus CheckRoomStatus(std::string id, const rooms_map &rooms) {
	if (rooms.find(id) != rooms.end()) {
		return RS_ACTIVE;
	}
	else {
		if (1) {
			//ToDo: fetch info from the HTTP server
			return RS_PENDING;
		}
		return RS_INVALID;
	}
}

void msgHandler(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
	uWS::Group<uWS::SERVER> *lg = uWS::Group<uWS::SERVER>::from(ws);
	lg->broadcast(message, length, opCode);
}

uWS::Group<uWS::SERVER> *g;

int main() {
	uWS::Hub h;
	Statistics stats;
	rooms_map rooms;

	rooms.insert(std::make_pair("default", &h.getDefaultGroup<uWS::SERVER>()));

	h.onMessage(msgHandler);
	
	h.onConnection([&stats, &rooms, &h](uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {

		stats.new_connection();
		std::string id = getRoomID((req.headers)->value);
		std::cout << "Connection No " << stats.total_connections() << "(active: " << stats.current_connections() << "), Room ID: " << id << std::endl;
		RoomStatus rs = CheckRoomStatus(id, rooms);
		if (rs == RS_ACTIVE) {
			ws->transfer(rooms.at(id));
		}
		if (rs == RS_PENDING) {
			g = h.createGroup<uWS::SERVER>();
			g->onMessage(msgHandler);
			//ToDo: onDisconnect shouldn't be lambda + I need to close empty rooms
			g->onDisconnection([&stats](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) { stats.new_disconnection(); });
			ws->transfer(g);
			rooms.insert(std::make_pair(id, g));
		}
	});


	h.onDisconnection([&stats](uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
		stats.new_disconnection();
	});

	if (h.listen("192.168.0.100", 3000)) {
		h.run();
	}
	return 0;
}