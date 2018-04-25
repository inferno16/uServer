#include <uWS.h>
#include <iostream>
#include "Statistics.h"
#include "Rooms.h"

#define SERVER_ADDR "192.168.0.100"
#define SERVER_PORT 3000

Statistics stats;
RoomsManager *rm = nullptr;

void msgHandler(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
	Room::from(ws)->broadcast(message, length, opCode);
}

void discHandler_g(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
	stats.new_disconnection();
}

void discHandler(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
	discHandler_g(ws, code, message, length);
	std::string msgstr = "room: User disconnected from the room.";
	Room::from(ws)->broadcast(msgstr.c_str(), msgstr.size(), uWS::OpCode::TEXT);
}

void transHandler(uWS::WebSocket<uWS::SERVER> *ws) {
	std::string message = "room: User connected to the room.";
	Room::from(ws)->broadcast(message.c_str(), message.size(), uWS::OpCode::TEXT);
}

void connHandler_g(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
	stats.new_connection();
	std::string id = rm->GetRoomID(std::string((req.headers)->value));
	std::cout << "Connection No " << stats.total_connections() << "(active: " << stats.current_connections() << "), ";
	RoomStatus rs = rm->CheckRoomStatus(id);

	switch (rs) {
	case RS_ACTIVE:
		if (!rm->JoinRoom(id, ws)) {
			//ToDo: Can log this later on
			std::cerr << "Error joining";
		}
		else {
			std::cout << "joined ";
		}
		break;
	case RS_PENDING:
		if (rm->CreateNewRoom(id, ws, msgHandler, transHandler, discHandler) == nullptr) {
			//ToDo: Can log this later on
			std::cerr << "Error creating";
		}
		else {
			std::cout << "created ";
		}
		break;
	case RS_INVALID:
		//ToDo: Can log this later on
		std::cout << "tried accessing invalid ";
		ws->terminate(); // not sure if this is the correct way of closing the connection
		break;
	default:
		//ToDo: Can log this later on
		break;
	}
	std::cout << "room with ID: " << id << std::endl;
}

int main() {
	uWS::Hub h;
	rm = new RoomsManager(&h, msgHandler, connHandler_g, discHandler_g);

	if (h.listen(SERVER_ADDR, SERVER_PORT)) {
		h.run();
	}
	else {
		//ToDo: Can log this later on
	}

	return 0;
}