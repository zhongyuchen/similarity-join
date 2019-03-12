#include "postgres.h"
#include "fmgr.h"

#include <ctype.h>
//tolower()

//magic lock
#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#define L_SIZE 100
#define J_SIZE 128
#define J_SIZE2 256

//Version 1 Calling Conventions

//levenshtein distance
PG_FUNCTION_INFO_V1(levenshtein_distance);

Datum levenshtein_distance(PG_FUNCTION_ARGS)
{
	//strict function, no need to check for null inputs

	//get arguments
	text *src = PG_GETARG_TEXT_PP(0);
	text *dst = PG_GETARG_TEXT_PP(1);

	const char *s_data;
	const char *t_data;
	int s_bytes, t_bytes;
	int i, j;
	int small;

	//2D array
	static int distance[L_SIZE][L_SIZE];

	//define pointers
	s_data = VARDATA_ANY(src);
	t_data = VARDATA_ANY(dst);

	//length of each string(bytes)
	s_bytes = VARSIZE_ANY_EXHDR(src);
	t_bytes = VARSIZE_ANY_EXHDR(dst);	

	//initialize
	for(j = 1; j <= s_bytes; j++)
		distance[0][j] = j;
	for(i = 1; i <= t_bytes; i++)
		distance[i][0] = i;

	//dynamic planning
	//O(m * n)
	for(i = 1; i <= t_bytes; i++)
		for(j = 1; j <= s_bytes; j++)
			if(tolower(s_data[j - 1]) == tolower(t_data[i - 1]))//case insensitive
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

//using the threshold
PG_FUNCTION_INFO_V1(levenshtein_distance_2);
Datum levenshtein_distance_2(PG_FUNCTION_ARGS)
{
    text       *string1=PG_GETARG_TEXT_PP(0);
    text       *string2=PG_GETARG_TEXT_PP(1);
    int        d=PG_GETARG_INT32(2)-1;
	const char *str1 = VARDATA_ANY(string1);
	const char *str2 = VARDATA_ANY(string2);
	bool	   result;
	int        i,j;
    static int distance[120][120];
	int        n,m,L,R,KL,KR;
	n = VARSIZE_ANY_EXHDR(string1);
	m = VARSIZE_ANY_EXHDR(string2);
	if(abs(n-m)>d) PG_RETURN_BOOL(false);

    for(i=0;i<=n;i++)
        distance[i][0]=i;
    for(j=0;j<=m;j++)
        distance[0][j]=j;

    KL=-((d-abs(n-m))/2);
    KR=(d-abs(n-m))/2;
    if (n<= m)
        KL+=n-m;
    else
        KR+=n-m;

    for (i=1;i<=n;i++) {
        L=1>i-KR?1:i-KR;
        R=m<i-KL?m:i-KL;
        for (j=L;j<=R;j++) {
            distance[i][j] = distance[i-1][j-1]+(toupper(str1[i-1])==toupper(str2[j-1])?0:1);
            if (KL<=(i-1)-j&&(i-1)-j<=KR)
                distance[i][j]=distance[i][j]<distance[i-1][j]+1?distance[i][j]:distance[i-1][j]+1;
            if (KL<=i-(j-1)&&i-(j-1)<=KR)
                distance[i][j]=distance[i][j]<distance[i][j-1]+1?distance[i][j]:distance[i][j-1]+1;
        }
    }
    result=distance[n][m]<=d;
    PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(jaccard_index);

Datum jaccard_index(PG_FUNCTION_ARGS)
{
	//strict function, no need to check for null inputs

	//get arguments
	text *src = PG_GETARG_TEXT_PP(0);
	text *dst = PG_GETARG_TEXT_PP(1);

	const char *s_data;
	const char *t_data;
	int	s_bytes,t_bytes;
	int i;
	char a, b;
	double intersect = 0, join = 0;

	static int hash[J_SIZE][J_SIZE];
	static int clear[2][J_SIZE2];
	int cp = 0;

	//define pointers
	s_data = VARDATA_ANY(src);
	t_data = VARDATA_ANY(dst);

	//length of each string(bytes)
	s_bytes = VARSIZE_ANY_EXHDR(src);
	t_bytes = VARSIZE_ANY_EXHDR(dst);	

	//O(m + n)
	//hash src
	for(i = -1; i < s_bytes; i++)
	{	
		a = (i < 0)? '$':s_data[i];
		b = (i + 1 < s_bytes)? s_data[i + 1]:'$';
		if(!hash[a][b])
		{
			join++;
			hash[a][b] = 1;
			clear[0][cp] = a;
			clear[1][cp++] = b;
		}	
	}

	//hash dst
	for(i = -1; i < t_bytes; i++)
	{	
		a = (i < 0)? '$':t_data[i];
		b = (i + 1 < t_bytes)? t_data[i + 1]:'$';
		switch(hash[a][b])
		{
			case 0: join++; hash[a][b] = 2; clear[0][cp] = a; clear[1][cp++] = b; break;
			case 1: intersect++; hash[a][b] = 3; break;
			default: break;
		}
	}

	//clear hash[][]
	for(i = 0; i < cp; i++)
		hash[clear[0][i]][clear[1][i]] = 0;
	//if use this to clear the whole hash[][],
	//memset(hash, 0, sizeof(hash));
	//it is O(J_SIZE * J_SIZE), even more than O(m * n)

	//return jaccard index
	PG_RETURN_FLOAT8(intersect / join);
}