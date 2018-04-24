#include "Rooms.h"



RoomsManager::RoomsManager(uWS::Hub *h)
	:hub(h)
{
	rooms.insert(std::make_pair("default", &h->getDefaultGroup<uWS::SERVER>()));
}

RoomsManager::RoomsManager(uWS::Hub * h, message_handler mh, connection_handler ch, disconnection_handler dh)
	:RoomsManager(h)
{
	this->AttachBasicHandlers(rooms.at("default"), mh, ch, dh);
}


RoomsManager::~RoomsManager()
{
}

std::string RoomsManager::GetRoomID(std::string header_val) {
	size_t i = 0;

	while (header_val[i] != '\0') { // ToDo: This doesn't work this way. PLS FIX
		if (header_val[i] == ' ' && i > 0) {
			return std::string(&header_val[1], i - 1);
		}
		i++;
	}
	return "";
}

RoomStatus RoomsManager::CheckRoomStatus(std::string id) {
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

Room* RoomsManager::CreateNewRoom(std::string id, uWS::WebSocket<uWS::SERVER> *ws, message_handler mh, connection_handler ch, disconnection_handler dh)
{
	if (rooms.find(id) == rooms.end())
		return nullptr;
	Room *r = nullptr;
	r = hub->createGroup<uWS::SERVER>();
	ws->transfer(r);
	this->AttachBasicHandlers(r, mh, ch, dh);
	rooms.insert(std::make_pair(id, r));
	return r;
}

bool RoomsManager::JoinRoom(std::string roomID, uWS::WebSocket<uWS::SERVER>* user)
{
	if (rooms.find(roomID) == rooms.end())
		return false;
	user->transfer(rooms.at(roomID));
	return true;
}

void RoomsManager::AttachBasicHandlers(Room* room, message_handler mh, connection_handler ch, disconnection_handler dh)
{
	room->onMessage(mh);
	room->onConnection(ch);
	room->onDisconnection(dh);
}
