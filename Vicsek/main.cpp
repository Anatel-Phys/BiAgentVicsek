#include "Engine.h"
#include <iostream>

//TODO : different weights for the two agents, where the weights are used when computing the mean orientation

//Warning : agents are detecting themselves for now
void reset_file(std::string file)
{
	std::ofstream f(file, std::ios::trunc);
}

int main()
{
	float R1 = 20.f;
	float R2 = 20.f;

	reset_file("polarization.txt");

	float speed_1 = 60.f;
	float d_range_1 = 50.f;
	float noise_1 = 0.1f;

	float speed_2 = 120.f;
	float d_range_2 = 20.f;
	float noise_2 = 0.f;

	Stat stat_1(speed_1, d_range_1, noise_1, "agent1tex.png");
	Stat stat_2(speed_2, d_range_2, noise_2, "agent2tex.png");

	float window_size = 1000.f;
	float dt = 0.02f;
	int N1 = 500;
	int N2 = 500;
	Engine engine(window_size, dt, N1, N2, stat_1, stat_2);

	engine.log_polarization("polarization.txt");
	while (engine.is_running())
	{
		engine.run_for(0.5f);
		engine.log_polarization("polarization.txt");
	}

	return 0;
}