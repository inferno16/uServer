#pragma once
#include <uWS.h>
#include <string>
#include <map>
#include <regex>
#include <vector>
#include "Database.h"

typedef uWS::Group<uWS::SERVER> Room;
typedef std::map<std::string, Room*> rooms_map;
typedef std::map<Room*, uWS::WebSocket<uWS::SERVER>*> room_owners;
typedef std::function<void(uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode)> message_handler;
typedef std::function<void(uWS::WebSocket<uWS::SERVER> *ws)> transfer_handler;
typedef std::function<void(uWS::WebSocket<uWS::SERVER> *ws, uWS::HttpRequest req)> connection_handler;
typedef std::function<void(uWS::WebSocket<uWS::SERVER> *ws, int code, char *message, size_t length)> disconnection_handler;

enum RoomStatus { RS_ACTIVE, RS_PENDING, RS_INVALID };

class RoomsManager
{
public:
	RoomsManager(uWS::Hub *h);
	RoomsManager(uWS::Hub *h, message_handler mh, connection_handler ch, disconnection_handler dh);
	~RoomsManager();
	std::string GetRoomID(const std::string &header_val);
	uWS::WebSocket<uWS::SERVER>* GetRoomOwner(Room* room);
	void SetRoomOwner(Room* room, uWS::WebSocket<uWS::SERVER>* ws);
	bool IsRoomMember(Room* room, uWS::WebSocket<uWS::SERVER>* ws);
	std::string GetUsername(const std::string &address);
	RoomStatus CheckRoomStatus(const std::string &id);
	Room* CreateNewRoom(const std::string &id, uWS::WebSocket<uWS::SERVER> *ws, message_handler mh, transfer_handler th,  disconnection_handler dh);
	Room* JoinRoom(const std::string &roomID, uWS::WebSocket<uWS::SERVER> *user);
private:
	void AttachBasicHandlers(Room* room, message_handler mh, transfer_handler th, connection_handler ch, disconnection_handler dh);
	std::vector<std::string> GetInfoFromHeader(const std::string &header_val);
private:
	rooms_map rooms;
	room_owners owners;
	uWS::Hub *hub;
	Database *db;
};