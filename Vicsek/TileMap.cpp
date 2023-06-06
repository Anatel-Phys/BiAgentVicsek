#include "TileMap.h"
//#define TEXTURE_PATH "Textures/Spritesheets/"
static const std::string TEXTURE_PATH = "";

TileMap::TileMap(std::string textureName, size_t _width, size_t _height, size_t _tileWidth, size_t _tileHeight, float _tileScale) : TextureMap(textureName, _width, _height, _tileWidth, _tileHeight)
{
	this->spriteContainer = new std::vector<sf::Sprite*>(_width * _height);
	
	this->tileScale = _tileScale;

	if (!this->textureSheet->loadFromFile(TEXTURE_PATH + textureName))
		std::cout << "Couldn't load texture " << textureName << std::endl;
	else
	{
		sf::IntRect bufferRect = {0, 0, (int)_tileWidth, (int)_tileHeight};
		size_t top;
		size_t left;
		for (size_t i = 0; i < _width * _height; i++) 
		{
			this->spriteContainer->at(i) = new sf::Sprite;
			this->spriteContainer->at(i)->setTexture(*this->textureSheet);
			this->spriteContainer->at(i)->setScale(_tileScale, _tileScale);
			top = (i / _width) * _tileHeight;
			left = (i % _width) * _tileWidth;
			bufferRect.top = top;
			bufferRect.left = left;
			this->spriteContainer->at(i)->setTextureRect(bufferRect);
		}
	}
		

}

void TileMap::set_tile_pos(size_t xIdx, size_t yIdx, float xPos, float yPos)
{
	this->spriteContainer->at(xIdx + (yIdx - 1) * this->width - 1)->setPosition(xPos, yPos);
}

sf::Sprite* TileMap::get_tile1(size_t idx)
{
	return this->spriteContainer->at(idx);
}

sf::Sprite* TileMap::get_tile2(size_t xIdx, size_t yIdx)
{
	return this->spriteContainer->at(xIdx + (yIdx - 1) * this->width - 1);
}

void TileMap::set_zoom_scale(float newZoomScale)
{
	this->zoomScale = newZoomScale;
	for (size_t i = 0; i < this->width * this->height; i++)
		this->spriteContainer->at(i)->setScale(this->tileScale * newZoomScale, this->tileScale * newZoomScale);
}

/*----------------*/
/*  TEXTURE MAP   */
/*----------------*/
TextureMap::TextureMap()
{
	this->textureSheet = nullptr;
}

TextureMap::TextureMap(std::string textureName, size_t _width, size_t _height, size_t _tileWidth, size_t _tileHeight)
{
	this->tileWidth = _tileWidth;
	this->tileHeight = _tileHeight;
	this->width = _width;
	this->height = _height;
	this->textureSheet = new sf::Texture;
	if (!this->textureSheet->loadFromFile(TEXTURE_PATH + textureName))
		std::cout << "Couldn't load texture " << textureName << std::endl;
}

size_t TextureMap::get_tile_width()
{
	return this->tileWidth;
}

size_t TextureMap::get_tile_height()
{
	return this->tileHeight;
}

size_t TextureMap::get_width()
{
	return this->width;
}

size_t TextureMap::get_height()
{
	return this->height;
}

sf::Texture* TextureMap::get_texture()
{
	return this->textureSheet;
}

sf::IntRect TextureMap::get_tex_rect_at(size_t x, size_t y)
{
	return sf::IntRect((x - 1) * this->tileWidth, (y - 1) * this->tileHeight, this->tileWidth, this->tileHeight);
}
