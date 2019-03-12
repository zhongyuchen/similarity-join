/*-------------------------------------------------------------------------
 * oracle_compat.c
 *	Oracle compatible functions.
 *
 * Copyright (c) 1996-2017, PostgreSQL Global Development Group
 *
 *	Author: Edmund Mergl <E.Mergl@bawue.de>
 *	Multibyte enhancement: Tatsuo Ishii <ishii@postgresql.org>
 *
 *
 * IDENTIFICATION
 *	src/backend/utils/adt/oracle_compat.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "utils/builtins.h"
#include "utils/formatting.h"
#include "mb/pg_wchar.h"
#include <ctype.h>
#include <math.h>


static text *dotrim(const char *string, int stringlen,
	   const char *set, int setlen,
	   bool doltrim, bool dortrim);


/********************************************************************
 *
 * lower
 *
 * Syntax:
 *
 *	 text lower(text string)
 *
 * Purpose:
 *
 *	 Returns string, with all letters forced to lowercase.
 *
 ********************************************************************/

Datum
lower(PG_FUNCTION_ARGS)
{
	text	   *in_string = PG_GETARG_TEXT_PP(0);
	char	   *out_string;
	text	   *result;

	out_string = str_tolower(VARDATA_ANY(in_string),
							 VARSIZE_ANY_EXHDR(in_string),
							 PG_GET_COLLATION());
	result = cstring_to_text(out_string);
	pfree(out_string);

	PG_RETURN_TEXT_P(result);
}


/********************************************************************
 *
 * upper
 *
 * Syntax:
 *
 *	 text upper(text string)
 *
 * Purpose:
 *
 *	 Returns string, with all letters forced to uppercase.
 *
 ********************************************************************/

Datum
upper(PG_FUNCTION_ARGS)
{
	text	   *in_string = PG_GETARG_TEXT_PP(0);
	char	   *out_string;
	text	   *result;

	out_string = str_toupper(VARDATA_ANY(in_string),
							 VARSIZE_ANY_EXHDR(in_string),
							 PG_GET_COLLATION());
	result = cstring_to_text(out_string);
	pfree(out_string);

	PG_RETURN_TEXT_P(result);
}


/********************************************************************
 *
 * initcap
 *
 * Syntax:
 *
 *	 text initcap(text string)
 *
 * Purpose:
 *
 *	 Returns string, with first letter of each word in uppercase, all
 *	 other letters in lowercase. A word is defined as a sequence of
 *	 alphanumeric characters, delimited by non-alphanumeric
 *	 characters.
 *
 ********************************************************************/

Datum
initcap(PG_FUNCTION_ARGS)
{
	text	   *in_string = PG_GETARG_TEXT_PP(0);
	char	   *out_string;
	text	   *result;

	out_string = str_initcap(VARDATA_ANY(in_string),
							 VARSIZE_ANY_EXHDR(in_string),
							 PG_GET_COLLATION());
	result = cstring_to_text(out_string);
	pfree(out_string);

	PG_RETURN_TEXT_P(result);
}


/********************************************************************
 *
 * lpad
 *
 * Syntax:
 *
 *	 text lpad(text string1, int4 len, text string2)
 *
 * Purpose:
 *
 *	 Returns string1, left-padded to length len with the sequence of
 *	 characters in string2.  If len is less than the length of string1,
 *	 instead truncate (on the right) to len.
 *
 ********************************************************************/

Datum
lpad(PG_FUNCTION_ARGS)
{
	text	   *string1 = PG_GETARG_TEXT_PP(0);
	int32		len = PG_GETARG_INT32(1);
	text	   *string2 = PG_GETARG_TEXT_PP(2);
	text	   *ret;
	char	   *ptr1,
			   *ptr2,
			   *ptr2start,
			   *ptr2end,
			   *ptr_ret;
	int			m,
				s1len,
				s2len;

	int			bytelen;

	/* Negative len is silently taken as zero */
	if (len < 0)
		len = 0;

	s1len = VARSIZE_ANY_EXHDR(string1);
	if (s1len < 0)
		s1len = 0;				/* shouldn't happen */

	s2len = VARSIZE_ANY_EXHDR(string2);
	if (s2len < 0)
		s2len = 0;				/* shouldn't happen */

	s1len = pg_mbstrlen_with_len(VARDATA_ANY(string1), s1len);

	if (s1len > len)
		s1len = len;			/* truncate string1 to len chars */

	if (s2len <= 0)
		len = s1len;			/* nothing to pad with, so don't pad */

	bytelen = pg_database_encoding_max_length() * len;

	/* check for integer overflow */
	if (len != 0 && bytelen / pg_database_encoding_max_length() != len)
		ereport(ERROR,
				(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
				 errmsg("requested length too large")));

	ret = (text *) palloc(VARHDRSZ + bytelen);

	m = len - s1len;

	ptr2 = ptr2start = VARDATA_ANY(string2);
	ptr2end = ptr2 + s2len;
	ptr_ret = VARDATA(ret);

	while (m--)
	{
		int			mlen = pg_mblen(ptr2);

		memcpy(ptr_ret, ptr2, mlen);
		ptr_ret += mlen;
		ptr2 += mlen;
		if (ptr2 == ptr2end)	/* wrap around at end of s2 */
			ptr2 = ptr2start;
	}

	ptr1 = VARDATA_ANY(string1);

	while (s1len--)
	{
		int			mlen = pg_mblen(ptr1);

		memcpy(ptr_ret, ptr1, mlen);
		ptr_ret += mlen;
		ptr1 += mlen;
	}

	SET_VARSIZE(ret, ptr_ret - (char *) ret);

	PG_RETURN_TEXT_P(ret);
}


/********************************************************************
 *
 * rpad
 *
 * Syntax:
 *
 *	 text rpad(text string1, int4 len, text string2)
 *
 * Purpose:
 *
 *	 Returns string1, right-padded to length len with the sequence of
 *	 characters in string2.  If len is less than the length of string1,
 *	 instead truncate (on the right) to len.
 *
 ********************************************************************/

Datum
rpad(PG_FUNCTION_ARGS)
{
	text	   *string1 = PG_GETARG_TEXT_PP(0);
	int32		len = PG_GETARG_INT32(1);
	text	   *string2 = PG_GETARG_TEXT_PP(2);
	text	   *ret;
	char	   *ptr1,
			   *ptr2,
			   *ptr2start,
			   *ptr2end,
			   *ptr_ret;
	int			m,
				s1len,
				s2len;

	int			bytelen;

	/* Negative len is silently taken as zero */
	if (len < 0)
		len = 0;

	s1len = VARSIZE_ANY_EXHDR(string1);
	if (s1len < 0)
		s1len = 0;				/* shouldn't happen */

	s2len = VARSIZE_ANY_EXHDR(string2);
	if (s2len < 0)
		s2len = 0;				/* shouldn't happen */

	s1len = pg_mbstrlen_with_len(VARDATA_ANY(string1), s1len);

	if (s1len > len)
		s1len = len;			/* truncate string1 to len chars */

	if (s2len <= 0)
		len = s1len;			/* nothing to pad with, so don't pad */

	bytelen = pg_database_encoding_max_length() * len;

	/* Check for integer overflow */
	if (len != 0 && bytelen / pg_database_encoding_max_length() != len)
		ereport(ERROR,
				(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
				 errmsg("requested length too large")));

	ret = (text *) palloc(VARHDRSZ + bytelen);
	m = len - s1len;

	ptr1 = VARDATA_ANY(string1);
	ptr_ret = VARDATA(ret);

	while (s1len--)
	{
		int			mlen = pg_mblen(ptr1);

		memcpy(ptr_ret, ptr1, mlen);
		ptr_ret += mlen;
		ptr1 += mlen;
	}

	ptr2 = ptr2start = VARDATA_ANY(string2);
	ptr2end = ptr2 + s2len;

	while (m--)
	{
		int			mlen = pg_mblen(ptr2);

		memcpy(ptr_ret, ptr2, mlen);
		ptr_ret += mlen;
		ptr2 += mlen;
		if (ptr2 == ptr2end)	/* wrap around at end of s2 */
			ptr2 = ptr2start;
	}

	SET_VARSIZE(ret, ptr_ret - (char *) ret);

	PG_RETURN_TEXT_P(ret);
}


/********************************************************************
 *
 * btrim
 *
 * Syntax:
 *
 *	 text btrim(text string, text set)
 *
 * Purpose:
 *
 *	 Returns string with characters removed from the front and back
 *	 up to the first character not in set.
 *
 ********************************************************************/

Datum
btrim(PG_FUNCTION_ARGS)
{
	text	   *string = PG_GETARG_TEXT_PP(0);
	text	   *set = PG_GETARG_TEXT_PP(1);
	text	   *ret;

	ret = dotrim(VARDATA_ANY(string), VARSIZE_ANY_EXHDR(string),
				 VARDATA_ANY(set), VARSIZE_ANY_EXHDR(set),
				 true, true);

	PG_RETURN_TEXT_P(ret);
}

/********************************************************************
 *
 * btrim1 --- btrim with set fixed as ' '
 *
 ********************************************************************/

Datum
btrim1(PG_FUNCTION_ARGS)
{
	text	   *string = PG_GETARG_TEXT_PP(0);
	text	   *ret;

	ret = dotrim(VARDATA_ANY(string), VARSIZE_ANY_EXHDR(string),
				 " ", 1,
				 true, true);

	PG_RETURN_TEXT_P(ret);
}

/*
 * Common implementation for btrim, ltrim, rtrim
 */
static text *
dotrim(const char *string, int stringlen,
	   const char *set, int setlen,
	   bool doltrim, bool dortrim)
{
	int			i;

	/* Nothing to do if either string or set is empty */
	if (stringlen > 0 && setlen > 0)
	{
		if (pg_database_encoding_max_length() > 1)
		{
			/*
			 * In the multibyte-encoding case, build arrays of pointers to
			 * character starts, so that we can avoid inefficient checks in
			 * the inner loops.
			 */
			const char **stringchars;
			const char **setchars;
			int		   *stringmblen;
			int		   *setmblen;
			int			stringnchars;
			int			setnchars;
			int			resultndx;
			int			resultnchars;
			const char *p;
			int			len;
			int			mblen;
			const char *str_pos;
			int			str_len;

			stringchars = (const char **) palloc(stringlen * sizeof(char *));
			stringmblen = (int *) palloc(stringlen * sizeof(int));
			stringnchars = 0;
			p = string;
			len = stringlen;
			while (len > 0)
			{
				stringchars[stringnchars] = p;
				stringmblen[stringnchars] = mblen = pg_mblen(p);
				stringnchars++;
				p += mblen;
				len -= mblen;
			}

			setchars = (const char **) palloc(setlen * sizeof(char *));
			setmblen = (int *) palloc(setlen * sizeof(int));
			setnchars = 0;
			p = set;
			len = setlen;
			while (len > 0)
			{
				setchars[setnchars] = p;
				setmblen[setnchars] = mblen = pg_mblen(p);
				setnchars++;
				p += mblen;
				len -= mblen;
			}

			resultndx = 0;		/* index in stringchars[] */
			resultnchars = stringnchars;

			if (doltrim)
			{
				while (resultnchars > 0)
				{
					str_pos = stringchars[resultndx];
					str_len = stringmblen[resultndx];
					for (i = 0; i < setnchars; i++)
					{
						if (str_len == setmblen[i] &&
							memcmp(str_pos, setchars[i], str_len) == 0)
							break;
					}
					if (i >= setnchars)
						break;	/* no match here */
					string += str_len;
					stringlen -= str_len;
					resultndx++;
					resultnchars--;
				}
			}

			if (dortrim)
			{
				while (resultnchars > 0)
				{
					str_pos = stringchars[resultndx + resultnchars - 1];
					str_len = stringmblen[resultndx + resultnchars - 1];
					for (i = 0; i < setnchars; i++)
					{
						if (str_len == setmblen[i] &&
							memcmp(str_pos, setchars[i], str_len) == 0)
							break;
					}
					if (i >= setnchars)
						break;	/* no match here */
					stringlen -= str_len;
					resultnchars--;
				}
			}

			pfree(stringchars);
			pfree(stringmblen);
			pfree(setchars);
			pfree(setmblen);
		}
		else
		{
			/*
			 * In the single-byte-encoding case, we don't need such overhead.
			 */
			if (doltrim)
			{
				while (stringlen > 0)
				{
					char		str_ch = *string;

					for (i = 0; i < setlen; i++)
					{
						if (str_ch == set[i])
							break;
					}
					if (i >= setlen)
						break;	/* no match here */
					string++;
					stringlen--;
				}
			}

			if (dortrim)
			{
				while (stringlen > 0)
				{
					char		str_ch = string[stringlen - 1];

					for (i = 0; i < setlen; i++)
					{
						if (str_ch == set[i])
							break;
					}
					if (i >= setlen)
						break;	/* no match here */
					stringlen--;
				}
			}
		}
	}

	/* Return selected portion of string */
	return cstring_to_text_with_len(string, stringlen);
}

/********************************************************************
 *
 * byteatrim
 *
 * Syntax:
 *
 *	 bytea byteatrim(byta string, bytea set)
 *
 * Purpose:
 *
 *	 Returns string with characters removed from the front and back
 *	 up to the first character not in set.
 *
 * Cloned from btrim and modified as required.
 ********************************************************************/

Datum
byteatrim(PG_FUNCTION_ARGS)
{
	bytea	   *string = PG_GETARG_BYTEA_PP(0);
	bytea	   *set = PG_GETARG_BYTEA_PP(1);
	bytea	   *ret;
	char	   *ptr,
			   *end,
			   *ptr2,
			   *ptr2start,
			   *end2;
	int			m,
				stringlen,
				setlen;

	stringlen = VARSIZE_ANY_EXHDR(string);
	setlen = VARSIZE_ANY_EXHDR(set);

	if (stringlen <= 0 || setlen <= 0)
		PG_RETURN_BYTEA_P(string);

	m = stringlen;
	ptr = VARDATA_ANY(string);
	end = ptr + stringlen - 1;
	ptr2start = VARDATA_ANY(set);
	end2 = ptr2start + setlen - 1;

	while (m > 0)
	{
		ptr2 = ptr2start;
		while (ptr2 <= end2)
		{
			if (*ptr == *ptr2)
				break;
			++ptr2;
		}
		if (ptr2 > end2)
			break;
		ptr++;
		m--;
	}

	while (m > 0)
	{
		ptr2 = ptr2start;
		while (ptr2 <= end2)
		{
			if (*end == *ptr2)
				break;
			++ptr2;
		}
		if (ptr2 > end2)
			break;
		end--;
		m--;
	}

	ret = (bytea *) palloc(VARHDRSZ + m);
	SET_VARSIZE(ret, VARHDRSZ + m);
	memcpy(VARDATA(ret), ptr, m);

	PG_RETURN_BYTEA_P(ret);
}

/********************************************************************
 *
 * ltrim
 *
 * Syntax:
 *
 *	 text ltrim(text string, text set)
 *
 * Purpose:
 *
 *	 Returns string with initial characters removed up to the first
 *	 character not in set.
 *
 ********************************************************************/

Datum
ltrim(PG_FUNCTION_ARGS)
{
	text	   *string = PG_GETARG_TEXT_PP(0);
	text	   *set = PG_GETARG_TEXT_PP(1);
	text	   *ret;

	ret = dotrim(VARDATA_ANY(string), VARSIZE_ANY_EXHDR(string),
				 VARDATA_ANY(set), VARSIZE_ANY_EXHDR(set),
				 true, false);

	PG_RETURN_TEXT_P(ret);
}

/********************************************************************
 *
 * ltrim1 --- ltrim with set fixed as ' '
 *
 ********************************************************************/

Datum
ltrim1(PG_FUNCTION_ARGS)
{
	text	   *string = PG_GETARG_TEXT_PP(0);
	text	   *ret;

	ret = dotrim(VARDATA_ANY(string), VARSIZE_ANY_EXHDR(string),
				 " ", 1,
				 true, false);

	PG_RETURN_TEXT_P(ret);
}

/********************************************************************
 *
 * rtrim
 *
 * Syntax:
 *
 *	 text rtrim(text string, text set)
 *
 * Purpose:
 *
 *	 Returns string with final characters removed after the last
 *	 character not in set.
 *
 ********************************************************************/

Datum
rtrim(PG_FUNCTION_ARGS)
{
	text	   *string = PG_GETARG_TEXT_PP(0);
	text	   *set = PG_GETARG_TEXT_PP(1);
	text	   *ret;

	ret = dotrim(VARDATA_ANY(string), VARSIZE_ANY_EXHDR(string),
				 VARDATA_ANY(set), VARSIZE_ANY_EXHDR(set),
				 false, true);

	PG_RETURN_TEXT_P(ret);
}

/********************************************************************
 *
 * rtrim1 --- rtrim with set fixed as ' '
 *
 ********************************************************************/

Datum
rtrim1(PG_FUNCTION_ARGS)
{
	text	   *string = PG_GETARG_TEXT_PP(0);
	text	   *ret;

	ret = dotrim(VARDATA_ANY(string), VARSIZE_ANY_EXHDR(string),
				 " ", 1,
				 false, true);

	PG_RETURN_TEXT_P(ret);
}


/********************************************************************
 *
 * translate
 *
 * Syntax:
 *
 *	 text translate(text string, text from, text to)
 *
 * Purpose:
 *
 *	 Returns string after replacing all occurrences of characters in from
 *	 with the corresponding character in to.  If from is longer than to,
 *	 occurrences of the extra characters in from are deleted.
 *	 Improved by Edwin Ramirez <ramirez@doc.mssm.edu>.
 *
 ********************************************************************/

Datum
translate(PG_FUNCTION_ARGS)
{
	text	   *string = PG_GETARG_TEXT_PP(0);
	text	   *from = PG_GETARG_TEXT_PP(1);
	text	   *to = PG_GETARG_TEXT_PP(2);
	text	   *result;
	char	   *from_ptr,
			   *to_ptr;
	char	   *source,
			   *target;
	int			m,
				fromlen,
				tolen,
				retlen,
				i;
	int			worst_len;
	int			len;
	int			source_len;
	int			from_index;

	m = VARSIZE_ANY_EXHDR(string);
	if (m <= 0)
		PG_RETURN_TEXT_P(string);
	source = VARDATA_ANY(string);

	fromlen = VARSIZE_ANY_EXHDR(from);
	from_ptr = VARDATA_ANY(from);
	tolen = VARSIZE_ANY_EXHDR(to);
	to_ptr = VARDATA_ANY(to);

	/*
	 * The worst-case expansion is to substitute a max-length character for a
	 * single-byte character at each position of the string.
	 */
	worst_len = pg_database_encoding_max_length() * m;

	/* check for integer overflow */
	if (worst_len / pg_database_encoding_max_length() != m)
		ereport(ERROR,
				(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
				 errmsg("requested length too large")));

	result = (text *) palloc(worst_len + VARHDRSZ);
	target = VARDATA(result);
	retlen = 0;

	while (m > 0)
	{
		source_len = pg_mblen(source);
		from_index = 0;

		for (i = 0; i < fromlen; i += len)
		{
			len = pg_mblen(&from_ptr[i]);
			if (len == source_len &&
				memcmp(source, &from_ptr[i], len) == 0)
				break;

			from_index++;
		}
		if (i < fromlen)
		{
			/* substitute */
			char	   *p = to_ptr;

			for (i = 0; i < from_index; i++)
			{
				p += pg_mblen(p);
				if (p >= (to_ptr + tolen))
					break;
			}
			if (p < (to_ptr + tolen))
			{
				len = pg_mblen(p);
				memcpy(target, p, len);
				target += len;
				retlen += len;
			}

		}
		else
		{
			/* no match, so copy */
			memcpy(target, source, source_len);
			target += source_len;
			retlen += source_len;
		}

		source += source_len;
		m -= source_len;
	}

	SET_VARSIZE(result, retlen + VARHDRSZ);

	/*
	 * The function result is probably much bigger than needed, if we're using
	 * a multibyte encoding, but it's not worth reallocating it; the result
	 * probably won't live long anyway.
	 */

	PG_RETURN_TEXT_P(result);
}

/********************************************************************
 *
 * ascii
 *
 * Syntax:
 *
 *	 int ascii(text string)
 *
 * Purpose:
 *
 *	 Returns the decimal representation of the first character from
 *	 string.
 *	 If the string is empty we return 0.
 *	 If the database encoding is UTF8, we return the Unicode codepoint.
 *	 If the database encoding is any other multi-byte encoding, we
 *	 return the value of the first byte if it is an ASCII character
 *	 (range 1 .. 127), or raise an error.
 *	 For all other encodings we return the value of the first byte,
 *	 (range 1..255).
 *
 ********************************************************************/

Datum
ascii(PG_FUNCTION_ARGS)
{
	text	   *string = PG_GETARG_TEXT_PP(0);
	int			encoding = GetDatabaseEncoding();
	unsigned char *data;

	if (VARSIZE_ANY_EXHDR(string) <= 0)
		PG_RETURN_INT32(0);

	data = (unsigned char *) VARDATA_ANY(string);

	if (encoding == PG_UTF8 && *data > 127)
	{
		/* return the code point for Unicode */

		int			result = 0,
					tbytes = 0,
					i;

		if (*data >= 0xF0)
		{
			result = *data & 0x07;
			tbytes = 3;
		}
		else if (*data >= 0xE0)
		{
			result = *data & 0x0F;
			tbytes = 2;
		}
		else
		{
			Assert(*data > 0xC0);
			result = *data & 0x1f;
			tbytes = 1;
		}

		Assert(tbytes > 0);

		for (i = 1; i <= tbytes; i++)
		{
			Assert((data[i] & 0xC0) == 0x80);
			result = (result << 6) + (data[i] & 0x3f);
		}

		PG_RETURN_INT32(result);
	}
	else
	{
		if (pg_encoding_max_length(encoding) > 1 && *data > 127)
			ereport(ERROR,
					(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
					 errmsg("requested character too large")));


		PG_RETURN_INT32((int32) *data);
	}
}

/********************************************************************
 *
 * chr
 *
 * Syntax:
 *
 *	 text chr(int val)
 *
 * Purpose:
 *
 *	Returns the character having the binary equivalent to val.
 *
 * For UTF8 we treat the argumwent as a Unicode code point.
 * For other multi-byte encodings we raise an error for arguments
 * outside the strict ASCII range (1..127).
 *
 * It's important that we don't ever return a value that is not valid
 * in the database encoding, so that this doesn't become a way for
 * invalid data to enter the database.
 *
 ********************************************************************/

Datum
chr			(PG_FUNCTION_ARGS)
{
	uint32		cvalue = PG_GETARG_UINT32(0);
	text	   *result;
	int			encoding = GetDatabaseEncoding();

	if (encoding == PG_UTF8 && cvalue > 127)
	{
		/* for Unicode we treat the argument as a code point */
		int			bytes;
		unsigned char *wch;

		/*
		 * We only allow valid Unicode code points; per RFC3629 that stops at
		 * U+10FFFF, even though 4-byte UTF8 sequences can hold values up to
		 * U+1FFFFF.
		 */
		if (cvalue > 0x0010ffff)
			ereport(ERROR,
					(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
					 errmsg("requested character too large for encoding: %d",
							cvalue)));

		if (cvalue > 0xffff)
			bytes = 4;
		else if (cvalue > 0x07ff)
			bytes = 3;
		else
			bytes = 2;

		result = (text *) palloc(VARHDRSZ + bytes);
		SET_VARSIZE(result, VARHDRSZ + bytes);
		wch = (unsigned char *) VARDATA(result);

		if (bytes == 2)
		{
			wch[0] = 0xC0 | ((cvalue >> 6) & 0x1F);
			wch[1] = 0x80 | (cvalue & 0x3F);
		}
		else if (bytes == 3)
		{
			wch[0] = 0xE0 | ((cvalue >> 12) & 0x0F);
			wch[1] = 0x80 | ((cvalue >> 6) & 0x3F);
			wch[2] = 0x80 | (cvalue & 0x3F);
		}
		else
		{
			wch[0] = 0xF0 | ((cvalue >> 18) & 0x07);
			wch[1] = 0x80 | ((cvalue >> 12) & 0x3F);
			wch[2] = 0x80 | ((cvalue >> 6) & 0x3F);
			wch[3] = 0x80 | (cvalue & 0x3F);
		}

		/*
		 * The preceding range check isn't sufficient, because UTF8 excludes
		 * Unicode "surrogate pair" codes.  Make sure what we created is valid
		 * UTF8.
		 */
		if (!pg_utf8_islegal(wch, bytes))
			ereport(ERROR,
					(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
					 errmsg("requested character not valid for encoding: %d",
							cvalue)));
	}
	else
	{
		bool		is_mb;

		/*
		 * Error out on arguments that make no sense or that we can't validly
		 * represent in the encoding.
		 */
		if (cvalue == 0)
			ereport(ERROR,
					(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
					 errmsg("null character not permitted")));

		is_mb = pg_encoding_max_length(encoding) > 1;

		if ((is_mb && (cvalue > 127)) || (!is_mb && (cvalue > 255)))
			ereport(ERROR,
					(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
					 errmsg("requested character too large for encoding: %d",
							cvalue)));

		result = (text *) palloc(VARHDRSZ + 1);
		SET_VARSIZE(result, VARHDRSZ + 1);
		*VARDATA(result) = (char) cvalue;
	}

	PG_RETURN_TEXT_P(result);
}

/********************************************************************
 *
 * repeat
 *
 * Syntax:
 *
 *	 text repeat(text string, int val)
 *
 * Purpose:
 *
 *	Repeat string by val.
 *
 ********************************************************************/

Datum
repeat(PG_FUNCTION_ARGS)
{
	text	   *string = PG_GETARG_TEXT_PP(0);
	int32		count = PG_GETARG_INT32(1);
	text	   *result;
	int			slen,
				tlen;
	int			i;
	char	   *cp,
			   *sp;

	if (count < 0)
		count = 0;

	slen = VARSIZE_ANY_EXHDR(string);
	tlen = VARHDRSZ + (count * slen);

	/* Check for integer overflow */
	if (slen != 0 && count != 0)
	{
		int			check = count * slen;
		int			check2 = check + VARHDRSZ;

		if ((check / slen) != count || check2 <= check)
			ereport(ERROR,
					(errcode(ERRCODE_PROGRAM_LIMIT_EXCEEDED),
					 errmsg("requested length too large")));
	}

	result = (text *) palloc(tlen);

	SET_VARSIZE(result, tlen);
	cp = VARDATA(result);
	sp = VARDATA_ANY(string);
	for (i = 0; i < count; i++)
	{
		memcpy(cp, sp, slen);
		cp += slen;
	}

	PG_RETURN_TEXT_P(result);
}


//修改开始
Datum
jaccard_index(PG_FUNCTION_ARGS)
{
	text       *string1=PG_GETARG_TEXT_PP(0);
    text       *string2=PG_GETARG_TEXT_PP(1);
	char	   *s1 = VARDATA_ANY(string1);
	char	   *s2 = VARDATA_ANY(string2);
	float4	   result;
	int        lenth1,lenth2;
	int        same=0,cnt=0;
	int        i,tmp;
	static int flag[16384];
	static int change[256];
	lenth1 = VARSIZE_ANY_EXHDR(string1);
	lenth2 = VARSIZE_ANY_EXHDR(string2);
	if(lenth1&&lenth2)
	{
        tmp=(36<<7)|toupper(s1[0]);
        flag[tmp]=1;
        change[cnt++]=tmp;
        for(i=1;i<lenth1;i++)
        {
            tmp=((tmp%128)<<7)|toupper(s1[i]);
            if(!flag[tmp])
            {
                flag[tmp]=1;
                change[cnt++]=tmp;
            }
        }
        tmp=((tmp%128)<<7)|36;
        if(!flag[tmp])
        {
            flag[tmp]=1;
            change[cnt++]=tmp;
        }

        tmp=(36<<7)|toupper(s2[0]);
        if(!flag[tmp]) change[cnt++]=tmp;
        else same++;
        flag[tmp]=2;
        for(i=1;i<lenth2;i++)
        {
            tmp=((tmp%128)<<7)|toupper(s2[i]);
            if(!flag[tmp]) change[cnt++]=tmp;
            else if(flag[tmp]==1) same++;
            flag[tmp]=2;
        }
        tmp=((tmp%128)<<7)|36;
        if(!flag[tmp]) change[cnt++]=tmp;
        else if(flag[tmp]==1) same++;
	}
	for(i=0;i<cnt;i++) flag[change[i]]=0;
    result=(float4)same/cnt;
	PG_RETURN_FLOAT4(result);
}

//levenshtein_distance（未优化）
/*
Datum
levenshtein_distance(PG_FUNCTION_ARGS)
{
    text       *string1=PG_GETARG_TEXT_PP(0);
    text       *string2=PG_GETARG_TEXT_PP(1);
	char	   *s1 = VARDATA_ANY(string1);
	char	   *s2 = VARDATA_ANY(string2);
	int32	   result;
	int        i,j;
    int        **distance;
	int        lenth1,lenth2;
	int        mini;
	lenth1 = VARSIZE_ANY_EXHDR(string1);
	lenth2 = VARSIZE_ANY_EXHDR(string2);
	distance = (int **)palloc ((lenth1+1)*sizeof(int *));
	for(i=0;i<=lenth1;i++)
        distance[i]=(int *)palloc ((lenth2+1)*sizeof(int));
    for(i=0;i<=lenth1;i++)
        distance[i][0]=i;
    for(j=0;j<=lenth2;j++)
        distance[0][j]=j;

    for(i=1;i<=lenth1;i++)
        for(j=1;j<=lenth2;j++)
        {
            if(toupper(s1[i-1])==toupper(s2[j-1])) distance[i][j]=distance[i-1][j-1];
            else
            {
                mini=distance[i][j-1]<distance[i-1][j]?distance[i][j-1]:distance[i-1][j];
                mini=mini<distance[i-1][j-1]?mini:distance[i-1][j-1];
                distance[i][j]=mini+1;
            }

        }
    result=distance[lenth1][lenth2];
    for(i=0;i<=lenth1;i++)
        pfree(distance[i]);
    pfree(distance);
	PG_RETURN_INT32(result);
}
*/

//levenshtein_distance（去掉相同前缀及后缀）

Datum
levenshtein_distance(PG_FUNCTION_ARGS)
{
    text       *string1=PG_GETARG_TEXT_PP(0);
    text       *string2=PG_GETARG_TEXT_PP(1);
	const char *str1 = VARDATA_ANY(string1);
	const char *str2 = VARDATA_ANY(string2);
	int32	   result;
	int        i,j;
    static int distance[120][120];
    static char s1[120],s2[120];
	int        lenth1,lenth2;
	int        mini;
	int        small,big;
	bool       flag;
	lenth1 = VARSIZE_ANY_EXHDR(string1);
	lenth2 = VARSIZE_ANY_EXHDR(string2);
	flag=(lenth1<lenth2);
	if(flag)
	{
        small=lenth1;
        big=lenth2;
	}
	else
	{
        small=lenth2;
        big=lenth1;
	}
    lenth1=lenth2=0;
	for(i=0;i<small;i++)
        if(toupper(str1[i])!=toupper(str2[i]))
           break;
    for(;i<small;i++)
    {
        s1[lenth1++]=str1[i];
        s2[lenth2++]=str2[i];
    }
    if(flag)
    {
        for(i=small;i<big;i++)
            s2[lenth2++]=str2[i];
    }
    else
    {
        for(i=small;i<big;i++)
            s1[lenth1++]=str1[i];
    }

    for(i=lenth1,j=lenth2;i>0&&j>0;i--,j--)
    {
        if(toupper(s1[i-1])==toupper(s2[j-1]))
        {
            lenth1--;
            lenth2--;
        }
        else break;
    }

    for(i=0;i<=lenth1;i++)
        distance[i][0]=i;
    for(j=0;j<=lenth2;j++)
        distance[0][j]=j;

    for(i=1;i<=lenth1;i++)
        for(j=1;j<=lenth2;j++)
        {
            if(toupper(s1[i-1])==toupper(s2[j-1])) distance[i][j]=distance[i-1][j-1];
            else
            {
                mini=distance[i][j-1]<distance[i-1][j]?distance[i][j-1]:distance[i-1][j];
                mini=mini<distance[i-1][j-1]?mini:distance[i-1][j-1];
                distance[i][j]=mini+1;
            }

        }
    result=distance[lenth1][lenth2];
    PG_RETURN_INT32(result);
}

//levenshtein_distance（利用上一次的编辑距离矩阵）
/*
Datum
levenshtein_distance(PG_FUNCTION_ARGS)
{
    text       *string1=PG_GETARG_TEXT_PP(0);
    text       *string2=PG_GETARG_TEXT_PP(1);
	const char *s1 = VARDATA_ANY(string1);
	const char *s2 = VARDATA_ANY(string2);
	int32	   result;
	int        i,j;
    static int distance[120][120];
    static char olds1[120],olds2[120];
    static int  oldlenth1=0,oldlenth2=0;
    int        start1=0,start2=0;
	int        lenth1,lenth2;
	int        mini;
	lenth1 = VARSIZE_ANY_EXHDR(string1);
	lenth2 = VARSIZE_ANY_EXHDR(string2);
    oldlenth1=oldlenth1<lenth1?oldlenth1:lenth1;
    oldlenth2=oldlenth2<lenth2?oldlenth2:lenth2;
	for(i=0;i<oldlenth1;i++)
	{
        if(toupper(s1[i])==toupper(olds1[i])) start1++;
        else break;
	}
    for(j=0;j<oldlenth2;j++)
    {
        if(toupper(s2[j])==toupper(olds2[j])) start2++;
        else break;
    }

    for(i=1;i<=lenth1;i++)
        distance[i][0]=i;
    for(j=1;j<=lenth2;j++)
        distance[0][j]=j;

    for(i=1;i<=start1;i++)
        for(j=start2+1;j<=lenth2;j++)
        {
            if(toupper(s1[i-1])==toupper(s2[j-1])) distance[i][j]=distance[i-1][j-1];
            else
            {
                mini=distance[i][j-1]<distance[i-1][j]?distance[i][j-1]:distance[i-1][j];
                mini=mini<distance[i-1][j-1]?mini:distance[i-1][j-1];
                distance[i][j]=mini+1;
            }

        }

    for(i=start1+1;i<=lenth1;i++)
        for(j=1;j<=start2;j++)
        {
            if(toupper(s1[i-1])==toupper(s2[j-1])) distance[i][j]=distance[i-1][j-1];
            else
            {
                mini=distance[i][j-1]<distance[i-1][j]?distance[i][j-1]:distance[i-1][j];
                mini=mini<distance[i-1][j-1]?mini:distance[i-1][j-1];
                distance[i][j]=mini+1;
            }

        }

    for(i=start1+1;i<=lenth1;i++)
        for(j=start2+1;j<=lenth2;j++)
        {
            if(toupper(s1[i-1])==toupper(s2[j-1])) distance[i][j]=distance[i-1][j-1];
            else
            {
                mini=distance[i][j-1]<distance[i-1][j]?distance[i][j-1]:distance[i-1][j];
                mini=mini<distance[i-1][j-1]?mini:distance[i-1][j-1];
                distance[i][j]=mini+1;
            }

        }
    for(i=0;i<lenth1;i++)
        olds1[i]=s1[i];
    for(j=0;j<lenth2;j++)
        olds2[j]=s2[j];
    olds1[lenth1]=olds2[lenth2]='\0';
    oldlenth1=lenth1;
    oldlenth2=lenth2;
    result=distance[lenth1][lenth2];
    PG_RETURN_INT32(result);
}
*/

Datum
levenshtein_distance_2(PG_FUNCTION_ARGS)
{
    text       *string1=PG_GETARG_TEXT_PP(0);
    text       *string2=PG_GETARG_TEXT_PP(1);
    int        d=PG_GETARG_INT32(2)-1;
	const char *str1 = VARDATA_ANY(string1);
	const char *str2 = VARDATA_ANY(string2);
	bool	   result;
	int        i,j;
    static int distance[120][120];
	int        lenth1,lenth2,L,R,KL,KR;
	lenth1=VARSIZE_ANY_EXHDR(string1);
	lenth2=VARSIZE_ANY_EXHDR(string2);
	if(abs(lenth1-lenth2)>d) PG_RETURN_BOOL(false);

    for(i=0;i<=lenth1;i++)
        distance[i][0]=i;
    for(j=0;j<=lenth2;j++)
        distance[0][j]=j;

    KL=-((d-abs(lenth1-lenth2))/2);
    KR=(d-abs(lenth1-lenth2))/2;
    if (lenth1<=lenth2)
        KL+=lenth1-lenth2;
    else
        KR+=lenth1-lenth2;

    for (i=1;i<=lenth1;i++) {
        L=1>i-KR?1:i-KR;
        R=lenth2<i-KL?lenth2:i-KL;
        for (j=L;j<=R;j++) {
            distance[i][j] = distance[i-1][j-1]+(toupper(str1[i-1])==toupper(str2[j-1])?0:1);
            if (KL<=(i-1)-j&&(i-1)-j<=KR)
                distance[i][j]=distance[i][j]<distance[i-1][j]+1?distance[i][j]:distance[i-1][j]+1;
            if (KL<=i-(j-1)&&i-(j-1)<=KR)
                distance[i][j]=distance[i][j]<distance[i][j-1]+1?distance[i][j]:distance[i][j-1]+1;
        }
    }
    result=distance[lenth1][lenth2]<=d;
    PG_RETURN_BOOL(result);
}






