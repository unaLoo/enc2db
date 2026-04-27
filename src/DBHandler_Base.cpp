#include "DBHandler_Base.h"

#include <locale>
#include <codecvt>

namespace ENC
{
	std::string DBHandler_Base::get_natfs_value(Feature& feature, const char* att_name)
	{
		char* att_value_char = NULL;
		for (int iAtt = 0; iAtt < feature.natfs.size(); iAtt++)
		{
			std::string att_value = feature.natfs[iAtt].atvl;
			int temp_code = feature.natfs[iAtt].attl;
			std::string code_name = m_chart->code_name_attribute_system[temp_code];
			if (code_name == att_name)
			{
				size_t size = att_value.size();
				if (size != 0)
				{
					att_value_char = new char[size];
					for (int iChar = 0; iChar < size; iChar++)
					{
						att_value_char[iChar] = att_value[iChar];
					}
					wchar_t* pwszUnicode = (wchar_t*)att_value_char;
					pwszUnicode[(size - 1) / 2] = '\0';
					std::wstring w_str(pwszUnicode);

					std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
					return converter.to_bytes(w_str);
				}
			}
		}
		return "";
	}

	std::string DBHandler_Base::get_attfs_value(Feature& feature, const char* att_name)
	{
		for (int iAtt = 0; iAtt < feature.attfs.size(); iAtt++)
		{
			std::string att_value = feature.attfs[iAtt].atvl;
			int temp_code = feature.attfs[iAtt].attl;
			std::string code_name = m_chart->code_name_attribute_system[temp_code];
			if (code_name == att_name)
			{
				return att_value;
			}
		}
		return "";
	}

	std::string DBHandler_Base::get_restrn_marker_name(AreaFeature& area)
	{
		std::string restrn_str = "";
		for (int iAtt = 0; iAtt < area.attfs.size(); iAtt++)
		{
			std::string att_value = area.attfs[iAtt].atvl;
			int temp_code = area.attfs[iAtt].attl;
			std::string code_name = m_chart->code_name_attribute_system[temp_code];
			if (code_name == "RESTRN")
			{
				restrn_str = att_value;
				break;
			}
		}

		if (restrn_str == "") return "";

		std::string marker_name = "";
		char restrn[1024] = { '\0' };
		_parseList(restrn_str.c_str(), restrn, sizeof(restrn));

		if (strpbrk(restrn, "\007\010\016")) {
			// continuation A
			if (strpbrk(restrn, "\001\002\003\004\005\006"))
				marker_name = "ENTRES61";
			else {
				if (strpbrk(restrn, "\011\012\013\014\015"))
					marker_name = "ENTRES71";
				else
					marker_name = "ENTRES51";
			}
		}
		else {
			if (strpbrk(restrn, "\001\002")) {
				// continuation B
				if (strpbrk(restrn, "\003\004\005\006"))
					marker_name = "ACHRES61";
				else {
					if (strpbrk(restrn, "\011\012\013\014\015"))
						marker_name = "ACHRES71";
					else
						marker_name = "ACHRES51";
				}

			}
			else {
				if (strpbrk(restrn, "\003\004\005\006")) {
					// continuation C
					if (strpbrk(restrn, "\011\012\013\014\015"))
						marker_name = "FSHRES71";
					else
						marker_name = "FSHRES51";
				}
				else {
					if (strpbrk(restrn, "\011\012\013\014\015"))
						marker_name = "INFARE51";
					else
						marker_name = "RSRDEF51";
				}
			}
		}
		return marker_name;
	}

	std::string DBHandler_Base::get_resare_marker_name(AreaFeature& area)
	{
		std::string restrn_str = "";
		std::string catrea_str = "";
		for (int iAtt = 0; iAtt < area.attfs.size(); iAtt++)
		{
			std::string att_value = area.attfs[iAtt].atvl;
			int temp_code = area.attfs[iAtt].attl;
			std::string code_name = m_chart->code_name_attribute_system[temp_code];
			if (code_name == "RESTRN")
			{
				restrn_str = att_value;
			}
			if (code_name == "CATREA")
			{
				catrea_str = att_value;
			}
		}

		if (restrn_str == "") return "";

		std::string marker_name = "";
		char restrn[1024] = { '\0' };
		_parseList(restrn_str.c_str(), restrn, sizeof(restrn));

		char catrea[1024] = { '\0' };
		_parseList(catrea_str.c_str(), catrea, sizeof(catrea));

		if (std::strpbrk(restrn, "\007\010\016"))
		{
			if (std::strpbrk(restrn, "\001\002\003\004\005\006")) marker_name = "ENTRES61";
			else
			{
				if (catrea_str != "" && std::strpbrk(catrea, "\001\010\011\014\016\023\025\031")) marker_name = "ENTRES61";
				else
				{
					if (std::strpbrk(restrn, "\011\012\013\014\015")) marker_name = "ENTRES71";
					else
					{
						if (catrea_str != "" && std::strpbrk(catrea, "\004\005\006\007\012\022\024\026\027\030")) marker_name = "ENTRES71";
						else marker_name = "ENTRES51";
					}
				}
			}
		}
		else
		{
			if (std::strpbrk(restrn, "\001\002"))
			{
				if (std::strpbrk(restrn, "\003\004\005\006")) marker_name = "ACHRES61";
				else
				{
					if (catrea_str != "" && std::strpbrk(catrea, "\001\010\011\014\016\023\025\031")) marker_name = "ACHRES61";
					else
					{
						if (std::strpbrk(restrn, "\011\012\013\014\015")) marker_name = "ACHRES71";
						else
						{
							if (catrea_str != "" && std::strpbrk(catrea, "\004\005\006\007\012\022\024\026\027\030")) marker_name = "ACHRES71";
							else marker_name = "ACHRES51";
						}
					}
				}
			}
			else
			{
				if (std::strpbrk(restrn, "\003\004\005\006"))
				{
					if (catrea_str != "" && std::strpbrk(catrea, "\001\010\011\014\016\023\025\031")) marker_name = "FSHRES51";
					else
					{
						if (strpbrk(restrn, "\011\012\013\014\015")) marker_name = "FSHRES71";
						else
						{
							if (catrea_str != "" && std::strpbrk(catrea, "\004\005\006\007\012\022\024\026\027\030")) marker_name = "FSHRES71";
							else marker_name = "FSHRES51";
						}
					}
				}
				else
				{
					if (std::strpbrk(restrn, "\011\012\013\014\015")) marker_name = "INFARE51";
					else marker_name = "RSRDEF51";
				}
			}
		}
		return marker_name;
	}

	std::string DBHandler_Base::get_OBSTRN04_marker_name(AreaFeature& area)
	{
		std::string markername = "";
		std::string catobs = get_attfs_value(area, "CATOBS");
		std::string watlev = get_attfs_value(area, "WATLEV");
		std::string expsou = get_attfs_value(area, "EXPSOU");
		std::string valsou = get_attfs_value(area, "VALSOU");

		double depth_value = -15.0;
		if (valsou != "")
		{
			depth_value = atof(valsou.c_str());
		}
		else {
			if (expsou != "1") {
				if ("6" == catobs)  depth_value = 0.01;
				else if ("0" == watlev)  depth_value = -15.0;
				else {
					if (watlev == "5") depth_value = 0.0;
					else if (watlev == "3")  depth_value = 0.01;
					else  depth_value = -15.0;
				}
			}
		}

		std::string udwhaz03str = get_UDWHAZ03(area, depth_value);
		std::string quapnt01str = get_CSQUAPNT01(area);

		if (udwhaz03str != "")
		{
			markername = udwhaz03str;
		}

		markername = get_CSQUAPNT01(area);

		return markername;
	}

	int DBHandler_Base::get_OBSTRN04_line_type(AreaFeature& area)
	{
		std::string markername = "";
		std::string catobs_str = get_attfs_value(area, "CATOBS");
		std::string watlev_str = get_attfs_value(area, "WATLEV");
		std::string expsou_str = get_attfs_value(area, "EXPSOU");
		std::string valsou_str = get_attfs_value(area, "VALSOU");

		double depth_value = -15.0;
		if (valsou_str != "")
		{
			depth_value = atof(valsou_str.c_str());
		}
		else {
			if (expsou_str != "1") {
				if ("6" == catobs_str)  depth_value = 0.01;
				else if ("0" == watlev_str)  depth_value = -15.0;
				else {
					if (watlev_str == "5") depth_value = 0.0;
					else if (watlev_str == "3")  depth_value = 0.01;
					else  depth_value = -15.0;
				}
			}
		}

		int line_type = 0;
		std::string udwhaz03str = get_UDWHAZ03(area, depth_value);
		if (udwhaz03str != "") line_type = 3; //LS(DOTT,2,CHBLK)

		if (valsou_str != "")
		{
			double valsou = atof(valsou_str.c_str());
			if (valsou <= 20.0) line_type = 3; //LS(DOTT,2,CHBLK)
			else line_type = 4; //LS(DASH,2,CHBLK)
		}
		else
		{
			int watlev = -9;
			if (watlev_str != "") watlev = atoi(watlev_str.c_str());
			if (watlev == -9)  // default
				line_type = 3; //LS(DOTT, 2, CHBLK)"));
			else {
				switch (watlev) {
				case 1:
				case 2:
					line_type = 5; //LS(SOLD,2,CSTLN)
					break;
				case 4:
					line_type = 6; //LS(DASH,2,CSTLN)
					break;
				case 5:
				case 3: {
					int catobs = -9;
					if (catobs_str != "") catobs = atoi(catobs_str.c_str());
					if (6 == catobs)
						line_type = 3;  //LS(DOTT,2,CHBLK)
					else
						line_type = 3;  //LS(DOTT,2,CHBLK)
						} break;
				default:
					line_type = 3;  //LS(DOTT,2,CHBLK)
					break;
				}
			}
		}

		return line_type;
	}

	int DBHandler_Base::get_OBSTRN04_line_type(LineFeature& line)
	{
		std::string markername = "";
		std::string catobs_str = get_attfs_value(line, "CATOBS");
		std::string watlev_str = get_attfs_value(line, "WATLEV");
		std::string expsou_str = get_attfs_value(line, "EXPSOU");
		std::string valsou_str = get_attfs_value(line, "VALSOU");

		double depth_value = -15.0;
		if (valsou_str != "")
		{
			depth_value = atof(valsou_str.c_str());
		}
		else {
			if (expsou_str != "1") {
				if ("6" == catobs_str)  depth_value = 0.01;
				else if ("0" == watlev_str)  depth_value = -15.0;
				else {
					if (watlev_str == "5") depth_value = 0.0;
					else if (watlev_str == "3")  depth_value = 0.01;
					else  depth_value = -15.0;
				}
			}
		}

		int line_type = 0;
		std::string udwhaz03str = get_UDWHAZ03(line, depth_value);

		if (udwhaz03str != "")
		{
			line_type = 3; //LS(DOTT,2,CHBLK)
		}
		if (valsou_str != "") 
		{
			double valsou = atof(valsou_str.c_str());
			if (valsou <= 20.0)
				line_type = 3; //LS(DOTT,2,CHBLK)
			else
				line_type = 4; //LS(DASH,2,CHBLK)
		}
		else
			line_type = 3; //LS(DOTT,2,CHBLK)

		return line_type;
	}

	std::string DBHandler_Base::get_CSQUAPNT01(Feature& feature)
	{
		std::string markername = "";
		bool accurate = true;
		int qualty = 10;
		std::string quapos_str = get_attfs_value(feature, "QUAPOS");
		if (quapos_str!="") qualty = atoi(quapos_str.c_str());
		if (quapos_str != "")
		{
			if (2 <= qualty && qualty < 10) accurate = false;
		}
		if (!accurate) {
			switch (qualty) {
			case 4:
				markername = "QUAPOS01";
				break;  // "PA"
			case 5:
				markername = "QUAPOS02";
				break;  // "PD"
			case 7:
			case 8:
				markername = "QUAPOS03";
				break;  // "REP"
			default:
				markername = "LOWACC03";
				break;  // "?"
			}
		}
		return markername;
	}

	std::string DBHandler_Base::get_UDWHAZ03(Feature& feature, double depth_value)
	{
		int danger = false;
		int expsou = 0;
		double safety_contour = 30;
		bool b_promote = false;

		if (depth_value == 1e6)
		{
			std::string expsou_str = get_attfs_value(feature, "EXPSOU");
			expsou = atoi(expsou_str.c_str());
			if (expsou != 1) danger = true;
		}
		if (danger == false && (expsou == 1 || depth_value <= safety_contour))
		{
			//?
		}
		if (danger == true)
		{
			std::string watlev_str = get_attfs_value(feature, "WATLEV");
			int watlev = 0;
			if (watlev_str != "") watlev = atoi(watlev_str.c_str());
			if ((1 == watlev) || (2 == watlev)) 
			{
			}
			else
			{
				return "ISODGR51";
			}
		}
		return "";
	}

	void DBHandler_Base::prepare_point_floating_rigid()
	{
		std::map<int, std::vector<PointFeature> >::iterator pfIter;
		std::map<int, std::vector<PointFeature> >::iterator end = m_chart->point_features_map.end();
		for (pfIter = m_chart->point_features_map.begin(); pfIter != end; ++pfIter)
		{
			int featureId = pfIter->first;
			std::vector<PointFeature>& temp_point_vec = pfIter->second;
			std::string shpName = m_chart->code_name_system[featureId];
			if (shpName == "LITFLT" || shpName == "LITVES" || shpName == "BOYCAR" || shpName == "BOYINB" ||
				shpName == "BOYISD" || shpName == "BOYLAT" || shpName == "BOYSAW" || shpName == "BOYSPP")
			{
				for (int iP = 0; iP < temp_point_vec.size(); iP++)
					m_FloatingATONArray.push_back(temp_point_vec[iP]);
			}
			if (shpName == "BCNCAR" || shpName == "BCNISD" || shpName == "BCNLAT" || shpName == "BCNSAW" ||
				shpName == "BCNSPP")
			{
				for (int iP = 0; iP < temp_point_vec.size(); iP++)
					m_RigidATONArray.push_back(temp_point_vec[iP]);
			}
		}
	}

	bool DBHandler_Base::judge_floating(PointFeature& p)
	{
		if (m_FloatingATONArray.size() == 0)
		{
			prepare_point_floating_rigid();
		}
		for (int iP = 0; iP < m_FloatingATONArray.size(); iP++)
		{
			PointFeature temp_p = m_FloatingATONArray[iP];
			if (temp_p.long_lat[0] == p.long_lat[0] && temp_p.long_lat[1] == p.long_lat[1])
			{
				return true;
			}
		}
		return false;
	}



	bool DBHandler_Base::filter_area_fill(const char* shpName_str)
	{
		std::string shpName(shpName_str);

		if (shpName == "AIRARE" || shpName == "ACHBRT" ||
			shpName == "BUAARE" || shpName == "BUISGL" ||
			shpName == "CAUSWY" || shpName == "CRANES" ||
			shpName == "DAMCON" || shpName == "DOCARE" || shpName == "DRYDOC" ||
			shpName == "FLODOC" || shpName == "FORSTC" ||
			shpName == "GATCON" || 
			shpName == "HULKES" || shpName == "ICEARE" ||
			shpName == "LAKARE" || shpName == "LNDRGN" || shpName == "LNDMRK" || shpName == "LOKBSN" || 
			shpName == "MARCUL" || shpName == "MORFAC" ||
			shpName == "OFSPLF" ||
			shpName == "PONTON" || shpName == "PRCARE" || shpName == "PYLONS" ||
			shpName == "RAPIDS" || shpName == "RIVERS" || shpName == "RUNWAY" ||
			shpName == "SNDWAV" ||
			shpName == "SEAARE" || shpName == "SBDARE" || shpName == "SLCONS" ||
			shpName == "SILTNK" || shpName == "SLOGRD" || shpName == "SMCFAC" ||
			shpName == "TSSLPT" || shpName == "TSEZNE" || shpName == "TSSCRS" || shpName == "TSSRON" ||
			shpName == "TUNNEL" || shpName == "VEGATN" ||
			shpName == "UNSARE" ) return true;
		else return false;
	}
	bool DBHandler_Base::filter_area_line(const char* shpName_str)
	{
		std::string shpName(shpName_str);

		if (shpName == "ADMARE" || shpName == "AIRARE" || shpName == "ACHBRT" || shpName == "ACHARE" || 
			shpName == "BERTHS" || shpName == "BRIDGE" || shpName == "BUAARE" || shpName == "BUISGL" ||
			shpName == "CAUSWY" || shpName == "CANALS" || shpName == "CBLARE" || shpName == "CTNARE" ||
			shpName == "CTSARE" || shpName == "CONZNE" || shpName == "COSARE" ||
			shpName == "CONVYR" || shpName == "CRANES" || shpName == "CUSZNE" ||
			shpName == "DAMCON" || shpName == "DOCARE" || shpName == "DRGARE" || shpName == "DRYDOC" ||
			shpName == "DYKCON" || shpName == "DMPGRD" || shpName == "DWRTPT" ||
			shpName == "EXEZNE" ||
			shpName == "FAIRWY" || shpName == "FERYRT" || shpName == "FLODOC" || shpName == "FSHZNE" || 
			shpName == "FSHFAC" || shpName == "FSHGRD" || shpName == "FORSTC" || shpName == "FRPARE" ||
			shpName == "GATCON" || shpName == "GRIDRN" ||
			shpName == "HRBARE" || shpName == "HULKES" || 
			shpName == "ICEARE" || shpName == "ICNARE" || shpName == "ISTZNE" ||
			shpName == "LAKARE" || shpName == "LNDMRK" || shpName == "LOKBSN" || shpName == "LOGPON" ||
			shpName == "LOCMAG" ||
			shpName == "MARCUL" || shpName == "MIPARE" || shpName == "MORFAC" ||
			shpName == "OBSTRN" || shpName == "OFSPLF" || shpName == "OSPARE" ||
			shpName == "PILBOP" || shpName == "PIPARE" || shpName == "PONTON" ||
			shpName == "PRCARE" || shpName == "PRDARE" || shpName == "PYLONS" || 
			shpName == "RADRNG" || shpName == "RECTRC" || shpName == "RESARE" || shpName == "RIVERS" ||
			shpName == "RUNWAY" ||
			shpName == "SNDWAV" || shpName == "SBDARE" || shpName == "SPLARE" || shpName == "SLCONS" ||
			shpName == "SBDARE" || shpName == "SILTNK" || shpName == "SLOGRD" || shpName == "SMCFAC" ||
			shpName == "SUBTLN" || shpName == "SWPARE" || 
			shpName == "TESARE" || 
			shpName == "TS_PRH" || shpName == "TS_PNH" || shpName == "TS_PAD" || shpName == "TS_TIS" ||
			shpName == "T_HMON" || shpName == "T_NHMN" || shpName == "T_TIMS" || shpName == "TUNNEL" ||
			shpName == "TWRTPT" || shpName == "UNSARE" || shpName == "VEGATN" || shpName == "WATTUR" ||
			shpName == "WEDKLP" || shpName == "TS_FEB"
			) return true;
		else return false;
	}
	bool DBHandler_Base::filter_area_mark(const char* shpName_str)
	{
		std::string shpName(shpName_str);

		if (shpName == "ACHARE" || shpName == "BRIDGE" || shpName == "BERTHS" ||
			shpName == "CBLARE" || shpName == "CTNARE" || shpName == "CRANES" || shpName == "CTSARE" ||
			shpName == "CHKPNT" || shpName == "CONVYR" || 
			shpName == "DMPGRD" || shpName == "DWRTPT" ||
			shpName == "FAIRWY" || shpName == "FERYRT" || shpName == "FSHFAC" || shpName == "FSHGRD" ||
			shpName == "GATCON" || shpName == "HRBFAC" ||
			shpName == "ICNARE" ||shpName == "ISTZNE" || 
			shpName == "LOCMAG" ||
			shpName == "MAGVAR" || shpName == "MARCUL" || shpName == "MIPARE" ||
			shpName == "OBSTRN" || shpName == "OSPARE" ||
			shpName == "PILBOP" || shpName == "PIPARE" || shpName == "PRCARE" || shpName == "PRDARE" ||
			shpName == "RECTRC" || shpName == "RCTLPT" || shpName == "RESARE" || 
			shpName == "SPLARE" || shpName == "SMCFAC" || shpName == "SUBTLN" || 
			shpName == "SWPARE" || shpName == "TESARE" || 
			shpName == "TS_PRH" || shpName == "TS_PNH" || shpName == "TS_PAD" || shpName == "TS_TIS" || 
			shpName == "T_HMON" || shpName == "T_NHMN" || shpName == "T_TIMS" ||
			shpName == "TIDEWY" || shpName == "TSSCRS" || shpName == "TSSLPT" || shpName == "TSSRON" ||
			shpName == "TWRTPT" || shpName == "WATTUR" || shpName == "WEDKLP" || shpName == "TS_FEB"
			) return true;
		else return false;
	}
	std::string DBHandler_Base::get_area_marker1(AreaFeature& area, const char* shpName_str)
	{
		std::string shpName(shpName_str);
		std::string markername = "";
		if (shpName == "ACHARE")
		{
			std::string cat_str = get_attfs_value(area, "CATACH");
			if (cat_str == "8") markername = "ACHARE02";
			else markername = "ACHARE51";
		}
		if (shpName == "BRIDGE")
		{
			std::string cat_str = get_attfs_value(area, "CATBRG");
			if (cat_str == "2" || cat_str == "3" || cat_str == "4" || cat_str == "5" || cat_str == "7" || cat_str == "8")
				markername = "BRIDGE01";
		}
		if (shpName == "CONVYR")
		{
			std::string conrad_str = get_attfs_value(area, "CONRAD");
			if (conrad_str == "1" || conrad_str == "3") markername = "RACNSP01";
			else markername = "";
		}
		if (shpName == "DMPGRD")
		{
			std::string catdpg_str = get_attfs_value(area, "CATDPG");
			if (catdpg_str != "5") markername = "INFARE51";
		}
		if (shpName == "DWRTPT")
		{
			std::string orient = get_attfs_value(area, "ORIENT");
			std::string trafic = get_attfs_value(area, "TRAFIC");
			if (orient != "" && (trafic == "1" || trafic == "2" || trafic == "3" || trafic == "4")) markername = "TSSLPT51";
			else if (orient != "" && trafic == "4") markername = "DWRUTE51";
			else markername = "TWRTPT52";
		}
		if (shpName == "FAIRWY")
		{
			std::string orient = get_attfs_value(area, "ORIENT");
			std::string trafic = get_attfs_value(area, "TRAFIC");
			if (orient != "" && (trafic == "1" || trafic == "2" || trafic == "3")) markername = "FAIRWY51";
			else if (orient != "" && trafic == "4") markername = "FAIRWY52";
			else markername = "";
		}
		if (shpName == "FERYRT")
		{
			std::string catfrt_str = get_attfs_value(area, "CATFRY");
			if (catfrt_str == "2") markername = "FRYARE52";
			else markername = "FRYARE51";
		}
		if (shpName == "FSHFAC")
		{
			std::string cat_str = get_attfs_value(area, "CATFIF");
			if (cat_str == "1") markername = "FSHFAC03";
			else markername = "FSHFAC02";
		}
		if (shpName == "GATCON")
		{
			std::string cat_str = get_attfs_value(area, "CATGAT");
			if (cat_str == "4") markername = "GATCON03";
			else markername = "GATCON04";
		}
		if (shpName == "HRBFAC")
		{
			std::string cat_str = get_attfs_value(area, "CATHAF");
			if (cat_str == "1") markername = "ROLROL01";
			else if (cat_str == "4") markername = "HRBFAC09";
			else if (cat_str == "5") markername = "SMCFAC02";
			else markername = "CHINFO07";
		}
		if (shpName == "OBSTRN")
		{
			std::string catobs = get_attfs_value(area, "CATOBS");
			std::string watlev = get_attfs_value(area, "WATLEV");
			std::string expsou = get_attfs_value(area, "EXPSOU");
			std::string valsou = get_attfs_value(area, "VALSOU");

			if (catobs == "7" && valsou != "") markername = "FOULGND1";
			else if (catobs == "10")  markername = "FLTHAZ02";
			else if (catobs == "6") markername = "OBSTRN04";
			else if (catobs == "7") markername = "FOULGND1";
			else if (catobs == "8") markername = "FLTHAZ02";
			else if (catobs == "9") markername = "ACHARE02";
			else if (watlev == "7") markername = "FLTHAZ02";
			else markername = "OBSTRN04";

			if (markername == "OBSTRN04")
			{
				markername = get_OBSTRN04_marker_name(area);
			}
		}
		if (shpName == "PRDARE")
		{
			std::string catpra_str = get_attfs_value(area, "CATPRA");
			std::string convis_str = get_attfs_value(area, "CONVIS");

			if (catpra_str == "5" && convis_str == "1") markername = "RFNERY11";
			else if (catpra_str == "8" && convis_str == "1") markername = "TNKFRM11";
			else if (catpra_str == "9" && convis_str == "1") markername = "WNDFRM61";
			else if (catpra_str == "1") markername = "QUARRY01";
			else if (catpra_str == "5") markername = "RFNERY01";
			else if (catpra_str == "6") markername = "TMBYRD01";
			else if (catpra_str == "8") markername = "TNKFRM01";
			else if (catpra_str == "9") markername = "WNDFRM51";
			else markername = "";
		}
		if (shpName == "RECTRC")
		{
			std::string orient_str = get_attfs_value(area, "ORIENT");
			std::string cattrk_str = get_attfs_value(area, "CATTRK");
			std::string trafic_str = get_attfs_value(area, "TRAFIC");
			if (orient_str != "" && cattrk_str == "1" && trafic_str == "1") markername = "RECTRC58";
			else if (orient_str != "" && cattrk_str == "1" && trafic_str == "2") markername = "RECTRC58";
			else if (orient_str != "" && cattrk_str == "1" && trafic_str == "3") markername = "RECTRC58";
			else if (orient_str != "" && cattrk_str == "1" && trafic_str == "4") markername = "RECTRC56";
			else if (orient_str != "" && cattrk_str == "2" && trafic_str == "1") markername = "RECTRC57";
			else if (orient_str != "" && cattrk_str == "2" && trafic_str == "2") markername = "RECTRC57";
			else if (orient_str != "" && cattrk_str == "2" && trafic_str == "3") markername = "RECTRC57";
			else if (orient_str != "" && cattrk_str == "2" && trafic_str == "4") markername = "RECTRC55";
			else if (orient_str != "" && trafic_str == "1") markername = "RECTRC57";
			else if (orient_str != "" && trafic_str == "2") markername = "RECTRC57";
			else if (orient_str != "" && trafic_str == "3") markername = "RECTRC57";
			else if (orient_str != "" && trafic_str == "4") markername = "RECTRC55";
			else markername = "RECDEF51";
		}
		if (shpName == "RCTLPT")
		{
			std::string orient_str = get_attfs_value(area, "ORIENT");
			if (orient_str != "") markername = "RCTLPT52";
			else markername = "RTLDEF51";
		}
		if (shpName == "TWRTPT")
		{
			std::string orient = get_attfs_value(area, "ORIENT");
			std::string trafic = get_attfs_value(area, "TRAFIC");
			if (orient != "" && (trafic == "1" || trafic == "2" || trafic == "3")) markername = "TWRTPT53";
			else if (orient != "" && trafic == "4") markername = "TWRTPT52";
			else markername = "TWRDEF51";
		}
		if (shpName == "TS_FEB")
		{
			std::string catts_str = get_attfs_value(area, "CAT_TS");
			std::string orient_str = get_attfs_value(area, "ORIENT");
			if (orient_str != "" && catts_str != "1") markername = "FLDSTR01";
			else if (orient_str != "" && catts_str != "2") markername = "EBBSTR01";
			else if (orient_str != "" && catts_str != "3") markername = "CURENT01";
			else markername = "CURDEF01";
		}

		return markername;
	}
	std::string DBHandler_Base::get_area_marker2(AreaFeature& area, const char* shpName_str)
	{
		std::string shpName(shpName_str);
		std::string markername_1 = "";

		if (shpName == "ACHARE")
		{
			std::string cat_str = get_attfs_value(area, "CATACH");
			if (cat_str == "8") markername_1 = get_restrn_marker_name(area);
			else markername_1 = get_resare_marker_name(area);
		}
		if (shpName == "DMPGRD")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "CBLARE")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "FAIRWY")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "ICNARE")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "PIPARE")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "RESARE")
		{
			std::string cat_str = get_attfs_value(area, "CATREA");
			if (cat_str == "27") markername_1 = "ESSARE01";
			else if (cat_str == "28") markername_1 = "PSSARE01";
			else markername_1 = get_resare_marker_name(area);
		}
		if (shpName == "MARCUL")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "MIPARE")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "OSPARE")
		{ 
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "PRCARE")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "SPLARE")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "SUBTLN")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "TESARE")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "TSSCRS")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "TSSLPT")
		{
			markername_1 = get_restrn_marker_name(area);
		}
		if (shpName == "TSSRON")
		{
			markername_1 = get_restrn_marker_name(area);
		}

		return markername_1;
	}

	int DBHandler_Base::get_fill_category(AreaFeature& area, const char* shpName_str)
	{
		std::string shpName(shpName_str);
		int fill_type = 0;
		if (shpName == "CAUSWY")
		{
			std::string watlev_str = get_attfs_value(area, "WATLEV");
			if (watlev_str == "4") fill_type = 1; //AC(DEPIT)
			else fill_type = 2; //AC(CHBRN)
		}
		if (shpName == "LNDRGN")
		{
			std::string cat_lnd_str = get_attfs_value(area, "CATLND");
			if (cat_lnd_str == "2" || cat_lnd_str == "12") fill_type = 1;
			else fill_type = 2;
		}
		if (shpName == "SBDARE")
		{
			std::string watlev_str = get_attfs_value(area, "WATLEV");
			std::string natsur_str = get_attfs_value(area, "NATSUR");
			if (watlev_str == "4" && (natsur_str == "11" || natsur_str == "14" || natsur_str == "9"))
				fill_type = 1;
			else if ((watlev_str == "3" || watlev_str == "4") && natsur_str != "") fill_type = 2;
		}
		if (shpName == "SLOGRD")
		{
			std::string catslo_str = get_attfs_value(area, "CATSLO");
			std::string conrad_str = get_attfs_value(area, "CONRAD");
			if ( (catslo_str == "1" || catslo_str == "2" || catslo_str == "3" || catslo_str == "4" || catslo_str == "5" || catslo_str == "7") &&
				conrad_str == "1")
				fill_type = 1;
			else if (catslo_str == "6") fill_type = 1;
		}
		if (shpName == "TUNNEL")
		{
			std::string burdep_str = get_attfs_value(area, "BURDEP");
			if (burdep_str == "0") fill_type = 1;
		}
		if (shpName == "VEGATN")
		{
			std::string catveg_str = get_attfs_value(area, "CATVEG");
			if (catveg_str == "3" || catveg_str == "4" || catveg_str == "5" || catveg_str == "6" ||
				catveg_str == "14" || catveg_str == "15" || catveg_str == "16" || catveg_str == "17" ||
				catveg_str == "18" || catveg_str == "19" || catveg_str == "20" || catveg_str == "22")
				fill_type = 1;
			else if (catveg_str == "21" || catveg_str == "7")
				fill_type = 2;
			else fill_type = 3;
		}
		return fill_type;
	}

	int DBHandler_Base::get_line_category(AreaFeature& area, const char* shpName_str)
	{
		std::string shpName(shpName_str);
		int line_type = 0;
		if (shpName == "AIRARE")
		{
			std::string convis_str = get_attfs_value(area, "CONVIS");
			if (convis_str == "1") line_type = 1; //LS(SOLD,1,CHBLK)
			else line_type = 2; //LS(SOLD,1,LANDF)
		}
		if (shpName == "BUISGL")
		{
			std::string functn_str = get_attfs_value(area, "FUNCTN");
			std::string convis_str = get_attfs_value(area, "CONVIS");
			if (convis_str == "1") line_type = 1;  //LS(SOLD,1,CHBLK)
			else line_type = 2;  // LS(SOLD,1,LANDF)
		}
		if (shpName == "CANALS")
		{
			std::string condtn_str = get_attfs_value(area, "CONDTN");
			if (condtn_str != "") line_type = 1; //LS(DASH,1,CHBLK)
			else line_type = 2; //LS(SOLD,1,CHBLK)
		}
		if (shpName == "CAUSWY")
		{
			std::string watlev_str = get_attfs_value(area, "WATLEV");
			if (watlev_str == "4") line_type = 1; //LS(DASH,2,CSTLN)
			else line_type = 2; //LS(SOLD,1,CSTLN)
		}
		if (shpName == "CRANES")
		{
			std::string convis_str = get_attfs_value(area, "CONVIS");
			if (convis_str == "1") line_type = 1;  //LS(SOLD, 1, CHBLK)
			else line_type = 2;  // LS(SOLD,1,LANDF)
		}
		if (shpName == "DAMCON")
		{
			std::string catdam_str = get_attfs_value(area, "CATDAM");
			if (catdam_str == "3") line_type = 1;  //LS(SOLD,2,CSTLN)
			else line_type = 2;  // LS(SOLD,1,LANDF)
		}
		if (shpName == "DMPGRD")
		{
			std::string catdpg_str = get_attfs_value(area, "CATDPG");
			if (catdpg_str == "5") line_type = 1;  //LC(NAVARE51)
			else line_type = 2;  //LC(CTYARE51)
		}
		if (shpName == "DOCARE")
		{
			std::string condtn_str = get_attfs_value(area, "CONDTN");
			if (condtn_str != "") line_type = 1;  //LS(DASH,1,CHBLK)
			else line_type = 2;  // LS(SOLD,1,CHBLK)
		}
		if (shpName == "FERYRT")
		{
			std::string catfrt_str = get_attfs_value(area, "CATFRY");
			if (catfrt_str == "2") line_type = 1; //LS(DASH,2,CHBLK)
			else line_type = 2; //LS(DASH,2,CHMGD)
		}
		if (shpName == "FORSTC")
		{
			std::string convis_str = get_attfs_value(area, "CONVIS");
			if (convis_str == "1") line_type = 1;  //LS(SOLD,1,CHBLK)
			else line_type = 2;  // LS(SOLD,1,LANDF)
		}
		if (shpName == "LNDMRK")
		{
			std::string catlmk_str = get_attfs_value(area, "CATLMK");
			std::string functn_str = get_attfs_value(area, "FUNCTN");
			std::string convis_str = get_attfs_value(area, "CONVIS");

			if (catlmk_str == "17" && functn_str == "33" && convis_str == "1") line_type = 1;  //LS(SOLD,1,CHBLK)
			else if (catlmk_str == "17" && functn_str == "33") line_type = 2;  //LS(SOLD,1,LANDF)
			else if (convis_str == "1") line_type = 1;  //LS(SOLD, 1, CHBLK)
			else line_type = 2;  //LS(SOLD,1,LANDF)
		}
		if (shpName == "OBSTRN")
		{
			std::string catobs = get_attfs_value(area, "CATOBS");
			std::string watlev = get_attfs_value(area, "WATLEV");
			std::string expsou = get_attfs_value(area, "EXPSOU");
			std::string valsou = get_attfs_value(area, "VALSOU");

			if (catobs == "10")  line_type = 1; // DASH, CSTLN
			else if (catobs == "6") line_type = 4; // DOTT, 2, CHBLK
			else if (catobs == "7")  line_type = 2; // DASH, NAVARE51
			else if (catobs == "8")  line_type = 3; // DASH, CSTLN
			else if (catobs == "9")  line_type = 2; // DASH, CHMGD
			else if (watlev == "7")  line_type = 1; // DASH, CSTLN
			else
			{
				if (watlev == "")  // default
					line_type = 4; // DOTT, 2, CHBLK
				else {
					int watlev_val = atoi(watlev.c_str());
					switch (watlev_val) {
					case 1:
					case 2:
						line_type = 5; // SOLD, 2, CSTLN
						break;
					case 4:
						line_type = 6; // DASH, 2, CSTLN
						break;
					case 5:
					case 3: {
						line_type = 7; // DOTT,2,CHBLK
							} break;
					default:
						line_type = 7; // DOTT,2,CHBLK
						break;
					}
				}
			}
		}
		if (shpName == "PIPARE")
		{
			std::string prodct_str = get_attfs_value(area, "PRODCT");

			if (prodct_str == "1" || prodct_str == "2")
				line_type = 1;  //CHMGD
			else
				line_type = 2; //CHGRD
		}
		if (shpName == "PRDARE")
		{
			std::string catpra_str = get_attfs_value(area, "CATPRA");
			std::string convis_str = get_attfs_value(area, "CONVIS");

			if (catpra_str == "5" && convis_str == "1") line_type = 1;
			else if (catpra_str == "8" && convis_str == "1") line_type = 1;
			else if (catpra_str == "9" && convis_str == "1") line_type = 1;
			else if (catpra_str == "1") line_type = 2;
			else if (catpra_str == "5") line_type = 2;
			else if (catpra_str == "6") line_type = 2;
			else if (catpra_str == "8") line_type = 2;
			else if (catpra_str == "9") line_type = 2;
			else line_type = 2;
		}
		if (shpName == "RUNWAY")
		{
			std::string convis_str = get_attfs_value(area, "CONVIS");
			if (convis_str == "1") line_type = 1;  //LS(SOLD,1,CHBLK)
			else line_type = 0;  //
		}
		if (shpName == "SILTNK")
		{
			std::string convis_str = get_attfs_value(area, "CONVIS");
			if (convis_str == "1") line_type = 1;  //LS(SOLD,1,CHBLK)
			else line_type = 2;  // LS(SOLD,1,LANDF)
		}
		if (shpName == "SLOGRD")
		{
			std::string catslo_str = get_attfs_value(area, "CATSLO");
			std::string conrad_str = get_attfs_value(area, "CONRAD");
			if ((catslo_str == "1" || catslo_str == "2" || catslo_str == "3" || catslo_str == "4" || catslo_str == "5" || catslo_str == "7") &&
				conrad_str == "1")
				line_type = 1;
			else if (catslo_str == "6") line_type = 1;
		}
		if (shpName == "TUNNEL")
		{
			std::string burdep_str = get_attfs_value(area, "BURDEP");
			if (burdep_str == "0") line_type = 1; // LS(DASH,1,CHBLK)
			else line_type = 2;  // LS(DASH,1,CHGRD)
		}
		if (shpName == "WRECKS")
		{
			std::string quapos_str = get_attfs_value(area, "QUAPOS");
			std::string valsou_str = get_attfs_value(area, "VALSOU");
			std::string watlev_str = get_attfs_value(area, "WATLEV");
			std::string catwrk_str = get_attfs_value(area, "CATWRK");
			std::string expsou_str = get_attfs_value(area, "EXPSOU");

			if (catwrk_str == "3") line_type = 1;
			else
			{
				int catwrk = atoi(catwrk_str.c_str());
				int watlev = atoi(watlev_str.c_str());
				int quapos = atoi(quapos_str.c_str());
				int valsou = atoi(valsou_str.c_str());
				int expsou = atoi(expsou_str.c_str());
				double depth_value = 0;
				{
					if (-9 != catwrk) {
						switch (catwrk) {
						case 1:
							depth_value = 20.0;
							break;  // safe
						case 2:
							depth_value = 0.0;
							break;  // dangerous
						case 4:
						case 5:
							depth_value = -15.0;
							break;
						}
					}
					else {
						if (-9 == watlev)  // default
							depth_value = -15.0;
						else
							switch (watlev) {
							case 1:
							case 2:
								depth_value = -15.0;
								break;
							case 3:
								depth_value = 0.01;
								break;
							case 4:
								depth_value = -15.0;
								break;
							case 5:
								depth_value = 0.0;
								break;
							case 6:
								depth_value = -15.0;
								break;
						}
					}
				}
				std::string markername = "";
				bool danger = false;
				if (expsou != 1) danger = true;
				if (danger)
				{
					if (!(1 == watlev) || (2 == watlev)) markername = "ISODGR51";
				}

				if (2 <= quapos && quapos < 10)
					line_type = 1;  // ";LC(LOWACC41)");
				else {
					if (0 != markername.size())
						line_type = 2;  //_T(";LS(DOTT,2,CHBLK)");
					else {
						if (valsou_str != "") {
							if (valsou <= 20)
								line_type = 2;  // _T(";LS(DOTT,2,CHBLK)");
							else
								line_type = 3; // = _T(";LS(DASH,2,CHBLK)");
						}
						else {
							if (-9 == watlev)
								line_type = 4; // _T(";LS(DOTT,2,CSTLN)");
							else {
								switch (watlev) {
								case 1:
								case 2:
									line_type = 5; // _T(";LS(SOLD,2,CSTLN)");
									break;
								case 4:
									line_type = 6; // _T(";LS(DASH,2,CSTLN)");
									break;
								case 3:
								case 5:

								default:
									line_type = 4; //  _T(";LS(DOTT,2,CSTLN)");
									break;
								}
							}
						}
					}
				}
			}
		}
		if (shpName == "TS_FEB")
		{
			std::string catts_str = get_attfs_value(area, "CAT_TS");
			std::string orient_str = get_attfs_value(area, "ORIENT");
			if (orient_str != "" && (catts_str != "1" || catts_str != "2" || catts_str == "3")) line_type = 0;
			else line_type = 1;
		}
		return line_type;
	}

	int DBHandler_Base::get_line_category(LineFeature& line, const char* shpName_str)
	{
		std::string shpName(shpName_str);
		int category = 0;
		if (shpName == "CAUSWY")
		{
			std::string watlev_str = get_attfs_value(line, "WATLEV");
			if (watlev_str == "4") category = 1;
			else category = 2;
		}
		if (shpName == "COALNE")
		{
			std::string catcoa_str = get_attfs_value(line, "CATCOA");
			if (catcoa_str == "10" || catcoa_str == "6" || catcoa_str == "7" || catcoa_str=="8") category = 1;
			else category = 2;
		}
		if (shpName == "CONVYR")
		{
			std::string catcon_str = get_attfs_value(line, "CATCON");
			std::string conrad_str = get_attfs_value(line, "CONRAD");
			if (catcon_str == "1" && conrad_str == "1") category = 1;
			else if (catcon_str == "1" && conrad_str == "3") category = 1;
			else if (catcon_str == "2" && conrad_str == "1") category = 2;
			else if (catcon_str == "2" && conrad_str == "3") category = 2;
			else if (catcon_str == "1") category = 1;
			else if (catcon_str == "2") category = 2;
			else if (conrad_str == "1") category = 1;
			else if (conrad_str == "3") category = 1;
			else category = 1;
		}
		if (shpName == "DYKCON")
		{
			std::string conrad_str = get_attfs_value(line, "CONRAD");
			if (conrad_str == "1") category = 1;
			else category = 2;
		}
		if (shpName == "FNCLNE")
		{
			std::string convis_str = get_attfs_value(line, "CONVIS");
			if (convis_str == "1") category = 1;
			else convis_str = 2;
		}
		if (shpName == "FERYRT")
		{
			std::string catfry_str = get_attfs_value(line, "CATFRY");
			if (catfry_str == "1") category = 1;
			else category = 2;
		}
		if (shpName == "FSHFAC")
		{
			std::string catfif_str = get_attfs_value(line, "CATFIF");
			if (catfif_str == "1") category = 1;
			else catfif_str = 2;
		}
		if (shpName == "LNDMRK")
		{
			std::string convis_str = get_attfs_value(line, "CONVIS");
			if (convis_str == "1") category = 1;
			else convis_str = 2;
		}
		if (shpName == "MORFAC")
		{
			std::string catmor_str = get_attfs_value(line, "CATMOR");
			if (catmor_str == "6") category = 1;
			else catmor_str = 2;
		}
		if (shpName == "OBSTRN")
		{
			std::string catobs_str = get_attfs_value(line, "CATOBS");
			std::string watlev_str = get_attfs_value(line, "WATLEV");

			if (catobs_str == "10" || catobs_str == "8")
				category = 1; //LS(DASH,1,CSTLN)
			else if (catobs_str == "9")
				category = 2; //LS(DASH,1,CHMGD)
			else if (watlev_str == "7")
				category = 1;
			else
				category = get_OBSTRN04_line_type(line);
		}
		if (shpName == "RECTRC")
		{
			std::string cattrk_str = get_attfs_value(line, "CATTRK");
			std::string trafic_str = get_attfs_value(line, "TRAFIC");
			if (cattrk_str == "1" && trafic_str == "1") category = 1; //RECTRC12
			else if (cattrk_str == "1" && trafic_str == "2") category = 1;
			else if (cattrk_str == "1" && trafic_str == "3") category = 1;
			else if (cattrk_str == "1" && trafic_str == "4") category = 2; //RECTRC10
			else if (cattrk_str == "2" && trafic_str == "1") category = 3;
			else if (cattrk_str == "2" && trafic_str == "2") category = 3;
			else if (cattrk_str == "2" && trafic_str == "3") category = 3; //RECTRC11
			else if (cattrk_str == "2" && trafic_str == "4") category = 4; //RECTRC09
			else if (trafic_str == "1") category = 3;
			else if (trafic_str == "2") category = 3;
			else if (trafic_str == "3") category = 3;
			else if (trafic_str == "4") category = 4;
			else category = 5; //RECDEF02

			//if (trafic_str == "3")
			//{
			//    if (cattrk_str == "1") category = 1;
			//    else category = 2;
			//}
			//else if (trafic_str == "4")
			//{
			//    if (cattrk_str == "1") category = 3;
			//    else category = 4;
			//}
			//else
			//{
			//    category = 5;
			//}
		}
		if (shpName == "SLCONS")
		{
			std::string quapos_str = get_attfs_value(line, "QUAPOS");
			std::string condtn_str = get_attfs_value(line, "CONDTN");
			std::string catslc_str = get_attfs_value(line, "CATSLC");
			std::string watlev_str = get_attfs_value(line, "WATLEV");

			int quapos = atoi(quapos_str.c_str());
			if (2 <= quapos && quapos < 10) category = 1; //LC(LOWACC01)
			else
			{
				if (condtn_str != "" && ("1" == condtn_str || "2" == condtn_str))
					category = 1; // "LS(DASH,1,CSTLN)";
				else {
					if (catslc_str != "" &&
						("6" == catslc_str || "15" == catslc_str || "16" == catslc_str))
						category = 2; //cmdw = "LS(SOLD,4,CSTLN)";
					else {
						if (watlev_str != "" && "2" == watlev_str)
							category = 3;  //"LS(SOLD,2,CSTLN)";
						else if (watlev_str != "" && ("3" == watlev_str || "4" == watlev_str))
							category = 4; //"LS(DASH,2,CSTLN)";
						else
							category = 5; // "LS(SOLD,2,CSTLN)";  // default
					}
				}
			}
		}
		if (shpName == "SLOTOP")
		{
			std::string catslo_str = get_attfs_value(line, "CATSLO");
			std::string conrad_str = get_attfs_value(line, "CONRAD");
			std::string convis_str = get_attfs_value(line, "CONVIS");
			if (catslo_str == "2" && conrad_str == "1") category = 1;
			else if (catslo_str == "6" && conrad_str == "1") category = 1;
			else if (catslo_str == "6" && convis_str == "1") category = 1;
			else if (catslo_str == "2") category = 2;
			else if (catslo_str == "6") category = 2;
			else category = 3;
		}
		if (shpName == "TUNNEL")
		{
			std::string burdep_str = get_attfs_value(line, "BURDEP");
			if (burdep_str == "0") category = 1;
			else category = 2;
		}
		if (shpName == "WATFAL")
		{
			std::string convis_str = get_attfs_value(line, "CONVIS");
			if (convis_str == "1") category = 1;
			else category = 2;
		}
		return category;
	}

	DBHandler_Base::DBHandler_Base()
	{

	}

	DBHandler_Base::~DBHandler_Base()
	{

	}

}
