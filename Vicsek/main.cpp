#include "Engine.h"
#include <iostream>

//TODO : maybe reuse the same clustering code bcs it is long and used in 4 functions => would gain a significant amount of line space (group it in smth like compute_clusters that could also be used to log data in custom ways)
//		 have only one "run" and one "run_for" function and choose the right run/run_with_display/run_with_display_and_interface depending on if interface/window is open =>run_for is more optimized because no "if" everywhere even tho if would always have the same result
//Idea : long-range weak components where agents tend to go towards each other, dominated by alignment interaction at short range if enough neighbors

//To test : segregity independant of N as long as the N's are the same ? Conclusion : for two different agents, symetric in interchanging N1 and N2

//Warning : agents are detecting themselves for now
void reset_file(std::string file)
{
	std::ofstream f(file, std::ios::trunc);
}

//main for segregation data
int main()
{
	std::string filename = "segregity_more_detailed.txt";
	reset_file(filename);

	float d_range = 20.f;
	Stat stat_1(0.f, d_range, 0.f, "agent1tex.png"); //d_range is required for engine constructor/cell construction. Need to make change in init_cells method so it just generates one big cell if d_range = 0 because if agents weren't initialized, the user means to reset the engine in a parameter-changing loop anyway
	Stat stat_2(0.f, d_range, 0.f, "agent2tex.png");

	float window_size = 500.f;
	float dt = 0.02f;
	int N = 500;
	Engine engine(window_size, dt, N, N, stat_1, stat_2);
	
	//calculations for two agents where one is slighly faster and more noisy
	//FOR THIS SIM : val1 is noise of agent 1, val2 is speed of agent 1. When making colormap, val1 is y, val2 is x
	//When changing to which engine param the values corresponds, change the "set_..." statements in the nested for loops
	float min_val1 = 0.01f;
	float max_val1 = 0.51f;
	float cur_val1 = min_val1;

	float min_val2 = 20.f;
	float max_val2 = 60.f;
	float cur_val2 = min_val2;

	int number_of_steps = 51;
	float val1_step = (max_val1 - min_val1) / (number_of_steps - 1);
	float val2_step = (max_val2 - min_val2) / (number_of_steps - 1);

	//FOR THIS SIM : computed val is segregity
	float computed_val_mean, buf_mean;
	int n_same_sim_means = 5;
	int n_diff_sim_means = 3;
	float same_sim_mean_run_time = 1.f; //time between two measurements on the same simulation
	float run_time = 25.f;

	engine.set_d_range_1(d_range);
	engine.set_d_range_2(d_range);
	engine.set_noise_2(min_val1);
	engine.set_speed_2(min_val2);
	engine.set_orientation_update(&Engine::update_orientation);
	engine.close_window();

	std::ofstream f(filename, std::ios::trunc);
	for (int i = 0; i < number_of_steps; i++)
	{
		cur_val2 = min_val2;
		engine.set_noise_1(cur_val1);

		for (int j = 0; j < number_of_steps; j++)
		{
			engine.set_speed_1(cur_val2);
			computed_val_mean = 0.f;

			for (int n = 0; n < n_diff_sim_means; n++)
			{
				engine.reset();
				engine.run_for(run_time);
				buf_mean = 0.f;
				for (int m = 0; m < n_same_sim_means; m++)
				{
					engine.run_for(same_sim_mean_run_time);
					buf_mean += engine.compute_segregation_param(d_range);
				}
				
				computed_val_mean += buf_mean / n_same_sim_means;
			}
			computed_val_mean /= n_diff_sim_means;

			f << computed_val_mean;
			if (j < number_of_steps - 1)
				f << "\t";

			cur_val2 += val2_step;
		}
		f << std::endl;
		cur_val1 += val1_step;
		std::cout << i + 1 << "/" << number_of_steps << std::endl;
	}

	return 0;
}


//main for polarization data
//int main()
//{
//	reset_file("polarization.txt");
//
//	float speed_1 = 30.f;
//	float d_range_1 = 20.f;
//	float noise_1 = 0.1f;
//
//	float speed_2 = 120.f;
//	float d_range_2 = 20.f;
//	float noise_2 = 0.f;
//
//	Stat stat_1(speed_1, d_range_1, noise_1, "agent1tex.png");
//	Stat stat_2(speed_2, d_range_2, noise_2, "agent2tex.png");
//
//	float window_size = 500.f;
//	float dt = 0.02f;
//	int N1 = 500;
//	int N2 = 0;
//	Engine engine(window_size, dt, N1, N2, stat_1, stat_2);
//	engine.close_window();
//	
//	float min_amp = 0.f;
//	float max_amp = 1;
//	int min_N = 50;
//	int max_N = 500;
//	int number_of_steps = 101;
//	float density;
//
//	float noise_step = (max_amp - min_amp) / (number_of_steps - 1);
//	float N_step = (max_N - min_N) / (number_of_steps - 1);
//
//	float cur_noise = min_amp;
//	float cur_N = min_N;
//
//	float polarization_mean;
//	int n_means = 10;
//	float run_time = 40.f;
//
//	std::ofstream f("polarization_1_agent.txt", std::ios::trunc);
//	for (int i = 0; i < number_of_steps; i++)
//	{
//		cur_N = min_N;
//		engine.set_noise_1(cur_noise);
//		for (int j = 0; j < number_of_steps; j++)
//		{
//			//density = cur_N / (window_size * window_size);
//			polarization_mean = 0.f;
//
//			engine.set_N1(cur_N);
//			engine.reset();
//
//			//Pour les prochaines simus, laisser tourner plus longtemps 
//			engine.run_for(40.f);
//			for (int m = 0; m < n_means; m++)
//			{
//				engine.run_for(1.f);
//				polarization_mean += engine.compute_polarization_1();
//			}
//			polarization_mean /= n_means;
//
//			f << polarization_mean;
//			if (j < number_of_steps - 1)
//				f << "\t";
//
//			cur_N += N_step;
//		}
//		f << std::endl;
//		cur_noise += noise_step;
//		std::cout << i + 1 << "/" << number_of_steps << std::endl;
//	}
//
//	return 0;
//}