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

	float speed_1 = 30.f;
	float d_range_1 = 20.f;
	float noise_1 = 0.1f;

	float speed_2 = 120.f;
	float d_range_2 = 20.f;
	float noise_2 = 0.f;

	Stat stat_1(speed_1, d_range_1, noise_1, "agent1tex.png");
	Stat stat_2(speed_2, d_range_2, noise_2, "agent2tex.png");

	float window_size = 500.f;
	float dt = 0.02f;
	int N1 = 500;
	int N2 = 0;
	Engine engine(window_size, dt, N1, N2, stat_1, stat_2);
	engine.close_window();
	
	float min_amp = 0.f;
	float max_amp = 3.14;
	int min_N = 50;
	int max_N = 500;
	int number_of_steps = 101;
	float density;

	float amp_step = (max_amp - min_amp) / (number_of_steps - 1);
	float N_step = (max_N - min_N) / (number_of_steps - 1);

	float cur_amp = min_amp;
	float cur_N = min_N;

	float polarization_mean;
	int n_means = 10;

	std::ofstream f("polarization_1_agent.txt", std::ios::trunc);
	for (int i = 0; i < number_of_steps; i++)
	{
		cur_N = min_N;
		engine.set_noise_1(cur_amp);
		for (int j = 0; j < number_of_steps; j++)
		{
			//density = cur_N / (window_size * window_size);
			polarization_mean = 0.f;

			engine.set_N1(cur_N);
			engine.reset();

			//Pour les prochaines simus, laisser tourner plus longtemps 
			//ATTENTION CHANGER LE SET NOISE POUR CHANGER LE RANDOM ENGINE GENERATOR
			engine.run_for(30.f);
			for (int m = 0; m < n_means; m++)
			{
				engine.run_for(1.f);
				polarization_mean += engine.compute_polarization_1();
			}
			polarization_mean /= n_means;

			f << polarization_mean;
			if (j < number_of_steps - 1)
				f << "\t";

			cur_N += N_step;
		}
		f << std::endl;
		cur_amp += amp_step;
		std::cout << i + 1 << "/" << number_of_steps << std::endl;
	}

	return 0;
}