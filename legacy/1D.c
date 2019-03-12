Datum levenshtein_distance(PG_FUNCTION_ARGS)
{
	//strict function, no need to check for null inputs

	//get arguments
	text *src = PG_GETARG_TEXT_PP(0);
	text *dst = PG_GETARG_TEXT_PP(1);

	const char *s_data;
	const char *t_data;
	int	s_bytes,t_bytes;
	int i, j;
	int small;

	//1D array
	static int distance[16384];

	//define pointers
	s_data = VARDATA_ANY(src);
	t_data = VARDATA_ANY(dst);

	//length of each string(bytes)
	s_bytes = VARSIZE_ANY_EXHDR(src);
	t_bytes = VARSIZE_ANY_EXHDR(dst);	

	//initialize
	for(j = 1; j <= s_bytes; j++)
		distance[j] = j;
	for(i = 1; i <= t_bytes; i++)
		distance[i*s_bytes] = i;

	//dynamic planning
	//O(m * n)
	for(i = 1; i <= t_bytes; i++)
		for(j = 1; j <= s_bytes; j++)
			//if(toupper(s_data[j - 1]) == toupper(t_data[i - 1]))
			if(tolower(s_data[j - 1]) == tolower(t_data[i - 1]))
				distance[i*s_bytes+j] = distance[(i - 1)*s_bytes+j - 1];
			else
			{
				//get the smallest
				if(distance[(i - 1)*s_bytes+j] <= distance[i*s_bytes+j - 1])
					small = distance[(i - 1)*s_bytes+j];
				else
					small = distance[i*s_bytes+j - 1];
				if(small > distance[(i - 1)*s_bytes+j - 1])
					small = distance[(i - 1)*s_bytes+j - 1];

				distance[i*s_bytes+j] = small + 1;
			}

	//return levenshtein distance
	PG_RETURN_INT32(distance[(t_bytes+1)*s_bytes]);
}