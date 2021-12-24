#include "date_time.h"
#include <stdlib.h>

struct tm *dateTime_construct(const int d, const int m, const int y)
{
	if (d < 1 || d > dateTime_getMonthDayCount(m, y) || m < 1 || m > 12 || y < 1971 || y > 2100) return NULL; //check bounds

	struct tm *t = malloc(sizeof(struct tm));
	if (!t) return NULL;
	t->tm_year = y - 1900;
	t->tm_mon = m - 1;
	t->tm_mday = d;
	t->tm_hour = 0;
	t->tm_min = 0;
	t->tm_sec = 0;
	return t;
}

void dateTime_getTrueDMY(const struct tm* dateTime, int* d, int* m, int* y)
{
	if (!dateTime)
	{
		d = 0;
		m = 0;
		y = 0;
	}
	else
	{
		*d = dateTime->tm_mday;
		*m = dateTime->tm_mon + 1;
		*y = dateTime->tm_year + 1900;
	}
}

int dateTime_getMonthDayCount(int m, int y)
{
	switch (m)
	{
	case 1: //Jan
		return 31;
	case 2: //Feb
		return dateTime_isYearLeap(y) ? 29 : 28;
	case 3: //Mar
		return 31;
	case 4: //Apr
		return 30;
	case 5: //May
		return 31;
	case 6: //Jun
		return 30;
	case 7: //Jul
		return 31;
	case 8: //Aug
		return 30;
	case 9: //Sep
		return 31;
	case 10: //Oct
		return 30;
	case 11: //Nov
		return 31;
	case 12: //Dec
		return 30;
	default: //out of bounds
		return 0;
	}
}

bool dateTime_isYearLeap(int y)
{
	//leap year happens every 4-th year
	return (y > 0) && !(y % 4) && ((y % 100) || !(y % 400));
}