#pragma once
#include <uWS.h>
#include <string>
#include <map>
#include <regex>
#include <vector>
#include "Database.h"

typedef uWS::Group<uWS::SERVER> Room;
typedef std::map<std::string, Room*> rooms_map;
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
	std::string GetRoomID(std::string header_val);
	RoomStatus CheckRoomStatus(std::string id);
	Room* CreateNewRoom(std::string id, uWS::WebSocket<uWS::SERVER> *ws, message_handler mh, transfer_handler th,  disconnection_handler dh);
	bool JoinRoom(std::string roomID, uWS::WebSocket<uWS::SERVER> *user);
private:
	void AttachBasicHandlers(Room* room, message_handler mh, transfer_handler th, connection_handler ch, disconnection_handler dh);
	std::vector<std::string> GetInfoFromHeader(const std::string &header_val);
private:
	rooms_map rooms;
	uWS::Hub *hub;
	Database *db;
};