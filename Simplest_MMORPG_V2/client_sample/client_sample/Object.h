#pragma once

class Object
{
private:
	bool m_showing;
	sf::Sprite m_sprite;
	int Player_ID = 0;
public:
	int m_x, m_y;
	Object(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
	}
	Object() {
		m_showing = false;
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

	void a_draw(sf::RenderWindow* window) {
		window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	void draw(sf::RenderWindow* window , int left_x, int top_y) {
		if (false == m_showing) return;
		float rx = (m_x - left_x) * 65.0f + 8;
		float ry = (m_y - top_y) * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		window->draw(m_sprite);
	}

public:
	void SetID(int _id) { Player_ID = _id; }

public:
	int GetID() { return Player_ID; }
};

