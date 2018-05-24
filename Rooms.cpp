#include "Rooms.h"



RoomsManager::RoomsManager(uWS::Hub *h)
	:hub(h)
{
	rooms.insert(std::make_pair("default", &h->getDefaultGroup<uWS::SERVER>()));
	db = new Database("127.0.0.1", 3306, "root", "", "letswatch");
}

RoomsManager::RoomsManager(uWS::Hub * h, message_handler mh, connection_handler ch, disconnection_handler dh)
	:RoomsManager(h)
{
	this->AttachBasicHandlers(rooms.at("default"), mh, NULL, ch, dh);
}


RoomsManager::~RoomsManager()
{
	delete db;
}

std::string RoomsManager::GetRoomID(std::string header_val) {
	std::string id = "";
	std::smatch mr;
	std::regex rxp("^\\/([A-Za-z0-9_\\-]+)\\s");
	if (std::regex_search(header_val, mr, rxp)) {
		id = mr[1];
	}
	return id;
}

RoomStatus RoomsManager::CheckRoomStatus(std::string id) {
	if (rooms.find(id) != rooms.end()) {
		return RS_ACTIVE;
	}
	else {
		if (db->RoomExists(id)) {
			return RS_PENDING;
		}
		return RS_INVALID;
	}
}

Room* RoomsManager::CreateNewRoom(std::string id, uWS::WebSocket<uWS::SERVER> *ws, message_handler mh, transfer_handler th, disconnection_handler dh)
{
	if (rooms.find(id) != rooms.end())
		return nullptr;
	Room *r = nullptr;
	r = hub->createGroup<uWS::SERVER>();
	ws->transfer(r);
	this->AttachBasicHandlers(r, mh, th, NULL, dh);
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

void RoomsManager::AttachBasicHandlers(Room* room, message_handler mh, transfer_handler th, connection_handler ch, disconnection_handler dh)
{
	room->onMessage(mh);
	room->onTransfer(th);
	room->onConnection(ch);
	room->onDisconnection(dh);
}

std::vector<std::string> RoomsManager::GetInfoFromHeader(const std::string & header_val)
{
	std::vector<std::string> v;
	std::smatch mr;
	std::regex rxp("^\\/(.+)\\/([A-Za-z0-9._%\\-]+)\\s"); // /^\/(.+)\/([A-Za-z0-9._%\-]+)\s/ -> "/(RoomID)/(Username) "
	if (std::regex_search(header_val, mr, rxp)) {
		if (mr.size() != 3) {
			throw std::exception("Data parsing error", 447453);
		}
		for (size_t i = 0; i < 2; i++) {
			v.push_back(mr[i + 1].str());
		}
	}
	return v;
}