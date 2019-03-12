//eliminate the corresponding char that is the same

//wrong!
//example: 'cdaba', 'aba'
//should be 2, but in this case, it's 3

PG_FUNCTION_INFO_V1(levenshtein_distance_test);

Datum levenshtein_distance_test(PG_FUNCTION_ARGS)
{
	//strict function, no need to check for null inputs

	//get arguments
	text *src = PG_GETARG_TEXT_PP(0);
	text *dst = PG_GETARG_TEXT_PP(1);

	const char *s_da;
	const char *t_da;
	int s_by, t_by;
	int s_bytes = 0, t_bytes = 0;
	int i, j;
	int small, big;
	bool flag;
	static char s_data[L_SIZE];
	static char t_data[L_SIZE];

	//2D array
	static int distance[L_SIZE][L_SIZE];

	//define pointers
	s_da = VARDATA_ANY(src);
	t_da = VARDATA_ANY(dst);

	//length of each string(bytes)
	s_by = VARSIZE_ANY_EXHDR(src);
	t_by = VARSIZE_ANY_EXHDR(dst);	

	if(s_by < t_by)
	{
		small = s_by;
		big = t_by;
		flag = 1;
	}
	else
	{
		small = t_by;
		big = s_by;
		flag = 0;
	}
	for(i = 0; i < small; i++)
	{
		if(tolower(s_da[i]) != tolower(t_da[i]))
		{
			s_data[s_bytes++] = tolower(s_da[i]);
			t_data[t_bytes++] = tolower(t_da[i]);
		}
	}
	if(flag)
		for(i = small; i < big; i++)
			t_data[t_bytes++] = tolower(t_da[i]);
	else
		for(i = small; i < big; i++)
			s_data[s_bytes++] = tolower(s_da[i]);

	//initialize
	for(j = 1; j <= s_bytes; j++)
		distance[0][j] = j;
	for(i = 1; i <= t_bytes; i++)
		distance[i][0] = i;

	//dynamic planning
	//O(m * n)
	for(i = 1; i <= t_bytes; i++)
		for(j = 1; j <= s_bytes; j++)
			if(s_data[j - 1] == t_data[i - 1])//case insensitive
				distance[i][j] = distance[i - 1][j - 1];
			else
			{
				//get the smallest
				if(distance[i - 1][j] <= distance[i][j - 1])
					small = distance[i - 1][j];
				else
					small = distance[i][j - 1];
				if(small > distance[i - 1][j - 1])
					small = distance[i - 1][j - 1];

				distance[i][j] = small + 1;
			}

	//return levenshtein distance
	PG_RETURN_INT32(distance[t_bytes][s_bytes]);
}