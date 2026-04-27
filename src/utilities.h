#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include "MyChartLoader2.h"

const double PI = 3.14159265359f;
const double HALF_PI = PI / 2.0f;
const double EARTH_RADIUS = 6378137.0;
const double PI64 = 3.1415926535897932384626433832795028841971693993751;
const double DEGTORAD64 = PI64 / 180.0;
const double RADTODEG64 = 180.0 / PI64;

struct Point3dd
{
	double X;
	double Y;
	double Z;
};

struct GPosition
{
	double X;
	double Y;
	double Z;
};

struct Tile
{
	int x;
	int y;
	int l;
};

struct LonLatRect
{
	double lon0;
	double lat0;
	double lon1;
	double lat1;
};

inline double radToDeg(double radians)
{
	return RADTODEG64 * radians;
}
inline double degToRad(double degrees)
{
	return DEGTORAD64 * degrees;
}

inline Point3dd fwd(const GPosition& lonlat)
{
	double lonlat_y = lonlat.Y;
	if (lonlat_y < -degToRad(85.05112877980659))
		lonlat_y = -degToRad(85.05112877980659);
	if (lonlat_y > degToRad(85.05112877980659))
		lonlat_y = degToRad(85.05112877980659);

	Point3dd point;

	point.X = lonlat.X * EARTH_RADIUS;
	point.Y = log(tan(lonlat_y * 0.5 + 0.785398)) * EARTH_RADIUS; //0.25 * core::PI64
	point.Z = lonlat.Z;

	return point;
}

inline GPosition inv(const Point3dd& point)
{
	GPosition lonlat;

	lonlat.X = point.X / EARTH_RADIUS;
	lonlat.Y = 2 * (atan(exp(point.Y / EARTH_RADIUS))) - HALF_PI;
	lonlat.Z = point.Z;

	return lonlat;
}

inline Tile LonLat2Tile(double lon, double lat, int level)
{
	GPosition g;
	g.X = degToRad(lon);
	g.Y = degToRad(lat);
	g.Z = 0;
	Point3dd p = fwd(g);
	int xcount = 0x1 << level;
	int ycount = xcount;

	g.X = -PI64; g.Y = -degToRad(85.05112877980659);
	Point3dd p0 = fwd(g);
	g.X = PI64; g.Y = degToRad(85.05112877980659);
	Point3dd p1 = fwd(g);

	double x_step = (p1.X - p0.X) / xcount;
	double y_step = (p1.Y - p0.Y) / xcount;

	int tile_x = (p.X - p0.X) / x_step;
	int tile_y = (p.Y - p0.Y) / y_step;

	Tile t;
	t.x = tile_x;
	t.y = tile_y;
	t.l = level;

	return t;
}

inline LonLatRect Tile2LonLat(int x, int y, int level)
{
	int xcount = 0x1 << level;
	int ycount = xcount;

	GPosition g;
	g.X = -PI64; g.Y = -degToRad(85.05112877980659);
	Point3dd p0 = fwd(g);
	g.X = PI64; g.Y = degToRad(85.05112877980659);
	Point3dd p1 = fwd(g);

	double x_step = (p1.X - p0.X) / xcount;
	double y_step = (p1.Y - p0.Y) / xcount;

	GPosition geop1, geop2;
	{
		double w_x = p0.X + x * x_step;
		double w_y = p0.Y + y * y_step;
		Point3dd w_p;
		w_p.X = w_x; w_p.Y = w_y; w_p.Z = 0;
		geop1 = inv(w_p);

		geop1.X = radToDeg(geop1.X);
		geop1.Y = radToDeg(geop1.Y);
	}
	{
		x += 1;
		y += 1;
		double w_x = p0.X + x * x_step;
		double w_y = p0.Y + y * y_step;
		Point3dd w_p;
		w_p.X = w_x; w_p.Y = w_y; w_p.Z = 0;
		geop2 = inv(w_p);

		geop2.X = radToDeg(geop2.X);
		geop2.Y = radToDeg(geop2.Y);
	}
	LonLatRect rect;
	rect.lon0 = geop1.X;
	rect.lat0 = geop1.Y;
	rect.lon1 = geop2.X;
	rect.lat1 = geop2.Y;

	return rect;
}

inline int _parseList(const char* str_in, char* buf, int buf_size)
{
	char* str = (char*)str_in;
	int i = 0;

	if (NULL != str && *str != '\0') {
		do {
			if (i >= 1024 - 1) {
				printf("OVERFLOW --value in list lost!!\n");
				break;
			}

			buf[i++] = (unsigned char)atoi(str);

			while (isdigit(*str)) str++;  // next

		} while (*str++ != '\0');  // skip ',' or exit
	}

	buf[i] = '\0';

	return i;
}

#endif