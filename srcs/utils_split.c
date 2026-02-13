#include "ssu_ext2.h"

static size_t	fix_memlen(char const *s, char c);
static char		*fix_splitalloc(char const **s, char c);
static void		fix_splitfree(char ***dest, size_t idx);

/**
 * @brief 주어진 일차원 배열을 쪼개 2차원 배열로 생성
 * 
 * @param s 일차원 배열
 * @param c 구분자
 * @return char** split된 결과 2차원 배열
 */
char	**fix_split(char const *s, char c)
{
	char	**dest;
	size_t	len;
	size_t	destidx;

	len = fix_memlen(s, c);
	dest = (char **)malloc((len + 1) * sizeof(char *));
	memset(dest, 0, (len + 1) * sizeof(char *));
	if (!dest)
		return (0);
	destidx = 0;
	while (destidx < len)
	{
		dest[destidx] = fix_splitalloc(&s, c);
		if (!dest[destidx])
		{
			fix_splitfree(&dest, destidx);
			return (0);
		}
		destidx++;
	}
	dest[destidx] = 0;
	return (dest);
}

void	fix_splitfree(char ***dest, size_t idx)
{
	size_t	m;

	m = 0;
	while (m < idx)
		free(*dest[m++]);
	free(*dest);
}

size_t	fix_strlcpy(char *dest, const char *src, size_t size)
{
	size_t	m;

	m = 0;
	while (*src && m + 1 < size)
		dest[m++] = *src++;
	if (size)
		dest[m] = '\0';
	return (m + strlen(src));
}

char	*fix_substr(char const *s, unsigned int start, size_t len)
{
	char	*dest;
	size_t	m;

	m = strlen(s);
	if (start >= m)
		return (strdup(""));
	if (len > m)
		len = m;
	dest = (char *)malloc((len + 1) * sizeof(char));
	memset(dest, 0, (len + 1) * sizeof(char));
	if (!dest)
		return (0);
	fix_strlcpy(dest, s + start, len + 1);
	return (dest);
}

char	*fix_splitalloc(char const **s, char c)
{
	size_t	m;
	char	*res;

	m = 0;
	while (**s && **s == c)
		(*s)++;
	while (*(*s + m) && *(*s + m) != c)
		m++;
	res = fix_substr(*s, 0, m);
	*s += m;
	return (res);
}

size_t	fix_memlen(char const *s, char c)
{
	size_t	m;

	m = 0;
	while (*s)
	{
		while (*s && *s == c)
			s++;
		if (*s)
		{
			while (*s && *s != c)
				s++;
			m++;
		}
	}
	return (m);
}
