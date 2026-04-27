#ifndef __DB_HANDLER_BASE_H__
#define __DB_HANDLER_BASE_H__

#include "Icon_Enum.h"
#include "utilities.h"
#include "MyChartLoader2.h"

#include <string>

namespace ENC
{
	class DBHandler_Base
	{
	public:
		DBHandler_Base();
		~DBHandler_Base();

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

	protected:
		MyChartLoader* m_chart;
		int m_chart_db_uid;
		std::string m_chart_db_uid_str;
		bool m_chart_tables_initialized;

		std::vector<PointFeature> m_FloatingATONArray;
		std::vector<PointFeature> m_RigidATONArray;

	}
}
#endif