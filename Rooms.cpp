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

std::string RoomsManager::GetRoomID(const std::string &header_val) {
	std::string id = "";
	std::smatch mr;
	std::regex rxp("^\\/([A-Za-z0-9_\\-]+)\\s");
	if (std::regex_search(header_val, mr, rxp)) {
		id = mr[1];
	}
	return id;
}

uWS::WebSocket<uWS::SERVER>* RoomsManager::GetRoomOwner(Room * room)
{
	if(owners.find(room) == owners.end())
		return nullptr;
	return owners.at(room);
}

void RoomsManager::SetRoomOwner(Room * room, uWS::WebSocket<uWS::SERVER>* ws)
{
	if (owners.find(room) == owners.end())
		return;
	owners.at(room) = ws;
}

bool RoomsManager::IsRoomMember(Room * room, uWS::WebSocket<uWS::SERVER>* ws)
{
	return (Room::from(ws) == room);
}

std::string RoomsManager::GetUsername(const std::string &address)
{
	return db->GetUsername(address);
}

RoomStatus RoomsManager::CheckRoomStatus(const std::string &id) {
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

Room* RoomsManager::CreateNewRoom(const std::string &id, uWS::WebSocket<uWS::SERVER> *ws, message_handler mh, transfer_handler th, disconnection_handler dh)
{
	if (rooms.find(id) != rooms.end())
		return nullptr;
	Room *r = nullptr;
	r = hub->createGroup<uWS::SERVER>();
	this->AttachBasicHandlers(r, mh, th, NULL, dh);
	ws->transfer(r);
	rooms.insert(std::make_pair(id, r));
	owners.insert(std::make_pair(r, ws));
	return r;
}

Room* RoomsManager::JoinRoom(const std::string &roomID, uWS::WebSocket<uWS::SERVER>* user)
{
	rooms_map::iterator it = rooms.find(roomID);
	if (it != rooms.end()) {
		user->transfer(it->second);
		return it->second;
	}
	return nullptr;
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