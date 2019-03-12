
//collum partition method
#define P_SIZE 128

Datum levenshtein_distance(PG_FUNCTION_ARGS) 
{
    //strict function, no need to check for null inputs

  //get arguments
  text *src = PG_GETARG_TEXT_PP(0);
  text *dst = PG_GETARG_TEXT_PP(1);

  char *s_data;
  char *t_data;
  int s_bytes,t_bytes;
  int i, j,k;

  //define pointers
  s_data = VARDATA_ANY(src);
  t_data = VARDATA_ANY(dst);

  //length of each string(bytes)
  s_bytes = VARSIZE_ANY_EXHDR(src);
  t_bytes = VARSIZE_ANY_EXHDR(dst);

  if(s_bytes<=t_bytes)
  {
  	s_data = VARDATA_ANY(dst);
  	t_data = VARDATA_ANY(src);
  	i=s_bytes;
  	s_bytes=t_bytes;
  	t_bytes=i;
  }	  



  int find[P_SIZE][1000];
  int length[P_SIZE] = {0};
  int part[1000];  
  int l, b, e;
  int temp, temp_value;

  for(i = 0; i < s_bytes; i++)
    find[s_data[i]][length[s_data[i]]++] = i + 1;
  
  int pre_cn = 0, next_cn = 1, min_v = s_bytes;
  int pre_v;
  part[0] = s_bytes;
  
  for(i = 0; i < t_bytes; ++i) {
    pre_cn = next_cn;
    next_cn = 0;
    l = part[0] + 1;
    b = 1;
    e = l;
    temp_value = 0;
    pre_v = part[0];

      temp=-1;
      for(k = 0; k< length[t_data[i]];k++) 
      {
          if(find[t_data[i]][k] >= b)  
          {
            temp=find[t_data[i]][k];
            break;
          }  
       }

  if(length[t_data[i]] >0 && temp!=-1 && temp <= e) 
      part[next_cn++] = temp - 1;
	else if(pre_cn >= 2 && part[1] - part[0] != 0)
	{
      part[next_cn++] = part[0] + 1;
    } 
	else 
	{
      part[next_cn++] = part[0];
    }
    temp_value = part[0];

    for(j = 1; j < pre_cn && part[next_cn - 1] < s_bytes; ++j) {
      int x = part[j], y = pre_v;
      pre_v = part[j];
      l = x - y;
      if(l == 0) 
	  {
        part[next_cn++] = x + 1;
      } 
	  else 
	  {
        b = x - l + 2;
        e = x + 1;

              temp=-1;
      for(k = 0; k < length[t_data[i]]; k++) 
      {
          if(find[t_data[i]][k] >= b)  
          {
            temp=find[t_data[i]][k];
            break;
          }  
       }

        if(b <= s_bytes && length[t_data[i]] > 0 && temp != -1 && temp <= e) {
          part[next_cn++] = temp - 1;
        } 
		else if(j + 1 < pre_cn && part[j + 1] - x != 0) 
		{
          part[next_cn++] = x + 1;
        } else 
		{
          part[next_cn++] = x;
        }
      }
      l = part[j] - part[j - 1];
      if(l == 0) {
       temp_value -= 1;
      } else {
        temp_value += l - 1;
      }
    }
    
    if(part[next_cn - 1] != s_bytes) 
    {
      part[next_cn++] = s_bytes;  
      temp_value += s_bytes - part[next_cn - 2] - 1;
      //if(temp_value < min_v) {
        min_v = temp_value;
      //}
    } else {
      //min_v = (min_v < temp_value) ? min_v : temp_value;
    	min_v = temp_value;
    }
  }

  PG_RETURN_INT32(min_v);
}
