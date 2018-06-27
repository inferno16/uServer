#include <uWS.h>
#include <iostream>
#include "Statistics.h"
#include "Rooms.h"
#include "json.h"

#define SERVER_ADDR "192.168.0.100"
#define SERVER_PORT 3000

typedef std::map<uWS::WebSocket<uWS::SERVER> *, std::string> UserMap;
typedef std::map<std::string, uWS::WebSocket<uWS::SERVER> *> ReverseUserMap;

Statistics stats;
RoomsManager *rm = nullptr;
UserMap *um = new UserMap();
ReverseUserMap waiting;


void eraseUser(UserMap::iterator it) {
	if (it != um->end())
		um->erase(it);
}

void promoteUser(UserMap::iterator it, Room *room) {
	if (it == um->end()) { 
		// ToDo: Can log this later on
		return; 
	}
	std::string owner = Json::GenerateUserJSON("promotion", it->second);
	it->first->send(owner.c_str(), owner.length(), uWS::TEXT);
	rm->SetRoomOwner(room, it->first);
	std::cout << "Room owner is " << it->second << std::endl;
}

void msgHandler(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
	Json::JSONMap m;
	std::string msg = std::string(message, length);
	try
	{
		m = Json::CreateMapFromJSON(msg);
		Json::JSONMap::iterator cmd = m.find("command");
		Json::JSONMap::iterator obj = m.find("object");
		Json::JSONMap::iterator usr = m.find("user");
		if (obj != m.end() && cmd != m.end() && usr != m.end()) {
			if (obj->second == "user" && cmd->second == "introduction") {
				Json::JSONMap::iterator val = m.find("value");
				ReverseUserMap::iterator user_entry = waiting.find(usr->second);
				if (val != m.end() && user_entry != waiting.end()) {
					user_entry->second->send(message, length, opCode);
					waiting.erase(user_entry);
				}
			}
			else {
				Room::from(ws)->broadcast(message, length, opCode);
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		// Malformed JSON
		// ToDo: Can log this later on
	}
}

void discHandler_g(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
	stats.new_disconnection();
}

void discHandler(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length) {
	discHandler_g(ws, code, message, length);
	UserMap::iterator it = um->find(ws);
	if (it == um->end()) { return; }
	std::string msg = Json::GenerateUserJSON("disconnection", it->second);
	eraseUser(it);
	Room *r = Room::from(ws);
	if (rm->GetRoomOwner(r) == ws) {
		std::cout << "Room owner left!\n";
		rm->SetRoomOwner(r, nullptr);
		for (UserMap::iterator it = um->begin(); it != um->end(); it++)
		{
			if (Room::from(it->first) == r) {
				promoteUser(it, r);
				break;
			}
		}
	}
	r->broadcast(msg.c_str(), msg.size(), uWS::OpCode::TEXT);
}

void transHandler(uWS::WebSocket<uWS::SERVER> *ws) {
	UserMap::iterator it = um->find(ws);
	if (it == um->end()) { return; }
	std::string msg = Json::GenerateUserJSON("connection", it->second);
	Room::from(ws)->broadcast(msg.c_str(), msg.size(), uWS::OpCode::TEXT);
}

void connHandler_g(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req) {
	stats.new_connection();
	std::string id = rm->GetRoomID(std::string((req.headers)->value));
	std::cout << "Connection No " << stats.total_connections() << "(active: " << stats.current_connections() << "), ";
	RoomStatus rs = rm->CheckRoomStatus(id);
	Room *room;

	std::string username = rm->GetUsername(ws->getAddress().address);
	if (username.empty()) {
		ws->terminate();
		std::cout << "user forcefully dropped from room with ID: " << id << std::endl;
	}
	else {
		um->insert(std::make_pair(ws, username));
		switch (rs) {
		case RS_ACTIVE:
			room = rm->JoinRoom(id, ws);
			if (room == nullptr) {
				//ToDo: Can log this later on
				std::cerr << "Error joining";
			}
			else {
				std::cout << "joined room with ID: " << id << std::endl;
				if(rm->GetRoomOwner(room) == nullptr)
					promoteUser(um->find(ws), room);
				else
					waiting.insert(std::make_pair(username, ws));
			}
			break;
		case RS_PENDING:
			room = rm->CreateNewRoom(id, ws, msgHandler, transHandler, discHandler);
			if (room == nullptr) {
				//ToDo: Can log this later on
				std::cerr << "Error creating";
			}
			else {
				std::cout << "created room with ID: " << id << std::endl;
				promoteUser(um->find(ws), room);
			}
			break;
		case RS_INVALID:
			//ToDo: Can log this later on
			std::cout << "tried accessing invalid room with ID: " << id << std::endl;
			eraseUser(um->find(ws));
			ws->terminate(); // not sure if this is the correct way of closing the connection
			break;
		default:
			//ToDo: Can log this later on
			break;
		}
	}
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

	delete rm;
	rm = nullptr;
	delete um;
	um = nullptr;
	return 0;
}