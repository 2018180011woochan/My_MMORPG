#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
using namespace std;

#ifdef _DEBUG
#pragma comment (lib, "lib/sfml-graphics-s-d.lib")
#pragma comment (lib, "lib/sfml-window-s-d.lib")
#pragma comment (lib, "lib/sfml-system-s-d.lib")
#pragma comment (lib, "lib/sfml-network-s-d.lib")
#else
#pragma comment (lib, "lib/sfml-graphics-s.lib")
#pragma comment (lib, "lib/sfml-window-s.lib")
#pragma comment (lib, "lib/sfml-system-s.lib")
#pragma comment (lib, "lib/sfml-network-s.lib")
#endif
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

#include "..\..\multi_iocp_server\multi_iocp_server\protocol.h"
#include "..\..\multi_iocp_server\multi_iocp_server\Enum.h"

sf::TcpSocket socket;

constexpr auto SCREEN_WIDTH = 16;
constexpr auto SCREEN_HEIGHT = 16;

constexpr auto TILE_WIDTH = 65;
constexpr auto TILE_HEIGHT = 65;
constexpr auto WINDOW_WIDTH = 65 * SCREEN_WIDTH + 10;   // size of window
constexpr auto WINDOW_HEIGHT = 65 * SCREEN_WIDTH + 10;

int g_left_x;
int g_top_y;

char Nickname[20] = "temp";

sf::RenderWindow* g_window;

void Login();

class OBJECT {
private:
	bool m_showing;
	sf::Sprite m_sprite;	
	sf::Text m_name;
	sf::Text m_level;
	sf::Text ui_m_name;
	
public:
	int id;
	int m_x, m_y;
	int level;
	int exp, maxexp;
	int hp, hpmax;
	char my_name[NAME_SIZE];

	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		set_name("NONAME", false);
		set_level(0);
	}
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2, sf::Texture& hpbar, int tx, int ty, int tx2, int ty2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		set_name("NONAME", false);
		set_level(0);
	}
	OBJECT() {
		m_showing = false;
	}
	void set_info(int _id, int _level, int _hp, int _hpmax, int _x, int _y)
	{
		id = _id;
		level = _level;
		hp = _hp;
		hpmax = _hpmax;
		m_x = _x;
		m_y = _y;
	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);

	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	
	void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * 65.0f + 8;
		float ry = (m_y - g_top_y) * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);

		m_name.setPosition(rx, ry - 40);
		g_window->draw(m_name);

		m_level.setPosition(rx - 40, ry - 40);
		g_window->draw(m_level);
	}
	void draw_hp() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * 65.0f + 8;
		float ry = (m_y - g_top_y) * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);

		m_name.setPosition(rx, ry - 40);
		g_window->draw(m_name);

		m_level.setPosition(rx - 40, ry - 40);
		g_window->draw(m_level);	
	}
	void draw_ui()
	{
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * 65.0f + 8;
		float ry = (m_y - g_top_y) * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);

		m_name.setPosition(rx, ry - 40);
		g_window->draw(m_name);

		m_level.setPosition(rx - 40, ry - 40);
		g_window->draw(m_level);

		ui_m_name.setPosition(0, 10);
		g_window->draw(ui_m_name);
	}
	void idraw()
	{
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * 65.0f + 8;
		float ry = (m_y - g_top_y) * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);
	}
	bool isShow()
	{
		return m_showing;
	}

	void set_name(const char str[], bool _ui) {
		//m_name.setFont(g_font);
		m_name.setString(str);
		m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);

		if (_ui) {
			//ui_m_name.setFont(g_font);
			ui_m_name.setString(str);
			ui_m_name.setFillColor(sf::Color(255, 255, 0));
			ui_m_name.setStyle(sf::Text::Bold);
		}
	}

	void set_level(const char level[]){
		//m_level.setFont(g_font);
		m_level.setString(level);
		m_level.setFillColor(sf::Color(255, 255, 0));
		m_level.setStyle(sf::Text::Bold);
	}
};

OBJECT avatar;
OBJECT players[MAX_USER];
OBJECT npcs[NUM_NPC];
OBJECT chaticon;
OBJECT chatUI;

vector<OBJECT> PlayerSkill;

OBJECT MapObj;

sf::Texture* board;
sf::Texture* pieces;
sf::Texture* skeleton;
sf::Texture* wraith;
sf::Texture* devil;
sf::Texture* diablo;
sf::Texture* AttackSource;
sf::Texture* HPBar;
sf::Texture* Chatimage;
sf::Texture* CharPicture;
sf::Texture* ChatUI;

void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	skeleton = new sf::Texture;
	wraith = new sf::Texture;
	devil = new sf::Texture;
	diablo = new sf::Texture;
	AttackSource = new sf::Texture;
	HPBar = new sf::Texture;
	Chatimage = new sf::Texture;
	CharPicture = new sf::Texture;
	ChatUI = new sf::Texture;

	board->loadFromFile("Texture/Map/map.bmp");
	pieces->loadFromFile("Texture/User/player.png");
	skeleton->loadFromFile("Texture/Monster/Skeleton.png");
	wraith->loadFromFile("Texture/Monster/wraith.png");
	devil->loadFromFile("Texture/Monster/Devil.png");
	diablo->loadFromFile("Texture/Monster/Diablo.png");
	AttackSource->loadFromFile("Texture/UserAttack/fire.png");
	HPBar->loadFromFile("Texture/User/HPBar.bmp");
	Chatimage->loadFromFile("Texture/User/chaticon.png");
	CharPicture->loadFromFile("Texture/User/CharPicture.png");
	ChatUI->loadFromFile("Texture/User/chatui.bmp");

	MapObj = OBJECT{ *board, 0, 0, 2000, 2000 };
	
	for (int i = 0; i < 8; ++i)
	{
		PlayerSkill.push_back(OBJECT{ *AttackSource, 0, 120, 60, 60 });
	}

	avatar = OBJECT{ *pieces, 65, 50, 200, 200, *HPBar, 0, 0, 89, 10 };
	avatar.move(4, 4);
	for (auto& pl : players) {
		pl = OBJECT{ *pieces, 50, 50, 200, 200, *HPBar, 0, 0, 89, 10 };
	}
	for (auto& npc : npcs) {
		npc = OBJECT{ *skeleton, 0, 0, 38, 73, *HPBar, 0, 0, 89, 10 };
	}

	chaticon = OBJECT{ *Chatimage, 0, 0, 90, 90 };
	chatUI = OBJECT{ *ChatUI, 0, 0, 400, 150 };
}

void client_finish()
{
	delete board;
	delete pieces;
	delete skeleton;
	delete wraith;
	delete devil;
	delete diablo;
	delete HPBar;
	delete Chatimage;
	delete CharPicture;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_OK:
	{
		SC_LOGIN_OK_PACKET* packet = reinterpret_cast<SC_LOGIN_OK_PACKET*>(ptr);
		avatar.id = packet->id;
		avatar.m_x = packet->x;
		avatar.m_y = packet->y;
		avatar.hp = packet->hp;
		avatar.hpmax = packet->hpmax;
		avatar.level = packet->level;
		avatar.exp = packet->exp;
		strcpy_s(avatar.my_name, Nickname);

		avatar.move(packet->x, packet->y);
		g_left_x = packet->x - 8;
		g_top_y = packet->y - 8;
		//players[packet->id].move(packet->x, packet->y);
		avatar.show();

		break;
	}
	case SC_LOGIN_FAIL:
	{
		
		break;
	}

	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(ptr);

		int id = packet->id;
		if (id < MAX_USER) {
			players[id].move(packet->x, packet->y);
			players[id].set_name(packet->name, false);
			char lev[10];
			sprintf_s(lev, "%d", packet->level);
			players[id].set_level(lev);
			players[id].set_info(packet->id, packet->level, packet->hp, packet->hpmax, packet->x, packet->y);
			players[id].show();
			strcpy_s(players[id].my_name, packet->name);
		}
		else {
			switch (packet->race)
			{
			case RACE_SKELETON:
				npcs[id - MAX_USER] = OBJECT{ *skeleton, 0, 0, 38, 73 , *HPBar, 0, 0, 89, 10 };
				break;
			case RACE_WRIATH:
				npcs[id - MAX_USER] = OBJECT{ *wraith, 0, 0, 138, 149 , *HPBar, 0, 0, 89, 10 };
				break;
			case RACE_DEVIL:
				npcs[id - MAX_USER] = OBJECT{ *devil, 0, 0, 161, 133 , *HPBar, 0, 0, 89, 10 };
				break;
			case RACE_DIABLO:
				npcs[id - MAX_USER] = OBJECT{ *diablo, 0, 0, 135, 158 , *HPBar, 0, 0, 89, 10 };
				break;
			default:
				break;
			}
			npcs[id - MAX_USER].move(packet->x, packet->y);
			char lev[10];
			sprintf_s(lev, "%d", packet->level);
			npcs[id - MAX_USER].set_level(lev);
			npcs[id - MAX_USER].set_name(packet->name, false);
			npcs[id - MAX_USER].set_info(packet->id, packet->level, packet->hp, packet->hpmax, packet->x, packet->y);
			npcs[id - MAX_USER].show();
		}
		break;
	}
	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET* packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(ptr);

		if (packet->id == avatar.id) {				// 본인 캐릭터 이동
			avatar.move(packet->x, packet->y);
			g_left_x = packet->x - 8;
			g_top_y = packet->y - 8;
		}
		else if (packet->id < MAX_USER) {			// 다른 유저 캐릭터 이동
			players[packet->id].move(packet->x, packet->y);
		}
		else {
			npcs[packet->id - MAX_USER].move(packet->x, packet->y);
		}

		break;
	}

	case SC_STAT_CHANGE:
	{
		break;
	}

	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(ptr);
		int other_id = packet->id;
		if (other_id == avatar.id) 
			avatar.hide();
		else if (other_id < MAX_USER) 
			players[other_id].hide();
		else 
			npcs[other_id - MAX_USER].hide();
		break;
	}
	case SC_PARTY:
	{
		
		break;
	}
	case SC_CHAT:
	{
		
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
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

unsigned int cnt = 0;

void client_main()
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i) {
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if (((tile_x + tile_y) % 2) <= 1) {

				MapObj.a_move(65 * i + 1, 65 * j + 1);
				MapObj.a_draw();
			}
		}
	}

	avatar.draw_ui();
	for (int i = 0; i < 8; ++i)
		PlayerSkill[i].idraw();
	for (auto& pl : players) pl.draw();
	for (auto& pl : npcs) pl.draw_hp(); 
	chaticon.a_move(0, 900);
	chaticon.a_draw();
	chatUI.a_move(600, 900);
	chatUI.a_draw();
}

void send_packet(void *packet)
{
	unsigned char *p = reinterpret_cast<unsigned char *>(packet);
	size_t sent = 0;
	socket.send(packet, p[0], sent);
}

void Login()
{
	int db_id = 0;

	CS_LOGIN_PACKET p;
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	p.db_id = db_id;

	strcpy_s(p.name, Nickname);
	send_packet(&p);
}

int main()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = socket.connect("127.0.0.1", PORT_NUM);
	socket.setBlocking(false);

	Login();

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	client_initialize();
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Simplest_MMORPG");
	g_window = &window;

	sf::Vector2i pos;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				int direction = -1;
				switch (event.key.code) {
				case sf::Keyboard::Left:
					direction = DIRECTION::DIRECTION_LEFT;
					break;
				case sf::Keyboard::Right:
					direction = DIRECTION::DIRECTION_RIGHT;
					break;
				case sf::Keyboard::Up:
					direction = DIRECTION::DIRECTION_UP;
					break;
				case sf::Keyboard::Down:
					direction = DIRECTION::DIRECTION_DOWN;
					break;
				case sf::Keyboard::A:	// 공격
					CS_ATTACK_PACKET p;
					p.size = sizeof(CS_ATTACK_PACKET);
					p.type = CS_ATTACK;
					send_packet(&p);
					break;
				case sf::Keyboard::Escape:
					window.close();
					break;
				}
				if (-1 != direction) {
					CS_MOVE_PACKET p;
					p.size = sizeof(p);
					p.type = CS_MOVE;
					p.direction = direction;
					send_packet(&p);
				}
				
			}
		}

		window.clear();
		client_main();
		window.display();
	}
	client_finish();

	return 0;
}


