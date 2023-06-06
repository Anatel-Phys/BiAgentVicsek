#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <vector>

struct Vector2s
{
	size_t x;
	size_t y;

	Vector2s(size_t _x, size_t _y)
	{
		this->x = _x;
		this->y = _y;
	}

};

class TextureMap //like a tile map but only taking in charge the texture part. Used to encapsulate a tilemap for sprites of various dimensions.
{
protected:
	sf::Texture* textureSheet;
	size_t width; //number of tiles in the width of the tilemap
	size_t height;

	size_t tileWidth; //dimension (in pixels) of each tile
	size_t tileHeight;

public:
	TextureMap();
	TextureMap(std::string textureName, size_t _width, size_t _height, size_t _tileWidth, size_t _tileHeight);
	//Getters
	size_t get_tile_width();
	size_t get_tile_height();
	size_t get_width();
	size_t get_height();

	virtual sf::Texture* get_texture();
	virtual sf::IntRect get_tex_rect_at(size_t x, size_t y);
};

class TileMap : public TextureMap //class specifically designed for creating maps with tiles. Scaled sprites for each tile as one does not need to create tiles with different scaling
{
private:
	std::vector<sf::Sprite*>* spriteContainer; //not really heavy as all sprite only has a reference to the texture in the GPU and no copy of it
	sf::IntRect* currentTile;

	float tileScale; //don't forget to make it independant from resolution (i.e scale = absolute distance per tile or smth like that)	
	float zoomScale;

public:
	TileMap(std::string textureName, size_t _width, size_t _height, size_t _tileWidth, size_t _tileHeight, float _tileScale);
	void set_tile_pos(size_t xIdx, size_t yIdx, float xPos, float yPos);
	
	sf::Sprite* get_tile1(size_t idx);
	sf::Sprite* get_tile2(size_t xIdx, size_t yIdx);

	//Setters
	void set_zoom_scale(float newZoomScale);
};

