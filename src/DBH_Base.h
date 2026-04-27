#ifndef __DB_HANDLER_BASE_H__
#define __DB_HANDLER_BASE_H__

#include "Icon_Enum.h"
#include "utilities.h"
#include "MyChartLoader2.h"

#include <string>

class DBH_Base
{
public:
	DBH_Base(void);
	~DBH_Base(void);

	virtual void prepare_spatial_database(const char* database_name_connstr) = 0;
	virtual void bind_chart_loader(MyChartLoader* loader) = 0;
	virtual void prepare_chart_tables() = 0;

	virtual bool check_record_exist(const char* table_name, int rcid) = 0;

	virtual void process_area_common_a() = 0;
	virtual void process_area_common_l() = 0;
	virtual void process_area_common_m() = 0;

	virtual void process_area_customize() = 0;
	virtual void process_area_DEPARE() = 0;
	virtual void process_area_LNDARE() = 0;
	virtual void process_area_DEPARE_TILED() = 0;
	virtual void process_area_LNDARE_TILED() = 0;
	virtual void process_area_DRGARE() = 0;
	virtual void process_area_OBSTRN() = 0;

	virtual void process_line_common_ml() = 0;

	virtual void process_point_common_icon() = 0;
	virtual void process_point_SOUNDG() = 0;

	virtual void close_database() = 0;

public:
	std::string get_natfs_value(Feature& feature, const char* att_name);
	std::string get_attfs_value(Feature& feature, const char* att_name);
	std::string get_restrn_marker_name(AreaFeature& area);
	std::string get_resare_marker_name(AreaFeature& area);
	std::string get_OBSTRN04_marker_name(AreaFeature& area);
	int get_OBSTRN04_line_type(AreaFeature& area);
	int get_OBSTRN04_line_type(LineFeature& line);
	std::string get_CSQUAPNT01(Feature& feature);
	std::string get_UDWHAZ03(Feature& feature, double depth_value=1e6);

	void prepare_point_floating_rigid();
	bool judge_floating(PointFeature& p);

	bool filter_area_fill(const char* shpName);
	bool filter_area_line(const char* shpName);
	bool filter_area_mark(const char* shpName);

	std::string get_area_marker1(AreaFeature& area, const char* shpName);
	std::string get_area_marker2(AreaFeature& area, const char* shpName);
	int get_fill_category(AreaFeature& area, const char* shpName);
	int get_line_category(AreaFeature& area, const char* shpName);
	int get_line_category(LineFeature& line, const char* shpName);
	int get_marker_category(PointFeature& point, const char* shpName);
public:
	MyChartLoader* m_chart;
	int m_chart_db_uid;
	std::string m_chart_db_uid_str;
	bool m_chart_tables_initialized;

	std::vector<PointFeature> m_FloatingATONArray;
	std::vector<PointFeature> m_RigidATONArray;
	
};


#endif