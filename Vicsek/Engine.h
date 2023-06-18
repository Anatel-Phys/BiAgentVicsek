#pragma once
#include <vector>
#include <array>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <random>
#include "Interface.h"

typedef sf::Vector2f v2f;

float dist(v2f v, v2f w);

struct Stat
{
	float speed;
	sf::Texture agent_tex;
	sf::Sprite shape;
	float detect_range;
	float noise;
	//other parameters if needed

	Stat();
	//size in pixels
	Stat(float _speed, float detect_range, float noise, std::string texture_path, int size = 9);
	Stat(std::string texture_path, int size = 9);
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
	sf::RenderWindow* gui_w;
	void draw_agent_1(Agent* a);
	void draw_agent_2(Agent* a);
	sf::Event ev;
	Interface* gui;
	sf::Font gui_font;
	TextureMap* button_textures;

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
	float weight_11;
	float weight_21; //for agents 2, what is the weight of agents of type 1
	float weight_12;
	float weight_22;
	float weight_self_1;
	float weight_self_2;

	//helpers
	float cellSize;
	int gridWidth; //number of horizontal cells which is also the number of vertical cells as cells are squares
	void place_in_cell_1(Agent* a);
	void place_in_cell_2(Agent* a);
	void reset_masters();
	void init_cells();
	void init_agents();
	void init_interface();
	void reset_checked_agents_1();
	void reset_checked_agents_2();
	void reset_checked_cells();
	void update_total_time();

	//run
	//void update_orientation_...
	void(Engine::*current_orientation_update)();
	void update_pos();
	void update_cells();
	void draw_sim();

public:
	~Engine();
	Engine(float L, float dt, int N1, int N2, Stat& stat_a1, Stat& stat_a2, size_t framerate = 165);
	void set_agent_stat(Stat& a, size_t idx);
	bool is_running();
	void run_and_display(); //TO MODIFY : when changing orientation update
	void run_for(float time);
	void run();

	//orientation updates. Public rn, will be handled through an enum class later
	void update_orientation(); //TO MODIFY
	void update_orientation_dodge();
	void update_orientation_weights();

	//data collector 
	//Every data collector should be called from out of the engine, int the main loop. 
	//Every "log" data collector logs the time of recording along with the data.
	//Every "log" data collector should open files in append mode so data storage is controlled by file management.
	//Every "compute" data collectors should return a value without affecting any file
	void log_mean_orientation(std::string file);
	//careful! if R is larger than the largest radius of interaction between agents (used for the cells size), the algorithm is incorrect
	void log_cluster_size(float R, std::string file);
	//measure of 
	float compute_cluster_segregity(float R);
	//measure of how agents of one type tend to have only neighbours of the same type. Result between 0 and 1. 1 when every agent 1 has only neighbour 1 and same for 2.
	float compute_neighbour_segregity();
	float compute_polarization_1();
	float compute_polarization_2();
	float compute_polarization_tot();
	float compute_clustering(float R);

	void log_polarization(std::string file);

	//TO MODIFY : other data collectors

	//setters and getters
	void reset_total_time();
	float get_elapsed_time();
	void set_orientation_update(void(Engine::* new_update)());
	void set_N1(int N);
	void set_N2(int N);
	void set_speed_1(float speed);
	void set_speed_2(float speed);
	void set_d_range_1(float d_range);
	void set_d_range_2(float d_range);
	void set_noise_1(float noise);
	void set_noise_2(float noise);
	void set_weight_11(float w);
	void set_weight_12(float w);
	void set_weight_21(float w);
	void set_weight_22(float w);
	void set_weight_self_1(float w);
	void set_weight_self_2(float w);
	void set_dt(float dt);
	void reset();

	//Interface functions
	void test();
	void set_speed1_from_button(std::vector<float>& data);
	void set_speed2_from_button(std::vector<float>& data);
	void set_N1_from_button_and_reset(std::vector<int>& data);
	void set_N2_from_button_and_reset(std::vector<int>& data);
	void set_noise1_from_button(std::vector<float>& data);
	void set_noise2_from_button(std::vector<float>& data);
	void set_d_range_1_from_button(std::vector<float>& data);
	void set_d_range_2_from_button(std::vector<float>& data);
	void set_weight_11_from_button(std::vector<float>& data);
	void set_weight_12_from_button(std::vector<float>& data);
	void set_weight_21_from_button(std::vector<float>& data);
	void set_weight_22_from_button(std::vector<float>& data);
	void set_weight_self_1_from_button(std::vector<float>& data);
	void set_weight_self_2_from_button(std::vector<float>& data);
	void set_orientation_dodge_from_button();
	void set_orientation_weight_from_button();
	void set_orientation_normal_from_button();

	//temp
	void close_window(); //needed when doing simulations without window. Else, window opens and freezes. Later, there will be a open and close method for the window, and the "run" func will check if window is open or not, and react accordingly
};