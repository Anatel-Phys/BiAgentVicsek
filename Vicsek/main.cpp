#include "Engine.h"
#include <iostream>

//TODO : different weights for the two agents, where the weights are used when computing the mean orientation

//Warning : agents are detecting themselves for now
int main()
{
	float R1 = 20.f;
	float R2 = 20.f;

	Stat stat_1(60.f, 20.f, 0.5f, sf::Color::Red);
	Stat stat_2(120.f, 20.f, 0.5f, sf::Color::Green);

	Engine engine(1000.f, 0.02f, 500, 500, stat_1, stat_2);

	engine.log_polarization("polarization.txt");
	while (engine.get_elapsed_time() < 10.f && engine.is_running())
	{
		engine.run_for(0.5f);
		engine.log_polarization("polarization.txt");
	}

	return 0;
}