#include "Engine.h"

#define PI 3.14159265359f
#define RAD_TO_DEG 57.2957795f

float dist(v2f v, v2f w)
{
	return sqrt((v.x - w.x) * (v.x - w.x) + (v.y - w.y) * (v.y - w.y));
}

Stat::Stat() {}

Stat::Stat(float _speed, float _range, float _noise, std::string texture_path, int size)
{
	noise = _noise;
	speed = _speed;
	agent_tex.loadFromFile(texture_path);
	shape.setTexture(agent_tex);
	shape.setOrigin({ static_cast<float>(agent_tex.getSize().x) / 2, static_cast<float>(agent_tex.getSize().y) / 2 });
	float scale_factor = static_cast<float>(size) / agent_tex.getSize().x; //assumes agent_tex is a square. Else, only sets size along one axis
	shape.setScale({ scale_factor, scale_factor });
	detect_range = _range;
}

Stat::Stat(std::string texture_path, int size)
{
	noise = 0.f;
	speed = 0.f;
	agent_tex.loadFromFile(texture_path);
	shape.setTexture(agent_tex);
	shape.setOrigin({ static_cast<float>(agent_tex.getSize().x) / 2, static_cast<float>(agent_tex.getSize().y) / 2 });
	float scale_factor = static_cast<float>(size) / agent_tex.getSize().x; //assumes agent_tex is a square. Else, only sets size along one axis
	shape.setScale({ scale_factor, scale_factor });
	detect_range = 0.f;
}

Agent::Agent()
{
	next = nullptr;
	orientation = 0.f;
	curCell = nullptr;
	pos = { 0.f, 0.f };
	checked = false;
	idx = 0;
}

Agent::Agent(v2f _pos, float _orientation, int _idx)
{
	pos = _pos;
	orientation = _orientation;
	next = nullptr;
	curCell = nullptr;
	idx = _idx;
}

Engine::~Engine()
{
	w->close();
	delete w;
	delete agents_1;
	delete agents_2;
	delete cells;
	delete noise_gen_1;
	delete noise_gen_2;
}

void Engine::draw_agent_1(Agent* a)
{
	stat_a1.shape.setPosition(a->pos);
	stat_a1.shape.setRotation(a->orientation * RAD_TO_DEG);
	w->draw(stat_a1.shape);
}

void Engine::draw_agent_2(Agent* a)
{
	stat_a2.shape.setPosition(a->pos);
	stat_a2.shape.setRotation(a->orientation * RAD_TO_DEG);
	w->draw(stat_a2.shape);
}

void Engine::place_in_cell_1(Agent* a)
{
	int i = (int)a->pos.x / cellSize;
	int j = (int)a->pos.y / cellSize;

	i = i < 0 ? 0 : i;
	i = i >= gridWidth ? gridWidth - 1 : i;
	j = j < 0 ? 0 : j;
	j = j >= gridWidth ? gridWidth - 1 : j;

	cells->at(i * gridWidth + j)->insert_agent_1(a);
	a->curCell = cells->at(i * gridWidth + j);
}

void Engine::place_in_cell_2(Agent* a)
{
	int i = (int)a->pos.x / cellSize;
	int j = (int)a->pos.y / cellSize;

	i = i < 0 ? 0 : i;
	i = i >= gridWidth ? gridWidth - 1 : i;
	j = j < 0 ? 0 : j;
	j = j >= gridWidth ? gridWidth - 1 : j;

	cells->at(i * gridWidth + j)->insert_agent_2(a);
	a->curCell = cells->at(i * gridWidth + j);
}

void Engine::reset_masters()
{
	for (Cell* c : *cells)
	{
		c->master1 = nullptr;
		c->master2 = nullptr;
	}

	for (Agent* a : *agents_1)
	{
		a->next = nullptr;
		a->curCell = nullptr;
	}

	for (Agent* a : *agents_2)
	{
		a->next = nullptr;
		a->curCell = nullptr;
	}
}

void Engine::init_cells()
{
	cellSize = std::max(stat_a1.detect_range, stat_a2.detect_range);
	float n = L / cellSize;
	gridWidth = (int)n;
	cellSize = L / gridWidth; //gridWidth is topped to be sure it can fit the range AND is integer

	cells = new std::vector<Cell*>;
	cells->resize(gridWidth * gridWidth);

	for (size_t i = 0; i < gridWidth * gridWidth; i++)
		cells->at(i) = new Cell();

	for (size_t i = 0; i < gridWidth; i++)
	{
		for (size_t j = 0; j < gridWidth; j++)
		{
			cells->at(i * gridWidth + j)->idx = i * gridWidth + j;
			cells->at(i * gridWidth + j)->master1 = nullptr;
			cells->at(i * gridWidth + j)->master2 = nullptr;

			int prevCol, nextCol, prevRow, nextRow;
			prevCol = j - 1;
			if (prevCol < 0)
				prevCol = gridWidth - 1;
			nextCol = j + 1;
			if (nextCol > gridWidth - 1)
				nextCol = 0;
			prevRow = i - 1;
			if (prevRow < 0)
				prevRow = gridWidth - 1;
			nextRow = i + 1;
			if (nextRow > gridWidth - 1)
				nextRow = 0;

			cells->at(i * gridWidth + j)->neighbors[0] = prevRow * gridWidth + prevCol;
			cells->at(i * gridWidth + j)->neighbors[1] = prevRow * gridWidth + j;
			cells->at(i * gridWidth + j)->neighbors[2] = prevRow * gridWidth + nextCol;
			cells->at(i * gridWidth + j)->neighbors[3] = i * gridWidth + prevCol;
			cells->at(i * gridWidth + j)->neighbors[4] = i * gridWidth + nextCol;
			cells->at(i * gridWidth + j)->neighbors[5] = nextRow * gridWidth + prevCol;
			cells->at(i * gridWidth + j)->neighbors[6] = nextRow * gridWidth + j;
			cells->at(i * gridWidth + j)->neighbors[7] = nextRow * gridWidth + nextCol;
		}
	}

}

void Engine::init_agents()
{
	for (size_t i = 0; i < N1; i++)
	{
		agents_1->at(i) = new Agent;
		agents_1->at(i)->pos.x = (rand() % 999) / 1000.f * L;
		agents_1->at(i)->pos.y = (rand() % 999) / 1000.f * L;
		agents_1->at(i)->orientation = (rand() % 999) / 1000.f * 2 * PI;
		agents_1->at(i)->idx = i;
	}

	for (size_t i = 0; i < N2; i++)
	{
		agents_2->at(i) = new Agent;
		agents_2->at(i)->pos.x = (rand() % 999) / 1000.f * L;
		agents_2->at(i)->pos.y = (rand() % 999) / 1000.f * L;
		agents_2->at(i)->orientation = (rand() % 999) / 1000.f * 2 * PI;
		agents_2->at(i)->idx = i;
	}
}

void Engine::reset_checked_agents_1()
{
	for (Agent* a : *agents_1)
	{
		a->checked = false;
	}
}

void Engine::reset_checked_agents_2()
{
	for (Agent* a : *agents_2)
	{
		a->checked = false;
	}
}

void Engine::reset_checked_cells()
{
	for (Cell* c : *cells)
	{
		c->checked = false;
	}
}

void Engine::log_mean_orientation(std::string file)
{
	std::ofstream f(file, std::ios::app);

	if (f.is_open())
	{
		float meanOrientation = 0.f;
		float meanSin = 0.f;
		float meanCos = 0.f;
		for (size_t i = 0; i < N1; i++)
		{
			meanSin += sin(agents_1->at(i)->orientation);
			meanCos += cos(agents_1->at(i)->orientation);
		}

		for (size_t i = 0; i < N2; i++)
		{
			meanSin += sin(agents_2->at(i)->orientation);
			meanCos += cos(agents_2->at(i)->orientation);
		}

		meanSin /= N1 + N2;
		meanCos /= N1 + N2;

		f << totalTime << "," << atan2(meanSin, meanCos) << std::endl;
		f.close();
	}
}

void Engine::log_cluster_size(float R, std::string file)
{
	reset_checked_agents_1();
	reset_checked_agents_2();

	Cell* curCell;
	Cell* neighbourCell;
	Agent* checkedAgent;
	Agent* curAgent;
	std::pair<std::vector<int>*, std::vector<int>*>* curCluster;
	bool neighbour_found = false; //if no neighbour found in the cell, no need to add its neighbours to checklist

	std::vector<std::pair<std::vector<int>*, std::vector<int>*>*> clusters;
	for (Agent* a : *agents_1)
	{
		if (!a->checked)
		{
			clusters.push_back(new std::pair<std::vector<int>*, std::vector<int>*>);
			clusters.at(clusters.size() - 1)->first = new std::vector<int>;
			clusters.at(clusters.size() - 1)->second = new std::vector<int>;

			std::vector<Agent*> to_check_1;
			std::vector<Agent*> to_check_2;
			to_check_1.push_back(a);
			a->checked = true;

			while (to_check_1.size() > 0 || to_check_2.size() > 0) //executed while agents are still found at a distance of R from some agents
			{
				if (to_check_1.size() > 0) //checks every agent of specie 1 in the checklist
				{
					curAgent = to_check_1.at(to_check_1.size() - 1);
					to_check_1.pop_back();
					clusters.at(clusters.size() - 1)->first->push_back(curAgent->idx);

					curCell = curAgent->curCell;

					checkedAgent = curCell->master1;
					while (checkedAgent != nullptr) //checks agent of type 1 in the current cell of the agent_1 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_1.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					checkedAgent = curCell->master2;
					while (checkedAgent != nullptr) //checks agent of type 2 in the current cell of the agent_1 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_2.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					for (int neighbour_idx : curCell->neighbors)
					{
						neighbourCell = cells->at(neighbour_idx);

						checkedAgent = neighbourCell->master1;
						while (checkedAgent != nullptr) //checks agent of type 1 in the neighbour cells of the agent_1 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_1.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}

						checkedAgent = neighbourCell->master2;
						while (checkedAgent != nullptr) //checks agent of type 2 in the neighbour cells of the agent_1 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_2.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}
					}
				}

				if (to_check_2.size() > 0) //checks every agent of specie 2 in the checklist
				{
					curAgent = to_check_2.at(to_check_2.size() - 1);
					to_check_2.pop_back();
					clusters.at(clusters.size() - 1)->second->push_back(curAgent->idx);

					curCell = curAgent->curCell;

					checkedAgent = curCell->master1;
					while (checkedAgent != nullptr) //checks agent of type 1 in the current cell of the agent_2 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_1.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					checkedAgent = curCell->master2;
					while (checkedAgent != nullptr) //checks agent of type 2 in the current cell of the agent_2 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_2.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					for (int neighbour_idx : curCell->neighbors)
					{
						neighbourCell = cells->at(neighbour_idx);

						checkedAgent = neighbourCell->master1;
						while (checkedAgent != nullptr) //checks agent of type 1 in the neighbours cells of the agent_2 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_1.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}

						checkedAgent = neighbourCell->master2;
						while (checkedAgent != nullptr) //checks agent of type 2 in the neighbours cells of the agent_2 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_2.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}
					}
				}
			}
		}
	}

	for (Agent* a : *agents_2)
	{
		if (!a->checked)
		{
			clusters.push_back(new std::pair<std::vector<int>*, std::vector<int>*>);
			clusters.at(clusters.size() - 1)->first = new std::vector<int>;
			clusters.at(clusters.size() - 1)->second = new std::vector<int>;
			std::vector<Agent*> to_check_1;
			std::vector<Agent*> to_check_2;
			to_check_2.push_back(a);
			a->checked = true;

			while (to_check_1.size() > 0 || to_check_2.size() > 0) //executed while agents are still found at a distance of R from some agents
			{
				if (to_check_1.size() > 0) //checks every agent of specie 1 in the checklist
				{
					curAgent = to_check_1.at(to_check_1.size());
					to_check_1.pop_back();
					clusters.at(clusters.size() - 1)->first->push_back(curAgent->idx);

					curCell = curAgent->curCell;

					checkedAgent = curCell->master1;
					while (checkedAgent != nullptr) //checks agent of type 1 in the current cell of the agent_1 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_1.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					checkedAgent = curCell->master2;
					while (checkedAgent != nullptr) //checks agent of type 2 in the current cell of the agent_1 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_2.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					for (int neighbour_idx : curCell->neighbors)
					{
						neighbourCell = cells->at(neighbour_idx);

						checkedAgent = neighbourCell->master1;
						while (checkedAgent != nullptr) //checks agent of type 1 in the neighbour cells of the agent_1 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_1.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}

						checkedAgent = neighbourCell->master2;
						while (checkedAgent != nullptr) //checks agent of type 2 in the neighbour cells of the agent_1 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_2.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}
					}
				}

				if (to_check_2.size() > 0) //checks every agent of specie 2 in the checklist
				{
					curAgent = to_check_2.at(to_check_2.size() - 1);
					to_check_2.pop_back();
					clusters.at(clusters.size() - 1)->second->push_back(curAgent->idx);

					curCell = curAgent->curCell;

					checkedAgent = curCell->master1;
					while (checkedAgent != nullptr) //checks agent of type 1 in the current cell of the agent_2 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_1.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					checkedAgent = curCell->master2;
					while (checkedAgent != nullptr) //checks agent of type 2 in the current cell of the agent_2 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_2.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					for (int neighbour_idx : curCell->neighbors)
					{
						neighbourCell = cells->at(neighbour_idx);

						checkedAgent = neighbourCell->master1;
						while (checkedAgent != nullptr) //checks agent of type 1 in the neighbours cells of the agent_2 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_1.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}

						checkedAgent = neighbourCell->master2;
						while (checkedAgent != nullptr) //checks agent of type 2 in the neighbours cells of the agent_2 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_2.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}
					}
				}
			}
		}
	}


	std::ofstream f(file, std::ios::app);
	if (f.is_open())
	{
		for (std::pair<std::vector<int>*, std::vector<int>*>* cluster : clusters)
		{
			f << totalTime << "\t" << cluster->first->size() << "\t" << cluster->second->size() << std::endl;
		}
		f.close();
	}
}

float Engine::compute_segregation_param(float R)
{
	reset_checked_agents_1();
	reset_checked_agents_2();

	Cell* curCell;
	Cell* neighbourCell;
	Agent* checkedAgent;
	Agent* curAgent;
	std::pair<std::vector<int>*, std::vector<int>*>* curCluster;
	bool neighbour_found = false; //if no neighbour found in the cell, no need to add its neighbours to checklist

	std::vector<std::pair<std::vector<int>*, std::vector<int>*>*> clusters;
	for (Agent* a : *agents_1)
	{
		if (!a->checked)
		{
			clusters.push_back(new std::pair<std::vector<int>*, std::vector<int>*>);
			clusters.at(clusters.size() - 1)->first = new std::vector<int>;
			clusters.at(clusters.size() - 1)->second = new std::vector<int>;

			std::vector<Agent*> to_check_1;
			std::vector<Agent*> to_check_2;
			to_check_1.push_back(a);
			a->checked = true;

			while (to_check_1.size() > 0 || to_check_2.size() > 0) //executed while agents are still found at a distance of R from some agents
			{
				if (to_check_1.size() > 0) //checks every agent of specie 1 in the checklist
				{
					curAgent = to_check_1.at(to_check_1.size() - 1);
					to_check_1.pop_back();
					clusters.at(clusters.size() - 1)->first->push_back(curAgent->idx);

					curCell = curAgent->curCell;

					checkedAgent = curCell->master1;
					while (checkedAgent != nullptr) //checks agent of type 1 in the current cell of the agent_1 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_1.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					checkedAgent = curCell->master2;
					while (checkedAgent != nullptr) //checks agent of type 2 in the current cell of the agent_1 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_2.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					for (int neighbour_idx : curCell->neighbors)
					{
						neighbourCell = cells->at(neighbour_idx);

						checkedAgent = neighbourCell->master1;
						while (checkedAgent != nullptr) //checks agent of type 1 in the neighbour cells of the agent_1 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_1.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}

						checkedAgent = neighbourCell->master2;
						while (checkedAgent != nullptr) //checks agent of type 2 in the neighbour cells of the agent_1 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_2.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}
					}
				}

				if (to_check_2.size() > 0) //checks every agent of specie 2 in the checklist
				{
					curAgent = to_check_2.at(to_check_2.size() - 1);
					to_check_2.pop_back();
					clusters.at(clusters.size() - 1)->second->push_back(curAgent->idx);

					curCell = curAgent->curCell;

					checkedAgent = curCell->master1;
					while (checkedAgent != nullptr) //checks agent of type 1 in the current cell of the agent_2 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_1.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					checkedAgent = curCell->master2;
					while (checkedAgent != nullptr) //checks agent of type 2 in the current cell of the agent_2 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_2.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					for (int neighbour_idx : curCell->neighbors)
					{
						neighbourCell = cells->at(neighbour_idx);

						checkedAgent = neighbourCell->master1;
						while (checkedAgent != nullptr) //checks agent of type 1 in the neighbours cells of the agent_2 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_1.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}

						checkedAgent = neighbourCell->master2;
						while (checkedAgent != nullptr) //checks agent of type 2 in the neighbours cells of the agent_2 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_2.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}
					}
				}
			}
		}
	}

	for (Agent* a : *agents_2)
	{
		if (!a->checked)
		{
			clusters.push_back(new std::pair<std::vector<int>*, std::vector<int>*>);
			clusters.at(clusters.size() - 1)->first = new std::vector<int>;
			clusters.at(clusters.size() - 1)->second = new std::vector<int>;
			std::vector<Agent*> to_check_1;
			std::vector<Agent*> to_check_2;
			to_check_2.push_back(a);
			a->checked = true;

			while (to_check_1.size() > 0 || to_check_2.size() > 0) //executed while agents are still found at a distance of R from some agents
			{
				if (to_check_1.size() > 0) //checks every agent of specie 1 in the checklist
				{
					curAgent = to_check_1.at(to_check_1.size());
					to_check_1.pop_back();
					clusters.at(clusters.size() - 1)->first->push_back(curAgent->idx);

					curCell = curAgent->curCell;

					checkedAgent = curCell->master1;
					while (checkedAgent != nullptr) //checks agent of type 1 in the current cell of the agent_1 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_1.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					checkedAgent = curCell->master2;
					while (checkedAgent != nullptr) //checks agent of type 2 in the current cell of the agent_1 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_2.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					for (int neighbour_idx : curCell->neighbors)
					{
						neighbourCell = cells->at(neighbour_idx);

						checkedAgent = neighbourCell->master1;
						while (checkedAgent != nullptr) //checks agent of type 1 in the neighbour cells of the agent_1 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_1.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}

						checkedAgent = neighbourCell->master2;
						while (checkedAgent != nullptr) //checks agent of type 2 in the neighbour cells of the agent_1 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_2.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}
					}
				}

				if (to_check_2.size() > 0) //checks every agent of specie 2 in the checklist
				{
					curAgent = to_check_2.at(to_check_2.size() - 1);
					to_check_2.pop_back();
					clusters.at(clusters.size() - 1)->second->push_back(curAgent->idx);

					curCell = curAgent->curCell;

					checkedAgent = curCell->master1;
					while (checkedAgent != nullptr) //checks agent of type 1 in the current cell of the agent_2 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_1.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					checkedAgent = curCell->master2;
					while (checkedAgent != nullptr) //checks agent of type 2 in the current cell of the agent_2 in the checklist
					{
						if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
						{
							to_check_2.push_back(checkedAgent);
							checkedAgent->checked = true;
						}
						checkedAgent = checkedAgent->next;
					}

					for (int neighbour_idx : curCell->neighbors)
					{
						neighbourCell = cells->at(neighbour_idx);

						checkedAgent = neighbourCell->master1;
						while (checkedAgent != nullptr) //checks agent of type 1 in the neighbours cells of the agent_2 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_1.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}

						checkedAgent = neighbourCell->master2;
						while (checkedAgent != nullptr) //checks agent of type 2 in the neighbours cells of the agent_2 in the checklist
						{
							if (dist(curAgent->pos, checkedAgent->pos) < R && !checkedAgent->checked)
							{
								to_check_2.push_back(checkedAgent);
								checkedAgent->checked = true;
							}
							checkedAgent = checkedAgent->next;
						}
					}
				}
			}
		}
	}

	int segregation = 0;
	for (std::pair<std::vector<int>*, std::vector<int>*>* cluster : clusters)
	{
		segregation += abs((int)cluster->first->size() - (int)cluster->second->size());
	}

	return static_cast<float>(segregation) / (N1 + N2);
}

float Engine::compute_polarization_1()
{
	v2f sum_1 = { 0.f, 0.f };

	for (Agent* a : *agents_1)
	{
		sum_1.x += cos(a->orientation);
		sum_1.y += sin(a->orientation);
	}

	v2f pol_1 = { sum_1.x / N1, sum_1.y / N1 };
	return sqrt(pol_1.x * pol_1.x + pol_1.y * pol_1.y);
}

float Engine::compute_polarization_2()
{
	v2f sum_2 = { 0.f, 0.f };

	for (Agent* a : *agents_2)
	{
		sum_2.x += cos(a->orientation);
		sum_2.y += sin(a->orientation);
	}

	v2f pol_2 = { sum_2.x / N2, sum_2.y / N2 };
	return sqrt(pol_2.x * pol_2.x + pol_2.y * pol_2.y);
}

float Engine::compute_polarization_tot()
{
	v2f sum_1 = { 0.f, 0.f };
	v2f sum_2 = { 0.f, 0.f };

	for (Agent* a : *agents_1)
	{
		sum_1.x += cos(a->orientation);
		sum_1.y += sin(a->orientation);
	}

	for (Agent* a : *agents_2)
	{
		sum_2.x += cos(a->orientation);
		sum_2.y += sin(a->orientation);
	}

	v2f pol_1 = { sum_1.x / N1, sum_1.y / N1 };
	v2f pol_2 = { sum_2.x / N2, sum_2.y / N2 };
	v2f pol_tot = { (sum_1.x + sum_2.x) / (N1 + N2), (sum_1.y + sum_2.y) / (N1 + N2) };
	return sqrt(pol_tot.x * pol_tot.x + pol_tot.y * pol_tot.y);
}

float Engine::compute_clustering(float R)
{
	
}

void Engine::log_polarization(std::string file)
{
	v2f sum_1 = { 0.f, 0.f };
	v2f sum_2 = { 0.f, 0.f };
	
	for (Agent* a : *agents_1)
	{
		sum_1.x += cos(a->orientation);
		sum_1.y += sin(a->orientation);
	}

	for (Agent* a : *agents_2)
	{
		sum_2.x += cos(a->orientation);
		sum_2.y += sin(a->orientation);
	}

	v2f pol_1 = { sum_1.x / N1, sum_1.y / N1 };
	v2f pol_2 = { sum_2.x / N2, sum_2.y / N2 };
	v2f pol_tot = { (sum_1.x + sum_2.x) / (N1 + N2), (sum_1.y + sum_2.y) / (N1 + N2) };

	std::ofstream f(file, std::ios::app);
	if (f.is_open())
	{
		//write the norms of the polarization vectors
		f << std::fixed << std::setprecision(6) << totalTime << "\t" << sqrt(pol_1.x * pol_1.x + pol_1.y * pol_1.y) << "\t" << sqrt(pol_2.x * pol_2.x + pol_2.y * pol_2.y) << "\t" << sqrt(pol_tot.x * pol_tot.x + pol_tot.y * pol_tot.y) << std::endl;
	}
}

void Engine::update_total_time()
{
	totalTime += dt;
}

void Engine::update_orientation() //currently : agents align with every other agents
{
	float meanSin, meanCos;
	size_t count;
	Cell* c;
	Agent* curA;

	for (Agent* a : *agents_1) //updates agent type 1
	{
		meanSin = 0.f;// sin(a->orientation);
		meanCos = 0.f;//cos(a->orientation);
		count = 1;

		c = a->curCell;
		curA = c->master1;

		while (curA != nullptr) //agent 1 tests for every agent 1 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a1.detect_range)
			{
				meanSin += sin(curA->orientation);
				meanCos += cos(curA->orientation);
				count++;
			}
			curA = curA->next;
		}

		curA = c->master2; 
		while (curA != nullptr) //agent 1 tests for every agent 2 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a1.detect_range)
			{
				meanSin += sin(curA->orientation);
				meanCos += cos(curA->orientation);
				count++;
			}
			curA = curA->next;
		}

		for (size_t i = 0; i < 8; i++) 
		{
			curA = cells->at(c->neighbors.at(i))->master1;
			while (curA != nullptr) //agent 1 tests for every agent 1 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a1.detect_range)
				{
					meanSin += sin(curA->orientation);
					meanCos += cos(curA->orientation);
					count++;
				}
				curA = curA->next;
			}	

			curA = cells->at(c->neighbors.at(i))->master2; 
			while (curA != nullptr) //agent 1 tests for every agent 2 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a1.detect_range)
				{
					meanSin += sin(curA->orientation);
					meanCos += cos(curA->orientation);
					count++;
				}
				curA = curA->next;
			}
		}

		meanCos /= count;
		meanSin /= count;

		a->orientation = atan2f(meanSin, meanCos);
		a->orientation += noise_gen_1->operator()(rnd_engine);
	}

	for (Agent* a : *agents_2) //updates agent type 2
	{
		meanSin = sin(a->orientation);
		meanCos = cos(a->orientation);
		count = 1;

		c = a->curCell;
		curA = c->master1;

		while (curA != nullptr) //agent 2 tests for every agent 1 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a2.detect_range)
			{
				meanSin += sin(curA->orientation);
				meanCos += cos(curA->orientation);
				count++;
			}
			curA = curA->next;
		}

		curA = c->master2;
		while (curA != nullptr) //agent 2 tests for every agent 2 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a2.detect_range)
			{
				meanSin += sin(curA->orientation);
				meanCos += cos(curA->orientation);
				count++;
			}
			curA = curA->next;
		}

		for (size_t i = 0; i < 8; i++)
		{
			curA = cells->at(c->neighbors.at(i))->master1;
			while (curA != nullptr) //agent 2 tests for every agent 1 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a2.detect_range)
				{
					meanSin += sin(curA->orientation);
					meanCos += cos(curA->orientation);
					count++;
				}
				curA = curA->next;
			}

			curA = cells->at(c->neighbors.at(i))->master2;
			while (curA != nullptr) //agent 2 tests for every agent 2 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a2.detect_range)
				{
					meanSin += sin(curA->orientation);
					meanCos += cos(curA->orientation);
					count++;
				}
				curA = curA->next;
			}
		}

		meanCos /= count;
		meanSin /= count;

		a->orientation = atan2f(meanSin, meanCos);
		a->orientation += noise_gen_2->operator()(rnd_engine);
	}
}

void Engine::update_orientation_dodge()
{
	float meanSin, meanCos;
	size_t count;
	Cell* c;
	Agent* curA;

	for (Agent* a : *agents_1)
	{
		meanSin = 0.f;// sin(a->orientation);
		meanCos = 0.f;//cos(a->orientation);
		count = 1;

		c = a->curCell;
		curA = c->master1;

		while (curA != nullptr) //agent 1 tests for every agent 1 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a1.detect_range)
			{
				meanSin += sin(curA->orientation);
				meanCos += cos(curA->orientation);
				count++;
			}
			curA = curA->next;
		}

		curA = c->master2;
		while (curA != nullptr) //agent 1 tests for every agent 2 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a1.detect_range)
			{
				meanSin += sin(atan2(a->pos.y - curA->pos.y, a->pos.x - curA->pos.x));
				meanCos += cos(atan2(a->pos.y - curA->pos.y, a->pos.x - curA->pos.x));
				count++;
			}
			curA = curA->next;
		}

		for (size_t i = 0; i < 8; i++)
		{
			curA = cells->at(c->neighbors.at(i))->master1;
			while (curA != nullptr) //agent 1 tests for every agent 1 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a1.detect_range)
				{
					meanSin += sin(curA->orientation);
					meanCos += cos(curA->orientation);
					count++;
				}
				curA = curA->next;
			}

			curA = cells->at(c->neighbors.at(i))->master2;
			while (curA != nullptr) //agent 1 tests for every agent 2 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a1.detect_range)
				{
					meanSin += sin(atan2(a->pos.y - curA->pos.y, a->pos.x - curA->pos.x));
					meanCos += cos(atan2(a->pos.y - curA->pos.y, a->pos.x - curA->pos.x));
					count++;
				}
				curA = curA->next;
			}
		}

		meanCos /= count;
		meanSin /= count;

		a->orientation = atan2f(meanSin, meanCos);
		a->orientation += noise_gen_1->operator()(rnd_engine);
		//std::cout << 
	}

	for (Agent* a : *agents_2)
	{
		meanSin = sin(a->orientation);
		meanCos = cos(a->orientation);
		count = 1;

		c = a->curCell;
		curA = c->master1;

		while (curA != nullptr) //agent 2 tests for every agent 1 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a2.detect_range)
			{
				meanSin += sin(atan2(a->pos.y - curA->pos.y, a->pos.x - curA->pos.x));
				meanCos += cos(atan2(a->pos.y - curA->pos.y, a->pos.x - curA->pos.x));
				count++;
			}
			curA = curA->next;
		}

		curA = c->master2;
		while (curA != nullptr) //agent 2 tests for every agent 2 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a2.detect_range)
			{
				meanSin += sin(curA->orientation);
				meanCos += cos(curA->orientation);
				count++;
			}
			curA = curA->next;
		}

		for (size_t i = 0; i < 8; i++)
		{
			curA = cells->at(c->neighbors.at(i))->master1;
			while (curA != nullptr) //agent 2 tests for every agent 1 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a2.detect_range)
				{
					meanSin += sin(atan2(a->pos.y - curA->pos.y, a->pos.x - curA->pos.x));
					meanCos += cos(atan2(a->pos.y - curA->pos.y, a->pos.x - curA->pos.x));
					count++;
				}
				curA = curA->next;
			}

			curA = cells->at(c->neighbors.at(i))->master2;
			while (curA != nullptr) //agent 2 tests for every agent 2 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a2.detect_range)
				{
					meanSin += sin(curA->orientation);
					meanCos += cos(curA->orientation);
					count++;
				}
				curA = curA->next;
			}
		}

		meanCos /= count;
		meanSin /= count;

		a->orientation = atan2f(meanSin, meanCos);
		a->orientation += noise_gen_2->operator()(rnd_engine);
	}
}

void Engine::update_orientation_weights()
{
	float meanSin, meanCos;
	size_t count;
	Cell* c;
	Agent* curA;

	for (Agent* a : *agents_1) //updates agent type 1
	{
		meanSin = 0.f;// sin(a->orientation);
		meanCos = 0.f;//cos(a->orientation);
		count = 1;

		c = a->curCell;
		curA = c->master1;

		while (curA != nullptr) //agent 1 tests for every agent 1 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a1.detect_range)
			{
				meanSin += sin(curA->orientation) * weight_11;
				meanCos += cos(curA->orientation) * weight_11;
				count++;
			}
			curA = curA->next;
		}

		curA = c->master2;
		while (curA != nullptr) //agent 1 tests for every agent 2 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a1.detect_range)
			{
				meanSin += sin(curA->orientation) * weight_12;
				meanCos += cos(curA->orientation) * weight_12;
				count++;
			}
			curA = curA->next;
		}

		for (size_t i = 0; i < 8; i++)
		{
			curA = cells->at(c->neighbors.at(i))->master1;
			while (curA != nullptr) //agent 1 tests for every agent 1 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a1.detect_range)
				{
					meanSin += sin(curA->orientation) * weight_11;
					meanCos += cos(curA->orientation) * weight_11;
					count++;
				}
				curA = curA->next;
			}

			curA = cells->at(c->neighbors.at(i))->master2;
			while (curA != nullptr) //agent 1 tests for every agent 2 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a1.detect_range)
				{
					meanSin += sin(curA->orientation) * weight_12;
					meanCos += cos(curA->orientation) * weight_12;
					count++;
				}
				curA = curA->next;
			}
		}

		meanCos /= count;
		meanSin /= count;

		a->orientation = atan2f(meanSin, meanCos);
		a->orientation += noise_gen_1->operator()(rnd_engine);
	}

	for (Agent* a : *agents_2) //updates agent type 2
	{
		meanSin = sin(a->orientation);
		meanCos = cos(a->orientation);
		count = 1;

		c = a->curCell;
		curA = c->master1;

		while (curA != nullptr) //agent 2 tests for every agent 1 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a2.detect_range)
			{
				meanSin += sin(curA->orientation) * weight_21;
				meanCos += cos(curA->orientation) * weight_21;
				count++;
			}
			curA = curA->next;
		}

		curA = c->master2;
		while (curA != nullptr) //agent 2 tests for every agent 2 in it's cell
		{
			if (dist(a->pos, curA->pos) < stat_a2.detect_range)
			{
				meanSin += sin(curA->orientation) * weight_22;
				meanCos += cos(curA->orientation) * weight_22;
				count++;
			}
			curA = curA->next;
		}

		for (size_t i = 0; i < 8; i++)
		{
			curA = cells->at(c->neighbors.at(i))->master1;
			while (curA != nullptr) //agent 2 tests for every agent 1 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a2.detect_range)
				{
					meanSin += sin(curA->orientation) * weight_21;
					meanCos += cos(curA->orientation) * weight_21;
					count++;
				}
				curA = curA->next;
			}

			curA = cells->at(c->neighbors.at(i))->master2;
			while (curA != nullptr) //agent 2 tests for every agent 2 in neighbor cells
			{
				if (dist(a->pos, curA->pos) < stat_a2.detect_range)
				{
					meanSin += sin(curA->orientation) * weight_22;
					meanCos += cos(curA->orientation) * weight_22;
					count++;
				}
				curA = curA->next;
			}
		}

		meanCos /= count;
		meanSin /= count;

		a->orientation = atan2f(meanSin, meanCos);
		a->orientation += noise_gen_2->operator()(rnd_engine);
	}
}

void Engine::update_pos()
{
	for (Agent* a : *agents_1)
	{
		a->pos.x += dt * stat_a1.speed * cos(a->orientation);
		a->pos.y += dt * stat_a1.speed * sin(a->orientation);

		if (a->pos.x > L)
			a->pos.x -= L;
		if (a->pos.x < 0)
			a->pos.x += L;
		if (a->pos.y > L)
			a->pos.y -= L;
		if (a->pos.y < 0)
			a->pos.y += L;
	}

	for (Agent* a : *agents_2)
	{
		a->pos.x += dt * stat_a2.speed * cos(a->orientation);
		a->pos.y += dt * stat_a2.speed * sin(a->orientation);

		if (a->pos.x > L)
			a->pos.x -= L;
		if (a->pos.x < 0)
			a->pos.x += L;
		if (a->pos.y > L)
			a->pos.y -= L;
		if (a->pos.y < 0)
			a->pos.y += L;
	}

}

void Engine::update_cells()
{
	reset_masters();
	for (size_t i = 0; i < N1; i++)
	{
		place_in_cell_1(agents_1->at(i));
	}
	for (size_t i = 0; i < N2; i++)
	{
		place_in_cell_2(agents_2->at(i));
	}
}

void Engine::draw_sim()
{
	for (Agent* a : *agents_1)
	{
		draw_agent_1(a);
	}

	for (Agent* a : *agents_2)
	{
		draw_agent_2(a);
	}
}

Engine::Engine(float _L, float _dt, int _N1, int _N2, Stat& stat_a1, Stat& stat_a2, size_t framerate)
{
	srand(time(NULL));

	noise_gen_1 = new std::uniform_real_distribution<float>(-stat_a1.noise,stat_a1.noise);
	noise_gen_2 = new std::uniform_real_distribution<float>(-stat_a2.noise, stat_a2.noise);
	current_orientation_update = &Engine::update_orientation;

	w = new sf::RenderWindow(sf::VideoMode((int)_L, (int)_L), "Vicsek model");
	w->setFramerateLimit(framerate);
	L = _L;
	dt = _dt;
	N1 = _N1;
	N2 = _N2;
	weight_11 = 0.f;
	weight_12 = 0.f;
	weight_21 = 0.f;
	weight_22 = 0.f;

	set_agent_stat(stat_a1, 1);
	set_agent_stat(stat_a2, 2);

	agents_1 = new std::vector<Agent*>;
	agents_2 = new std::vector<Agent*>;

	agents_1->resize(_N1);
	agents_2->resize(_N2);
	
	for (size_t i = 0; i < _N1; i++)
	{
		agents_1->at(i) = new Agent;
		agents_1->at(i)->pos.x = (rand() % 999) / 1000.f * L;
		agents_1->at(i)->pos.y = (rand() % 999) / 1000.f * L;
		agents_1->at(i)->orientation = (rand() % 999) / 1000.f * 2 * PI;
		agents_1->at(i)->idx = i;
	}

	for (size_t i = 0; i < _N2; i++)
	{
		agents_2->at(i) = new Agent;
		agents_2->at(i)->pos.x = (rand() % 999) / 1000.f * L;
		agents_2->at(i)->pos.y = (rand() % 999) / 1000.f * L;
		agents_2->at(i)->orientation = (rand() % 999) / 1000.f * 2 * PI;
		agents_2->at(i)->idx = i;
	}

	init_cells();
}

void Engine::set_agent_stat(Stat& a, size_t idx)
{
	if (idx == 1)
		stat_a1 = a;
	else if (idx == 2)
		stat_a2 = a;
	else
		std::cout << "set_agent_stats : idx should be either 1 or 2\n";
}

bool Engine::is_running()
{
	return w->isOpen();
}

void Engine::run_and_display()
{
	while (w->pollEvent(ev))
	{
		switch (ev.type)
		{
		default:
			break;
		case sf::Event::Closed:
			w->close();
			break;
		}
	}

	run();

	w->clear();
	draw_sim();
	w->display();

}

void Engine::run()
{
	update_cells();
	(this->*current_orientation_update)();
	update_pos();
	update_total_time();
}

void Engine::run_for(float duration)
{
	float init_time = totalTime;

	while (totalTime < init_time + duration)
		run();
}

float Engine::get_elapsed_time()
{
	return totalTime;
}

void Engine::set_orientation_update(void(Engine::* new_update)())
{
	current_orientation_update = new_update;
}

void Engine::set_N1(int N)
{
	N1 = N;
}

void Engine::set_N2(int N)
{
	N2 = N;
}

void Engine::set_speed_1(float speed)
{
	stat_a1.speed = speed;
}

void Engine::set_speed_2(float speed)
{
	stat_a2.speed = speed;
}

void Engine::set_d_range_1(float d_range)
{
	stat_a1.detect_range = d_range;
}

void Engine::set_d_range_2(float d_range)
{
	stat_a2.detect_range = d_range;
}

void Engine::set_noise_1(float noise)
{
	stat_a1.noise = noise;
	delete noise_gen_1;
	noise_gen_1 = new std::uniform_real_distribution<float>(-stat_a1.noise, stat_a1.noise);
}

void Engine::set_noise_2(float noise)
{
	stat_a2.noise = noise;
	delete noise_gen_2;
	noise_gen_2 = new std::uniform_real_distribution<float>(-stat_a2.noise, stat_a2.noise);
}

void Engine::set_weight_11(float w)
{
	weight_11 = w;
}

void Engine::set_weight_12(float w)
{
	weight_12 = w;
}

void Engine::set_weight_21(float w)
{
	weight_21 = w;
}

void Engine::set_weight_22(float w)
{
	weight_22 = w;
}

void Engine::set_dt(float dt)
{
	this->dt = dt;
}

void Engine::reset()
{
	for (int i = 0; i < agents_1->size(); i++)
		delete agents_1->at(i);

	for (int i = 0; i < agents_2->size(); i++)
		delete agents_2->at(i);
	
	for (int i = 0; i < cells->size(); i++)
		delete cells->at(i);

	agents_1->resize(N1);
	agents_2->resize(N2);

	init_cells();
	init_agents();
	reset_total_time();
}

void Engine::close_window()
{
	w->close();
}

void Engine::reset_total_time()
{
	totalTime = 0.f;
}

void Cell::insert_agent_1(Agent* a)
{
	if (master1 == nullptr)
		master1 = a;
	else
	{
		a->next = master1;
		master1 = a;
	}
}

void Cell::insert_agent_2(Agent* a)
{
	if (master2 == nullptr)
		master2 = a;
	else
	{
		a->next = master2;
		master2 = a;
	}
}