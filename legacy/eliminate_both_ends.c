PG_FUNCTION_INFO_V1(levenshtein_distance_test);

Datum levenshtein_distance_test(PG_FUNCTION_ARGS)
{
	//strict function, no need to check for null inputs

	//get arguments
	text *src = PG_GETARG_TEXT_PP(0);
	text *dst = PG_GETARG_TEXT_PP(1);

	const char *s_data;
	const char *t_data;
	int s_by, t_by;
	int s_bytes, t_bytes;
	int s_start = 0, t_start = 0;
	int i, j;
	int small;

	//2D array
	static int distance[L_SIZE][L_SIZE];

	//define pointers
	s_data = VARDATA_ANY(src);
	t_data = VARDATA_ANY(dst);

	//length of each string(bytes)
	s_by = VARSIZE_ANY_EXHDR(src);
	t_by = VARSIZE_ANY_EXHDR(dst);	

	s_bytes = s_by;
	t_bytes = t_by;

	for(i = 0; i < s_by && i < t_by; i++)
	{
		if(tolower(s_data[i]) == tolower(t_data[i]))
		{
			s_start++;
			t_start++;
		}
		else
			break;
	}
	for(i = t_by - 1, j = s_by - 1; i > t_start && j > s_start; i--, j--)
		if(tolower(s_data[j]) == tolower(t_data[i]))
		{
			s_bytes--;
			t_bytes--;
		}
		else
			break;

	//initialize
	for(j = 1; j <= s_bytes - s_start; j++)
		distance[0][j] = j;
	for(i = 1; i <= t_bytes - t_start; i++)
		distance[i][0] = i;

	//dynamic planning
	//O(m * n)
	for(i = 1; i <= t_bytes - t_start; i++)
		for(j = 1; j <= s_bytes - s_start; j++)
			if(tolower(s_data[j - 1 + s_start]) == tolower(t_data[i - 1 + t_start]))//case insensitive
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
	PG_RETURN_INT32(distance[t_bytes - t_start][s_bytes - s_start]);
}