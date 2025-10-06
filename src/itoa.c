#include "cnake.h"

static size_t	count_digits(int integer)
{
	size_t	digits;

	digits = 1;
	if (integer < 0) ++digits;
	while (integer /= 10)
		++digits;
	return (digits);
}

static void	insert_digit(char *result, int integer, size_t digits)
{
	if (integer == 0) {
		result[0] = '0';
		result[1] = '\0';
		return ;
	} 
	while (integer) {
		result[--digits] = integer % 10 + '0';
		integer /= 10;
	}
}

char	*itoa(int integer)
{
	char	*result;
	size_t	digits;

	digits = count_digits(integer);
	result = malloc((digits + 1) * sizeof(char));
	if (!result) return (NULL);
	if (integer < 0) {
		integer *= -1;
		result[0] = '-';
	}
	insert_digit(result, integer, digits);
	result[digits] = '\0';
	return (result);
}
