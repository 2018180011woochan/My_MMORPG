//#pragma region test
//#include "pch.h"
//
//#include <iostream>
//
//#include "ThreadManager.h"
//#include "Service.h"
//#include "Session.h"
//#include "BufferReader.h"
//
//#include "ClientPacketHandler.h"
//
//#include "Object.h"
//
//#include "../../Server/protocol.h"
//
//
////sf::TcpSocket socket;
//
//constexpr auto SCREEN_WIDTH = 9;
//constexpr auto SCREEN_HEIGHT = 9;
//
//constexpr auto TILE_WIDTH = 65;
//constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;   // size of window
//constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + 10;
//
//Object avatar;
//Object players[MAX_USER];
//
//Object white_tile;
//Object black_tile;
//
//sf::Texture* board;
//sf::Texture* pieces;
//
//void client_initialize()
//{
//	board = new sf::Texture;
//	pieces = new sf::Texture;
//	board->loadFromFile("chessmap.bmp");
//	pieces->loadFromFile("chess2.png");
//	white_tile = Object{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
//	black_tile = Object{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
//	avatar = Object{ *pieces, 128, 0, 64, 64 };
//	avatar.move(4, 4);
//	for (auto& pl : players) {
//		pl = Object{ *pieces, 64, 0, 64, 64 };
//	}
//}
//
//void client_finish()
//{
//	delete board;
//	delete pieces;
//}
//
//class ServerSession : public PacketSession
//{
//public:
//	~ServerSession()
//	{
//		cout << "~ServerSession" << endl;
//	}
//
//	virtual void OnConnected() override
//	{
//	}
//
//	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
//	{
//		ClientPacketHandler::HandlePacket(buffer, len);
//	}
//
//	virtual void OnSend(int32 len) override
//	{
//		//cout << "OnSend Len = " << len << endl;
//
//	}
//
//	virtual void OnDisconnected() override
//	{
//		//cout << "DisConnected" << endl;
//	}
//};
//
//int main()
//{
//	wcout.imbue(locale("korean"));
//
//	ClientServiceRef service = MakeShared<ClientService>(
//		NetAddress(L"127.0.0.1", PORT_NUM),
//		MakeShared<IocpCore>(),
//		MakeShared<ServerSession>,
//		1 /*동접*/);
//
//	service->Start();
//
//	client_initialize();
//	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
//	g_window = &window;
//
//	for (int32 i = 0; i < 2; ++i)
//	{
//		GThreadManager->Launch([=]()
//			{
//				while (true)
//				{
//					service->GetIocpCore()->Dispatch();
//				}
//			});
//	}
//	GThreadManager->Join();
//
//	
//}
//#pragma endregion

#include "pch.h"

//#include <iostream>
//using namespace std;

#include "Object.h"
#include "..\..\Server\protocol.h"

sf::TcpSocket mysocket;

constexpr auto SCREEN_WIDTH = 9;
constexpr auto SCREEN_HEIGHT = 9;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;   // size of window
constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + 10;

int g_left_x;
int g_top_y;
int g_myid;
sf::RenderWindow* g_window;

Object avatar;
Object players[MAX_USER];

Object white_tile;
Object black_tile;

sf::Texture* board;
sf::Texture* pieces;

void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	board->loadFromFile("chessmap.bmp");
	pieces->loadFromFile("chess2.png");
	white_tile = Object{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
	black_tile = Object{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
	avatar = Object{ *pieces, 128, 0, 64, 64 };
	avatar.move(4, 4);
	for (auto& pl : players) {
		pl = Object{ *pieces, 64, 0, 64, 64 };
	}
}

void client_finish()
{
	delete board;
	delete pieces;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	//switch (ptr[1])
	//{
	//case SC_LOGIN_OK:
	//{
	//	sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(ptr);
	//	g_myid = packet->id;
	//}

	//case SC_PUT_PLAYER:
	//{
	//	sc_packet_put_player* my_packet = reinterpret_cast<sc_packet_put_player*>(ptr);
	//	int id = my_packet->id;

	//	if (id == g_myid) {
	//		avatar.move(my_packet->x, my_packet->y);
	//		g_left_x = my_packet->x - 4;
	//		g_top_y = my_packet->y - 4;
	//		avatar.show();
	//	}
	//	else if (id < MAX_USER) {
	//		players[id].move(my_packet->x, my_packet->y);
	//		players[id].show();
	//	}
	//	else {
	//		//npc[id - NPC_START].x = my_packet->x;
	//		//npc[id - NPC_START].y = my_packet->y;
	//		//npc[id - NPC_START].attr |= BOB_ATTR_VISIBLE;
	//	}
	//	break;
	//}
	//case SC_POS:
	//{
	//	sc_packet_pos* my_packet = reinterpret_cast<sc_packet_pos*>(ptr);
	//	int other_id = my_packet->id;
	//	if (other_id == g_myid) {
	//		avatar.move(my_packet->x, my_packet->y);
	//		g_left_x = my_packet->x - 4;
	//		g_top_y = my_packet->y - 4;
	//	}
	//	else if (other_id < MAX_USER) {
	//		players[other_id].move(my_packet->x, my_packet->y);
	//	}
	//	else {
	//		//npc[other_id - NPC_START].x = my_packet->x;
	//		//npc[other_id - NPC_START].y = my_packet->y;
	//	}
	//	break;
	//}

	//case SC_REMOVE_PLAYER:
	//{
	//	sc_packet_remove_player* my_packet = reinterpret_cast<sc_packet_remove_player*>(ptr);
	//	int other_id = my_packet->id;
	//	if (other_id == g_myid) {
	//		avatar.hide();
	//	}
	//	else if (other_id < MAX_USER) {
	//		players[other_id].hide();
	//	}
	//	else {
	//		//		npc[other_id - NPC_START].attr &= ~BOB_ATTR_VISIBLE;
	//	}
	//	break;
	//}
	//default:
	//	printf("Unknown PACKET type [%d]\n", ptr[1]);
	//}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void client_main()
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = mysocket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if (((tile_x + tile_y) % 6) < 3) {
				white_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				white_tile.a_draw(g_window);
			}
			else
			{
				black_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				black_tile.a_draw(g_window);
			}
		}
	avatar.draw(g_window, g_left_x, g_top_y);
	for (auto& pl : players) pl.draw(g_window, g_left_x, g_top_y);
}

void send_packet(int p_type)
{
	/*cs_packet_up packet;
	packet.size = sizeof(packet);
	packet.type = p_type;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);*/
}

int main()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = mysocket.connect("127.0.0.1", PORT_NUM);
	mysocket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	client_initialize();

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				int p_type = -1;
				switch (event.key.code) {
				case sf::Keyboard::Left:
					//p_type = CS_LEFT;
					break;
				case sf::Keyboard::Right:
					//p_type = CS_RIGHT;
					break;
				case sf::Keyboard::Up:
					//p_type = CS_UP;
					break;
				case sf::Keyboard::Down:
					//p_type = CS_DOWN;
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				}
				if (-1 != p_type) send_packet(p_type);

			}
		}

		window.clear();
		client_main();
		window.display();
	}
	client_finish();

	return 0;
}