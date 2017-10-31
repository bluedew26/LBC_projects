#pragma once

struct gamemap_point
{
	int num_of_node;
	int point[100][2];
	double final_time[100];

	gamemap_point()
	{
		num_of_node = 0;
		for (int i = 0; i < 100; i++)
			final_time[i] = 0;
	}
};

#define MAXLENGTH 3000

struct output
{
	int n;
	int x[MAXLENGTH];
	int y[MAXLENGTH];
	float v[MAXLENGTH];
	float t[MAXLENGTH];

	output(int length)
	{
	}
	~output()
	{
	}
};
