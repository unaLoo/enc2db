#ifndef __DB_HANDLER_SPATIALITE_H__
#define __DB_HANDLER_SPATIALITE_H__

#include "DBH_Base.h"

#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gaiageo.h"

struct tiled_spatialite_geom
{
	gaiaGeomCollPtr geom;
	int tile_level;
	int tile_x;
	int tile_y;
};

class DBHandler_Spatialite: public DBH_Base
{
public:
	DBHandler_Spatialite();
	~DBHandler_Spatialite();

public:
	void prepare_spatial_database(const char* database_name_connstr);
	void bind_chart_loader(MyChartLoader* loader);

	void prepare_chart_tables();

	bool check_record_exist(const char* table_name, int rcid);

	void process_area_common_a();
	void process_area_common_l();
	void process_area_common_m();

	void process_area_customize();
	void process_area_DEPARE();
	void process_area_LNDARE();
	void process_area_DEPARE_TILED();
	void process_area_LNDARE_TILED();
	void process_area_DRGARE();
	void process_area_OBSTRN();

	void process_line_common_ml();

	void process_point_common_icon();
	void process_point_SOUNDG();

	void close_database();

private:
	void check_table_geom_index_prepared(const char* table_name, const char* geom_type);
	
	std::vector<tiled_spatialite_geom> area_to_tiled_spatialite_polygon(AreaFeature& area);
	void area_to_spatialite_polygon(AreaFeature& area, gaiaGeomCollPtr geo);
	void area_to_spatialite_polyline(AreaFeature& area, gaiaGeomCollPtr geo);
	void area_to_spatialite_centroid(AreaFeature& area, gaiaGeomCollPtr geo);
	void line_to_spatialite_line(LineFeature& line, gaiaGeomCollPtr geo);
	void point_to_spatialite_point(PointFeature& point, gaiaGeomCollPtr geo);
	void geometry_printout(gaiaGeomCollPtr geom);

private:
	sqlite3* handle;
	void* cache;
	sqlite3_stmt* stmt;
};


#endif