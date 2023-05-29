#include "Interface.h"

Interface::Interface(float _x, float _y, float _xSize, float _ySize) : Displayable(_x, _y, _xSize, _ySize)
{
	this->buttonTiles = nullptr;
	this->pLastListCreated = nullptr;
	this->currentDrawingOrder = new std::vector<Displayable*>;
	this->currentActiveDisplayable = nullptr;
	this->currentSelectedDisplayable = nullptr;
}

void Interface::set_font(sf::Font* _font)
{
	this->font = _font;
}

List* Interface::get_last_list_created()
{
	return this->pLastListCreated;
}

Interface* Interface::get_last_interface_created()
{
	return this->pLastInterfaceCreated;
}

Displayable* Interface::get_last_displayable_created()
{
	return this->pLastDisplayableCreated;
}

void Interface::on_user_click(sf::RenderWindow* window)
{
	float x = sf::Mouse::getPosition(*window).x; //sous-optimal car 2 appels a getPosition
	float y = sf::Mouse::getPosition(*window).y;

	if (this->currentSelectedDisplayable != nullptr) {
		this->currentSelectedDisplayable->unselect();
		this->currentSelectedDisplayable = nullptr;
	}

	for (int i = this->currentDrawingOrder->size() - 1; i >= 0; i--) //iterate from the highest priority displayable to the lowest and stops as soon as one is hovered when click
	{
		if (this->currentDrawingOrder->at(i)->is_active())
		{
			if (this->currentDrawingOrder->at(i)->is_hovered(x, y))
			{
				this->currentDrawingOrder->at(i)->on_user_click(window);
				this->currentActiveDisplayable = this->currentDrawingOrder->at(i);
				this->currentSelectedDisplayable = this->currentDrawingOrder->at(i);
				break;
			}
		}
	}
}

void Interface::on_user_hold(sf::RenderWindow* window)
{
	if (this->currentActiveDisplayable != nullptr)
		this->currentActiveDisplayable->on_user_hold(window);
}

void Interface::on_user_key_pressed(sf::Event event) //needs to be thought about a second time, still a test version
{
	for (size_t i = 0; i < currentDrawingOrder->size(); i++)
		this->currentDrawingOrder->at(i)->on_user_key_pressed(event);
}

void Interface::on_user_release()
{
	if (this->currentActiveDisplayable != nullptr)
	{
		this->currentActiveDisplayable->on_user_release();
		this->currentActiveDisplayable = nullptr;
	}
}

void Interface::add_list(float _x, float _y, float _xSize, float _ySize, size_t listLength)
{
	if (_x < 0 || _x + _xSize > this->size.x)
		std::cout << "List out of the interface range" << std::endl;
	List* pList = new List(this->pos.x + _x, this->pos.y + _y, _xSize, _ySize, this->font, listLength);
	pList->set_draw_priority(0);
	this->pLastListCreated = pList;
	this->pLastDisplayableCreated = pList;
	this->currentDrawingOrder->push_back(pList);
}

void Interface::add_interface(float _x, float _y, float _xSize, float _ySize)
{
	if (_x < 0 || _x + _xSize > this->size.x || _y < 0 || _y + _ySize > this->size.y)
		std::cout << "Interface out of the interface range" << std::endl;
	else 
	{
		Interface* pInterface = new Interface(_x, _y, _xSize, _ySize);
		this->currentDrawingOrder->push_back(pInterface);
		this->pLastInterfaceCreated = pInterface;
	}
}

void Interface::add_displayable(Displayable* displayable, bool adjust_on_interface)
{
	if (displayable->get_pos().x + displayable->get_size().x > this->pos.x + this->size.x || displayable->get_pos().x < 0)
		std::cout << "Displayable out of interface range along x axis\n";
	if (displayable->get_pos().y + displayable->get_size().y > this->pos.y + this->size.y || displayable->get_pos().y < 0)
		std::cout << "Displayable out of interface range along y axis\n";

	if (adjust_on_interface) {
		displayable->set_pos(displayable->get_pos().x + this->pos.x, displayable->get_pos().y + this->pos.y);
		displayable->adjust_sprite_position();
	}
	this->currentDrawingOrder->push_back(displayable);
}

void Interface::lock_interface_setup()
{
	size_t nInterfaceElement = this->currentDrawingOrder->size();
	std::vector<ptrWithKey<Displayable>>* pDrawingOrder = new std::vector<ptrWithKey<Displayable>>(nInterfaceElement);
	
	for (size_t i = 0; i < nInterfaceElement; i++)
	{
		pDrawingOrder->at(i) = ptrWithKey<Displayable>(this->currentDrawingOrder->at(i), this->currentDrawingOrder->at(i)->get_draw_priority());
	}
	
	heap_sort_g<Displayable>(pDrawingOrder, nInterfaceElement);
	
	for (size_t i = 0; i < nInterfaceElement; i++)
	{
		this->currentDrawingOrder->at(i) = pDrawingOrder->at(i).obj;
	}

	pDrawingOrder->~vector();
}

void Interface::adjust_sprite_position()
{
	for (size_t i = 0; i < currentDrawingOrder->size(); i++)
		currentDrawingOrder->at(i)->adjust_sprite_position();
}

void Interface::set_texture_map(std::string path, size_t mapWidth, size_t mapHeight, size_t tileWidth, size_t tileHeight)
{
	if (this->buttonTiles != nullptr)
		delete this->buttonTiles;
	this->buttonTiles = new TextureMap(path, mapWidth, mapHeight, tileWidth, tileHeight);
}

TextureMap* Interface::get_texture_map()
{
	return this->buttonTiles;
}

void Interface::draw(sf::RenderWindow* window)
{
	for (auto* displayable : *this->currentDrawingOrder)
	{	
		if (displayable->is_visible())
			displayable->draw(window);
	}
}
