#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <random>

typedef sf::Vector2f v2f;

float dist(v2f v, v2f w);

struct Stat
{
	float speed;
	sf::CircleShape shape;
	float detect_range;
	float noise;
	//other parameters if needed

	Stat();
	Stat(float _speed, float detect_range, float noise, sf::Color color, float _radius = 3.f);
};

struct Agent;

struct Cell
{
	Agent* master1;
	Agent* master2;
	int idx;
	std::array<int, 8> neighbors;
	bool checked = false; //helper for some data measurement algorithm

	void insert_agent_1(Agent* a);
	void insert_agent_2(Agent* a);
};

struct Agent
{
	v2f pos;
	Agent* next;
	Cell* curCell;
	int idx;
	float orientation;
	bool checked; //helper for some data measurement algorithm

	Agent();
	Agent(v2f pos, float orientation, int idx);
};

class Engine
{
private:
	//rendering
	sf::RenderWindow* w;
	void draw_agent_1(v2f pos);
	void draw_agent_2(v2f pos);
	sf::Event ev;
 
	//world
	float L;
	float dt;
	float totalTime;
	size_t N1, N2;
	std::vector<Agent*>* agents_1;
	std::vector<Agent*>* agents_2;
	std::vector<Cell*>* cells;
	std::default_random_engine rnd_engine;
	std::uniform_real_distribution<float>* noise_gen_1; //noise generator for agent type 1
	std::uniform_real_distribution<float>* noise_gen_2; //noise generator for agent type 2

	//sim parameters
	Stat stat_a1;
	Stat stat_a2;

	//helpers
	float cellSize;
	int gridWidth; //number of horizontal cells which is also the number of vertical cells as cells are squares
	void place_in_cell_1(Agent* a);
	void place_in_cell_2(Agent* a);
	void reset_masters();
	void init_cells();
	void init_agents();
	void reset_checked_agents_1();
	void reset_checked_agents_2();
	void reset_checked_cells();
	void update_total_time();

	//run
	void update_orientation(); //TO MODIFY
	void update_orientation_dodge();
	//void update_orientation_...
	void update_pos();
	void update_cells();
	void draw_sim();

public:
	~Engine();
	Engine(float L, float dt, int N1, int N2, Stat& stat_a1, Stat& stat_a2, size_t framerate = 165);
	void set_agent_stat(Stat& a, size_t idx);
	bool is_running();
	void run(); //TO MODIFY : when changing orientation update
	float get_elapsed_time();
	void reset();

	//data collector
	void log_mean_orientation(std::string file);
	//careful! if R is larger than the largest radius of interaction between agents (used for the cells size), the algorithm is incorrect
	void log_cluster_size(float R, std::string file);

	//TO MODIFY : other data collectors

	//setters and getters
	void reset_total_time();
};