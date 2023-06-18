#pragma once
#include "TileMap.h"
#include "Heapsort.h"

struct Displayable //static displayable
{
	sf::Vector2f pos;
	sf::Vector2f size;

	sf::Sprite* displayedSprite;

	size_t drawPriority; //high priority -> drawn first
	bool active; //reacts to inputs or not
	bool visible; //is displayed to the screen or not
	bool selected;

	Displayable(float _x, float _y, float _xSize, float _ySize)
	{
		this->pos.x = _x;
		this->pos.y = _y;

		this->size.x = _xSize;
		this->size.y = _ySize;

		this->active = true;
		this->visible = true;
		this->drawPriority = 0;
	}

	//void on_position_change(float _x, float _y) = 0;

	/*void set_pos(float _x, float _y)
	{
		this->pos.x = _x;
		this->pos.y = _y;
		this->on_position_change(_x, _y);
	}*/

	bool is_hovered(float x, float y)
	{
		if (x > this->pos.x && x < this->pos.x + this->size.x && y > this->pos.y && y < this->pos.y + this->size.y)
			return true;
		return false;
	}

	void set_draw_priority(size_t _drawPriority)
	{
		this->drawPriority = _drawPriority;
	}

	void set_activity(bool active_flag)
	{
		this->active = active_flag;
	}

	void hide()
	{
		this->visible = false;
	}

	void show()
	{
		this->visible = true;
	}

	void disable()
	{
		this->active = false;
	}

	void enable()
	{
		this->active = true;
	}

	virtual void unselect()
	{
		this->selected = false;
	}

	virtual void select()
	{
		this->selected = true;
	}

	bool is_active()
	{
		return this->active;
	}

	bool is_visible()
	{
		return this->visible;
	}

	void set_pos(float _x, float _y)
	{
		this->pos = { _x, _y };
	}

	sf::Vector2f get_pos()
	{
		return this->pos;
	}

	sf::Vector2f get_size()
	{
		return this->size;
	}

	size_t get_draw_priority()
	{
		return this->drawPriority;
	}

	void set_sprite(sf::Texture* texture, sf::IntRect texRect)		
	{
		if (this->displayedSprite == nullptr)
			this->displayedSprite = new sf::Sprite;
		this->displayedSprite = new sf::Sprite; //maybe there's a bit of suboptimality (i.e leaks) here lol gotta check how the destructor of sprite works
		this->displayedSprite->setTexture(*texture);
		this->displayedSprite->setTextureRect(texRect);
		this->displayedSprite->setPosition(this->pos.x, this->pos.y);
		this->displayedSprite->setScale(this->size.x / texRect.width, this->size.y / texRect.height);
	}

	void bar()
	{
		std::cout << "foo" << std::endl;
	}

	virtual void adjust_sprite_position() = 0;

	virtual void draw(sf::RenderWindow* window) = 0;

	virtual void on_user_click(sf::RenderWindow* window) = 0;

	virtual void on_user_hold(sf::RenderWindow* window) = 0;

	virtual void on_user_release() = 0;

	virtual void on_user_key_pressed(sf::Event event) = 0;
};

struct Panel : public Displayable
{
	Panel(float _x, float _y, float _xSize, float _ySize) : Displayable(_x, _y, _xSize, _ySize) {}

	void draw(sf::RenderWindow* window)
	{
		window->draw(*this->displayedSprite);
	}

	void adjust_sprite_position() override
	{
		this->displayedSprite->setPosition(this->pos.x, this->pos.y);
	}

	void on_user_click(sf::RenderWindow* window) {}

	void on_user_hold(sf::RenderWindow* window) {};

	void on_user_release() {};

	void on_user_key_pressed(sf::Event event) {};
};

struct List : Displayable
{
private:
	std::vector<sf::Text>* texts;
	float elementSpacing;
	size_t nElements;

	float cursorY;
	float maxCursorY;
	float minCursorY;
	float cursorOffset;
	float cursorX;
	float cursorWidth;
	float cursorHeight;
	float cursorYScale;
	bool isCursorSelected;
public:
	sf::RectangleShape* background;
	sf::RectangleShape* cursor;

	List(float _x, float _y, float _xSize, float _ySize, sf::Font* font, size_t listLength) : Displayable(_x, _y, _xSize, _ySize)
	{
		this->background = new sf::RectangleShape(sf::Vector2f(_xSize, _ySize));
		this->background->setPosition(_x, _y);
		this->background->setFillColor(sf::Color::White);
		this->background->setOutlineColor(sf::Color::Black);
		this->background->setOutlineThickness(5.f);

		this->cursorWidth = 15;
		this->cursorHeight = 45;
		this->cursorOffset = 20;
		this->cursorX = this->pos.x + this->size.x - this->cursorWidth * 2;
		this->minCursorY = this->pos.y + this->cursorOffset; 
		this->maxCursorY = this->pos.y + this->size.y - this->cursorHeight - this->cursorOffset;
		this->cursor = new sf::RectangleShape(sf::Vector2f(this->cursorWidth, this->cursorHeight));
		this->cursor->setFillColor(sf::Color::Black);
		this->cursor->setPosition(this->cursorX, this->minCursorY);
		this->cursorY = this->minCursorY;

		this->nElements = listLength;
		this->texts = new std::vector<sf::Text>(listLength);
		for (size_t i = 0; i < listLength; i++)
		{
			this->texts->at(i).setFont(*font);
			this->texts->at(i).setFillColor(sf::Color::Black);
		
			//tests
			this->texts->at(i).setString("Lorem ipsum mdr" + std::to_string(i));
		}

		this->elementSpacing = this->texts->at(0).getCharacterSize();
		this->cursorYScale = (this->elementSpacing * (listLength + 2) - _ySize) / (this->maxCursorY - this->minCursorY);

		for (size_t i = 0; i < listLength; i++)
		{
			if (this->texts->at(i).findCharacterPos(this->texts->at(i).getString().getSize()).x > this->size.x)
			{
				size_t left = 0;
				size_t right = this->texts->at(i).getString().getSize() - 1;
				size_t mid;

				while (right > left + 1)
				{
					mid = (left + right) / 2;
					if (this->texts->at(i).findCharacterPos(mid).x > this->size.x)
						right = mid;
					else
						left = mid;
				}
				size_t length = this->texts->at(i).getString().getSize();
				std::string buffer = this->texts->at(i).getString();
				for (size_t j = 0; j < length - left; j++)
					buffer.pop_back();
				this->texts->at(i).setString(buffer);
			}
		}
		update_text_position();
	}

	void add_element(std::string element)
	{
		sf::Text text;
		text.setFont(*this->texts->at(0).getFont());
		text.setFillColor(sf::Color::Black);
		text.setString(element);
		this->texts->push_back(text); //vector makes a copy so should be fine right ?
		this->maxCursorY += this->elementSpacing;
		this->nElements++;
	}

	void move_cursor(float displacement)
	{
		this->cursorY += displacement;
		if (this->cursorY < this->minCursorY)
			this->cursorY = this->minCursorY;
		if (this->cursorY > this->maxCursorY)
			this->cursorY = this->maxCursorY;
	}

	void set_cursor(float newPos)
	{
		this->cursorY = newPos;
		if (this->cursorY < this->minCursorY)
			this->cursorY = this->minCursorY;
		if (this->cursorY > this->maxCursorY)
			this->cursorY = this->maxCursorY;
	}

	void adjust_sprite_position() override
	{
		background->setPosition(this->pos.x, this->pos.y);
		//other elements are updated automatically I believe. Else, need to change them when pos is updated by adding the list to a non-(0, 0)-positionned interface
	}

	void on_user_click(sf::RenderWindow* window)
	{
		float x = sf::Mouse::getPosition(*window).x;
		float y = sf::Mouse::getPosition(*window).y;

		if (x > this->cursorX && x < this->cursorX + this->cursorWidth && y > this->cursorY && y < this->cursorY + this->cursorHeight)
			this->isCursorSelected = true;
	}

	void on_user_hold(sf::RenderWindow* window)
	{
		if (this->isCursorSelected)
		{
			this->set_cursor(sf::Mouse::getPosition(*window).y - this->cursorHeight / 2);
			this->cursor->setPosition(this->cursorX, this->cursorY);
			update_text_position();
		}
	}

	void update_text_position()
	{
		float textY;
		float textX = this->pos.x + 5.f; //arbitrary offset
		for (size_t i = 0; i < this->texts->size(); i++)
		{
			textY = i * this->elementSpacing - (cursorY - this->pos.y - 10.f) * this->cursorYScale; //arbitrary offset
			this->texts->at(i).setPosition(textX, textY);
		}
	}

	void on_user_release()
	{
		this->isCursorSelected = false;
	}

	void on_user_key_pressed(sf::Event event)
	{

	}

	void draw(sf::RenderWindow* window) override
	{
		window->draw(*this->background);

		for (size_t i = 0; i < this->nElements; i++)
		{
			if (this->texts->at(i).getPosition().y > this->pos.y && this->texts->at(i).getPosition().y < this->pos.y + this->size.y - this->texts->at(i).getCharacterSize())
			{
				window->draw(this->texts->at(i));
			}
		}

		window->draw(*this->cursor);
	}
};

struct Button : Displayable
{
	sf::Text textDisplayer;
	sf::Vector2f textRelPos;

	Button(float _x, float _y, float _xSize, float _ySize) : Displayable(_x, _y, _xSize, _ySize)
	{	
		this->textDisplayer.setCharacterSize(14);
		this->textDisplayer.setFillColor(sf::Color::Black);
		center_text();
	}

	void set_font(sf::Font* font)
	{
		this->textDisplayer.setFont(*font);
	}

	void center_text()
	{
		float textLength = this->textDisplayer.findCharacterPos(this->textDisplayer.getString().getSize()).x - this->textDisplayer.getPosition().x;
		this->textDisplayer.setPosition(this->pos.x + this->size.x / 2 - textLength / 2, this->pos.y + this->size.y / 2 - this->textDisplayer.getCharacterSize() / 2);
	}

	void set_text(std::string buttonText)
	{
		this->textDisplayer.setString(buttonText);
		this->center_text();
	}

	void set_text_color(sf::Color color)
	{
		this->textDisplayer.setFillColor(color);
	}

	void set_character_size(size_t _size)
	{
		this->textDisplayer.setCharacterSize(_size);
		this->center_text();
	}

	virtual void on_button_press() = 0;

	void on_user_key_pressed(sf::Event event)
	{

	}

	void draw(sf::RenderWindow* window) override
	{
		window->draw(*this->displayedSprite);
		window->draw(this->textDisplayer);
	}

	void foo()
	{
		std::cout << "bar" << "\n";
	}
};

template <class T>
struct FuncButton : public Button
{
protected:

	sf::Sprite* pressedSprite;
	sf::Sprite* unpressedSprite;

	T* target;
	std::vector<void(T::*)()>* activationFuncs;
	size_t nActivationFuncs;
public:
	FuncButton(float _x, float _y, float _xSize, float _ySize) : Button(_x, _y, _xSize, _ySize)
	{
		this->pressedSprite = nullptr;
		this->unpressedSprite = nullptr;
		this->activationFuncs = new std::vector<void(T::*)()>;
		this->nActivationFuncs = 0;
	}

	void set_pressed_sprite(sf::Texture* texture, sf::IntRect texRect) //probably that those will be handled by only moving texture rects later ? Probably more optimal to maybe have a vector of texture rects or a texture Sheet with predictable setup
	{
		if (this->pressedSprite != nullptr)
			delete this->pressedSprite;
		this->pressedSprite = new sf::Sprite;
		this->pressedSprite->setTexture(*texture);
		this->pressedSprite->setTextureRect(texRect);
		this->pressedSprite->setPosition(this->pos.x, this->pos.y);
		this->pressedSprite->setScale(this->size.x / texRect.width, this->size.y / texRect.height);
	}

	void set_unpressed_sprite(sf::Texture* texture, sf::IntRect texRect) //probably that those will be handled by only moving texture rects later ? Probably more optimal to maybe have a vector of texture rects or a texture Sheet with predictable setup
	{
		if (this->unpressedSprite != nullptr)
			delete this->unpressedSprite;
		this->unpressedSprite = new sf::Sprite;
		this->unpressedSprite->setTexture(*texture);
		this->unpressedSprite->setTextureRect(texRect);
		this->unpressedSprite->setPosition(this->pos.x, this->pos.y);
		this->unpressedSprite->setScale(this->size.x / texRect.width, this->size.y / texRect.height);
	}

	void adjust_sprite_position()
	{
		this->unpressedSprite->setPosition(this->pos.x, this->pos.y);
		this->pressedSprite->setPosition(this->pos.x, this->pos.y);
		center_text();
	}

	void add_activation_function(void (T::* _activationFunc)())
	{
		this->activationFuncs->push_back(_activationFunc);
		this->nActivationFuncs++;
	}

	void on_button_press()
	{
		for (size_t i = 0; i < this->nActivationFuncs; i++)
		{
			(this->target->*activationFuncs->at(i))();
		}
	}

	void set_target(T* _target)
	{
		this->target = _target;
	}

	void on_user_click(sf::RenderWindow* window)
	{
		this->on_button_press();
		this->displayedSprite = this->pressedSprite;
	}

	void on_user_hold(sf::RenderWindow* window)
	{

	}

	void on_user_release()
	{
		this->displayedSprite = this->unpressedSprite;
	}
};
	
template <class T>
struct FuncSwitch : public Button
{
protected:
	T* target;
	std::vector<void(T::*)()>* activationFuncs;
	size_t nActivationFuncs;
	size_t nextFuncToExecute;
	std::vector<sf::Sprite*>* sprites;
	size_t nextSpriteToDisplay; //should be the same as nextFuncToExecute but adds a little bit of flexibility	
	size_t nSprites;
public:
	FuncSwitch(float _x, float _y, float _xSize, float _ySize) : Button(_x, _y, _xSize, _ySize)
	{
		this->activationFuncs = new std::vector<void(T::*)()>;
		this->sprites = new std::vector<sf::Sprite*>;
		this->nextSpriteToDisplay = 0;
		this->nSprites = 0;
		this->nActivationFuncs = 0;
		this->nextFuncToExecute = 0;
	}

	void add_sprite(sf::Texture* texture, sf::IntRect texRect)
	{
		this->sprites->push_back(new sf::Sprite);
		this->nSprites++;
		this->sprites->at(this->nSprites - 1)->setTexture(*texture);
		this->sprites->at(this->nSprites - 1)->setTextureRect(texRect);
		this->sprites->at(this->nSprites - 1)->setPosition(this->pos.x, this->pos.y);
		this->sprites->at(this->nSprites - 1)->setScale(this->size.x / texRect.width, this->size.y / texRect.height);
	}

	void adjust_sprite_position() override
	{
		for (sf::Sprite* s : *sprites)
		{
			s->setPosition(this->pos.x, this->pos.y);
		}
		center_text();
	}

	void add_activation_function(void (T::* _activationFunc)())
	{
		this->activationFuncs->push_back(_activationFunc);
		this->nActivationFuncs++;
	}

	void on_button_press()
	{
		(this->target->*activationFuncs->at(this->nextFuncToExecute))();
		this->nextFuncToExecute++;
		if (this->nextFuncToExecute >= this->nActivationFuncs)
			this->nextFuncToExecute = 0;
	}

	void set_target(T* _target)
	{
		this->target = _target;
	}

	void on_user_click(sf::RenderWindow* window)
	{
		this->on_button_press();
		this->nextSpriteToDisplay++;
		if (this->nextSpriteToDisplay >= this->nSprites)
			this->nextSpriteToDisplay = 0;
		this->displayedSprite = this->sprites->at(this->nextSpriteToDisplay);
	}

	void on_user_hold(sf::RenderWindow* window)
	{

	}

	void on_user_release()
	{

	}

	void draw(sf::RenderWindow* window) override
	{
		window->draw(*this->sprites->at(this->nextSpriteToDisplay));
		window->draw(this->textDisplayer);
	}
};

template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
struct DataPoint //will encapsulate the functionnalities like having an internal vector of values, changing it and accessing it. Writable will interact directly with datapoints instead of each data-driven displayable separately
{
	std::vector<T>* pParameters;
	size_t nParameters;

	DataPoint(size_t _nParameters)
	{
		this->pParameters = new std::vector<T>(_nParameters);
		this->nParameters = _nParameters;
	}

	void set_value(size_t valIdx, T value)
	{
		this->pParameters->at(valIdx) = value;
	}

	T get_value(size_t valIdx)
	{
		return this->pParameters->at(valIdx);
	}
};

//If you need to access your button later to change its values, keep a pointer to it. If you need to automatically update values with a writable, feature is a WIP
template <class T, typename DataType, typename = typename std::enable_if<std::is_arithmetic<DataType>::value, DataType>::type>
struct ValueButton : public Button, public DataPoint<DataType>
{
protected:
	sf::Sprite* pressedSprite;
	sf::Sprite* unpressedSprite;

	T* target;
	std::vector<void(T::*)(std::vector<DataType>&)>* activationFuncs;
	size_t nActivationFuncs;

public:	
	ValueButton(float _x, float _y, float _xSize, float _ySize, size_t nParameters) : Button(_x, _y, _xSize, _ySize), DataPoint<DataType>(nParameters)
	{
		this->pressedSprite = nullptr;
		this->unpressedSprite = nullptr;
		this->activationFuncs = new std::vector<void(T::*)(std::vector<DataType>&)>;
		this->nActivationFuncs = 0;
	}

	void set_pressed_sprite(sf::Texture* texture, sf::IntRect texRect) //probably that those will be handled by only moving texture rects later ? Probably more optimal to maybe have a vector of texture rects or a texture Sheet with predictable setup
	{
		if (this->pressedSprite != nullptr)
			delete this->pressedSprite;
		this->pressedSprite = new sf::Sprite;
		this->pressedSprite->setTexture(*texture);
		this->pressedSprite->setTextureRect(texRect);
		this->pressedSprite->setPosition(this->pos.x, this->pos.y);
		this->pressedSprite->setScale(this->size.x / texRect.width, this->size.y / texRect.height);
	}

	void set_unpressed_sprite(sf::Texture* texture, sf::IntRect texRect) //probably that those will be handled by only moving texture rects later ? Probably more optimal to maybe have a vector of texture rects or a texture Sheet with predictable setup
	{
		if (this->unpressedSprite != nullptr)
			delete this->unpressedSprite;
		this->unpressedSprite = new sf::Sprite;
		this->unpressedSprite->setTexture(*texture);
		this->unpressedSprite->setTextureRect(texRect);
		this->unpressedSprite->setPosition(this->pos.x, this->pos.y);
		this->unpressedSprite->setScale(this->size.x / texRect.width, this->size.y / texRect.height);

		this->displayedSprite = this->unpressedSprite;
	}

	void adjust_sprite_position() override
	{
		this->pressedSprite->setPosition(this->pos.x, this->pos.y);
		this->unpressedSprite->setPosition(this->pos.x, this->pos.y);
		center_text();
	}

	void add_activation_function(void (T::* _activationFunc)(std::vector<DataType>&))
	{
		this->activationFuncs->push_back(_activationFunc);
		this->nActivationFuncs++;
	}

	void on_button_press()
	{
		for (size_t i = 0; i < this->nActivationFuncs; i++)
		{
			(this->target->*activationFuncs->at(i))(*this->pParameters);
		}
	}

	void set_target(T* _target)
	{
		this->target = _target;
	}

	void on_user_click(sf::RenderWindow* window)
	{
		this->on_button_press();
		this->displayedSprite = this->pressedSprite;
	}

	void on_user_hold(sf::RenderWindow* window)
	{

	}

	void on_user_release()
	{
		this->displayedSprite = this->unpressedSprite;
	}
};

struct Writable : public Displayable
{
	sf::String content;
	sf::Text text;

	//
	sf::RectangleShape* outline;

	Writable(float _x, float _y, float _xSize, float _ySize) : Displayable(_x, _y, _xSize, _ySize)
	{
		this->content = "";
		this->selected = false;
		this->text.setFillColor(sf::Color::Black);
		this->text.setPosition(_x, _y);

		this->outline = new sf::RectangleShape(sf::Vector2f(_xSize, _ySize));
		this->outline->setPosition(_x, _y);
		this->outline->setOutlineColor(sf::Color::Black);
		this->outline->setOutlineThickness(-2.f);
		this->outline->setFillColor(sf::Color::White);
	}

	void adjust_sprite_position() override
	{
		this->outline->setPosition(this->pos.x, this->pos.y);
		this->text.setPosition(this->pos.x, this->pos.y);
	}

	void set_font(sf::Font* font)
	{
		this->text.setFont(*font);
	}

	void unselect() override
	{
		this->selected = false;
		this->outline->setOutlineColor(sf::Color::Black);
	}

	void select() override
	{
		this->selected = true;
		this->outline->setOutlineColor(sf::Color::Red);
	}

	void on_user_click(sf::RenderWindow* window)
	{
		this->select();
	}

	void on_user_hold(sf::RenderWindow* window)
	{

	}

	void on_user_release()
	{

	}

	void on_user_key_pressed(sf::Event event)
	{
		if (this->selected){
			if (event.text.unicode == 13) { //enter, maybe a "on_enter method could be of use"	
				this->unselect();
			}
			else if (event.text.unicode == 8) { //backspace
				if (this->content.getSize() > 0)
				{
					this->content.erase(this->content.getSize() - 1, 1);
				}
			}
			else if (event.text.unicode == 10) { //line jump

			}
			else {
				if (this->text.findCharacterPos(this->text.getString().getSize() - 1).x + this->text.getCharacterSize() < this->pos.x + this->size.x)
					this->content += event.text.unicode;
			}
			this->text.setString(this->content);
		}
	}

	std::string get_string()
	{
		return this->content.toAnsiString();
	}

	void draw(sf::RenderWindow* window) override
	{
		window->draw(*this->outline);
		window->draw(this->text);
	}

	void set_character_size(size_t newCharacterSize)
	{
		this->text.setCharacterSize(newCharacterSize);
	}

	//basically updates on click and on keyboard press. If this is selected (will be highlighted when clicked and interface will keep a pointer to the current writable)
	//every keyboard input that is suitable for the zone (i.e numbers in some case, letters in others) will be written on the writable until enter is pressed
};

template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
struct ValueWritable : public Writable
{
protected:
	DataPoint<T>* target = nullptr; //maybe it will have several targets later
	size_t targetDataSlot = -1;

public:
	ValueWritable(float _x, float _y, float _xSize, float _ySize) : Writable(_x, _y, _xSize, _ySize) {}

	void set_target(DataPoint<T>* _target, size_t _targetDataSlot)
	{
		this->target = _target;
		this->targetDataSlot = _targetDataSlot;
	}

	void on_user_key_pressed(sf::Event event) override
	{
		if (this->selected) {
			if (event.text.unicode == 13) { //enter, maybe a "on_enter_press() method could be of use"	
				this->unselect();
			}
			else if (event.text.unicode == 8) { //backspace
				if (this->content.getSize() > 0)
				{
					this->content.erase(this->content.getSize() - 1, 1);
				}
			}
			else if (event.text.unicode <= 57 && event.text.unicode >= 48 || event.text.unicode == 46 && !std::is_same<T, int>::value){ //the dot is not valid if type is int. Currently doesn't work with other integer types (long int etc) -> TO FIX{
				if (this->text.findCharacterPos(this->text.getString().getSize() - 1).x + this->text.getCharacterSize() < this->pos.x + this->size.x)
					this->content += event.text.unicode;
			}
			this->text.setString(this->content);

			if (this->target != nullptr && this->content.getSize() > 0)
				this->target->set_value(this->targetDataSlot, static_cast<T>(std::stof(this->get_string()))); //throws exception for several things, like when you erase everything
		}
	}

	//will throw an error if string entered is not a number
	void set_content(std::string _content)
	{
		this->text.setString(_content);
		this->content = _content;
		if (this->target != nullptr && this->content.getSize() > 0)
			this->target->set_value(this->targetDataSlot, static_cast<T>(std::stof(this->get_string())));
	}

	void set_content(double _content)
	{
		this->text.setString(std::to_string(_content));
		this->content = std::to_string(_content);
		this->target->set_value(this->targetDataSlot, _content);
	}
};
 
struct Text : Displayable
{
	sf::Text textDisplayer;

	Text(float _x, float _y, float _xSize, float _ySize) : Displayable(_x, _y, _xSize, _ySize)
	{
		this->textDisplayer.setPosition(_x, _y);
		set_text_color(sf::Color::Black);
	}

	void adjust_sprite_position() override
	{
		this->textDisplayer.setPosition(this->pos.x, this->pos.y);
	}

	void set_auto_character_size(std::string maxString) //sets the character size accordingly with the size of the text zone
	{
		set_string(maxString);
		set_character_size(10);
		float mul = 1.f;
		float maxStringLengthX = (this->textDisplayer.findCharacterPos(maxString.size()).x - this->pos.x) / this->size.x;
		float maxStringLengthY = 10.f / this->size.y;
		if (maxStringLengthX > maxStringLengthY)
			mul = maxStringLengthX;
		else
			mul = maxStringLengthY;

		set_character_size(10.f / mul);
		set_string("");
	}

	void set_font(sf::Font* font)
	{
		this->textDisplayer.setFont(*font);
	}

	void set_string(std::string newText)
	{
		this->textDisplayer.setString(newText);
	}

	void set_text_color(sf::Color newColor)
	{
		this->textDisplayer.setFillColor(newColor);
	}

	void set_character_size(size_t newCharacterSize)
	{
		this->textDisplayer.setCharacterSize(newCharacterSize);
	}

	void set_style(sf::Text::Style newStyle)
	{
		this->textDisplayer.setStyle(newStyle);
	}

	void draw(sf::RenderWindow* window)
	{
		window->draw(this->textDisplayer);
	}

	size_t get_character_size()
	{
		return textDisplayer.getCharacterSize();
	}

	void on_user_click(sf::RenderWindow* window) {}
	void on_user_hold(sf::RenderWindow* window) {}
	void on_user_key_pressed(sf::Event event) {}
	void on_user_release() {};
};

struct Interface : Displayable
{
private:
	std::vector<Displayable*>* currentDrawingOrder;
	Displayable* currentActiveDisplayable; //the displayable that is held
	Displayable* currentSelectedDisplayable; //the displayable that has been clicked and keeps effect until the user clicks elsewhere

	List* pLastListCreated;
	Interface* pLastInterfaceCreated;
	Displayable* pLastDisplayableCreated;

	sf::Font* font;
	TextureMap* buttonTiles; //will be different from map as buttons might be of different sizes -> need to be multiple sprite objects, so can't store them in tileMap, or at least it probably underperforms;


public:
	Interface(float _x, float _y, float _xSize, float _ySize);
	void set_font(sf::Font* _font);
	List* get_last_list_created();
	Interface* get_last_interface_created();
	Displayable* get_last_displayable_created();

	void on_user_click(sf::RenderWindow* window);
	void on_user_hold(sf::RenderWindow* window);
	void on_user_key_pressed(sf::Event event);
	void on_user_release();

	template<class Target>	
	FuncButton<Target>* add_button(float _x, float _y, float _xSize, float _ySize)
	{
		if (_x < 0 || _x + _xSize > this->size.x) {
			std::cout << "Button out of the interface range" << std::endl;
			return nullptr;
		}
		else {
			FuncButton<Target>* pButton = new FuncButton<Target>(this->pos.x + _x, this->pos.y + _y, _xSize, _ySize);
			pButton->set_sprite(this->buttonTiles->get_texture(), this->buttonTiles->get_tex_rect_at(2, 1));
			pButton->set_pressed_sprite(this->buttonTiles->get_texture(), this->buttonTiles->get_tex_rect_at(1, 1));
			pButton->set_unpressed_sprite(this->buttonTiles->get_texture(), this->buttonTiles->get_tex_rect_at(2, 1));
			pButton->set_font(this->font);

			this->currentDrawingOrder->push_back(pButton);
			return pButton;
		}
	}

	template<class Target>
	FuncSwitch<Target>* add_switch(float _x, float _y, float _xSize, float _ySize)
	{
		if (_x < 0 || _x + _xSize > this->size.x) {
			std::cout << "Switch out of the interface range" << std::endl;
			return nullptr;
		}
		else {
			FuncSwitch<Target>* pSwitch = new FuncSwitch<Target>(this->pos.x + _x, this->pos.y + _y, _xSize, _ySize);
			pSwitch->set_sprite(this->buttonTiles->get_texture(), this->buttonTiles->get_tex_rect_at(2, 1));
			pSwitch->add_sprite(this->buttonTiles->get_texture(), this->buttonTiles->get_tex_rect_at(2, 1));
			pSwitch->add_sprite(this->buttonTiles->get_texture(), this->buttonTiles->get_tex_rect_at(1, 1));
			pSwitch->set_font(this->font);

			this->currentDrawingOrder->push_back(pSwitch);
			return pSwitch;
		}
	}
	void add_list(float _x, float _y, float _xSize, float _ySize, size_t listLength);
	void add_interface(float _x, float _y, float _xSize, float _ySize);
	//by default, adjusts displayables position to this interface position (i.e sets their position to be relative to the interface). Add false as an optionnal argument to disable
	void add_displayable(Displayable* displayable, bool adjuste_on_interface = true);
	void lock_interface_setup();
	void adjust_sprite_position() override;
	void set_texture_map(std::string path, size_t _width, size_t _height, size_t _tileWidth, size_t _tileHeight);
	TextureMap* get_texture_map();

	void draw(sf::RenderWindow* window) override;

};
