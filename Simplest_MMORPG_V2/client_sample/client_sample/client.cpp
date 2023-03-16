#include "pch.h"

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"
#include "ServerSession.h"

#include "Object.h"
#include "ClientPacketHandler.h"
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
	//delete mySession;
}

void process_data(char* net_buf, size_t io_byte)
{
	ClientPacketHandler::HandlePacket((BYTE*)net_buf, io_byte);
	avatar.show();
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

int main()
{
	wcout.imbue(locale("korean"));

	sf::Socket::Status status = mysocket.connect("127.0.0.1", PORT_NUM);
	mysocket.setBlocking(false);

	ClientServiceRef service = MakeShared<ClientService>(NetAddress(L"127.0.0.1", PORT_NUM), make_shared<IocpCore>(),
		MakeShared<ServerSession>, 1);

	service->Start();

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	client_initialize();

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;

	while (window.isOpen())
	{
		for (int32 i = 0; i < 2; i++)
		{
			GThreadManager->Launch([=]()
				{

					service->GetIocpCore()->Dispatch();

				});
		}

		sf::Event event;
		SendBufferRef sendBuffer;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				int p_type = -1;
				switch (event.key.code) {
				case sf::Keyboard::Left:
					sendBuffer = ClientPacketHandler::Make_CS_MOVE(avatar.GetID(), DIRECTION::LEFT);
					mysession->Send(sendBuffer);

					break;
				case sf::Keyboard::Right:
					sendBuffer = ClientPacketHandler::Make_CS_MOVE(avatar.GetID(), DIRECTION::RIGHT);
					mysession->Send(sendBuffer);
					break;
				case sf::Keyboard::Up:
					sendBuffer = ClientPacketHandler::Make_CS_MOVE(avatar.GetID(), DIRECTION::UP);
					mysession->Send(sendBuffer);
					break;
				case sf::Keyboard::Down:
					sendBuffer = ClientPacketHandler::Make_CS_MOVE(avatar.GetID(), DIRECTION::DOWN);
					mysession->Send(sendBuffer);
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				}
			}
		}

		window.clear();
		client_main();
		window.display();
	}
	client_finish();
}