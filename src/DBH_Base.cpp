#include "compat.h"
#include "DBH_Base.h"

#include <locale>
#include <codecvt>
#include <algorithm>

DBH_Base::DBH_Base(void)
{
}
DBH_Base::~DBH_Base(void)
{
}


std::string DBH_Base::get_natfs_value(Feature& feature, const char* att_name)
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

std::string DBH_Base::get_attfs_value(Feature& feature, const char* att_name)
{
	for (int iAtt = 0; iAtt < feature.attfs.size(); iAtt++)
	{
		std::string att_value = feature.attfs[iAtt].atvl;
		int temp_code = feature.attfs[iAtt].attl;
		std::string code_name = m_chart->code_name_attribute_system[temp_code];
		if (code_name == att_name)
		{
			char temp_char[1024];
			int flag = 0;
			for (int ic = 0; ic<att_value.size(); ic++)
			{
				if (int(att_value[ic]) < 0 || int(att_value[ic]) > 255 )
				{
					continue;
				}
				else
				{
					temp_char[flag] = att_value[ic];
					flag++;
				}
			}
			temp_char[flag] = '\0';
			att_value = std::string(temp_char);
			
			return att_value;
		}
	}
	return "";
}

std::string DBH_Base::get_restrn_marker_name(AreaFeature& area)
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

	if (STRPBRK(restrn, "\007\010\016")) {
		// continuation A
		if (STRPBRK(restrn, "\001\002\003\004\005\006"))
			marker_name = "ENTRES61";
		else {
			if (STRPBRK(restrn, "\011\012\013\014\015"))
				marker_name = "ENTRES71";
			else
				marker_name = "ENTRES51";
		}
	}
	else {
		if (STRPBRK(restrn, "\001\002")) {
			// continuation B
			if (STRPBRK(restrn, "\003\004\005\006"))
				marker_name = "ACHRES61";
			else {
				if (STRPBRK(restrn, "\011\012\013\014\015"))
					marker_name = "ACHRES71";
				else
					marker_name = "ACHRES51";
			}

		}
		else {
			if (STRPBRK(restrn, "\003\004\005\006")) {
				// continuation C
				if (STRPBRK(restrn, "\011\012\013\014\015"))
					marker_name = "FSHRES71";
				else
					marker_name = "FSHRES51";
			}
			else {
				if (STRPBRK(restrn, "\011\012\013\014\015"))
					marker_name = "INFARE51";
				else
					marker_name = "RSRDEF51";
			}
		}
	}
	return marker_name;
}

std::string DBH_Base::get_resare_marker_name(AreaFeature& area)
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

	if (STRPBRK(restrn, "\007\010\016"))
	{
		if (STRPBRK(restrn, "\001\002\003\004\005\006")) marker_name = "ENTRES61";
		else
		{
			if (catrea_str != "" && STRPBRK(catrea, "\001\010\011\014\016\023\025\031")) marker_name = "ENTRES61";
			else
			{
				if (STRPBRK(restrn, "\011\012\013\014\015")) marker_name = "ENTRES71";
				else
				{
					if (catrea_str != "" && STRPBRK(catrea, "\004\005\006\007\012\022\024\026\027\030")) marker_name = "ENTRES71";
					else marker_name = "ENTRES51";
				}
			}
		}
	}
	else
	{
		if (STRPBRK(restrn, "\001\002"))
		{
			if (STRPBRK(restrn, "\003\004\005\006")) marker_name = "ACHRES61";
			else
			{
				if (catrea_str != "" && STRPBRK(catrea, "\001\010\011\014\016\023\025\031")) marker_name = "ACHRES61";
				else
				{
					if (STRPBRK(restrn, "\011\012\013\014\015")) marker_name = "ACHRES71";
					else
					{
						if (catrea_str != "" && STRPBRK(catrea, "\004\005\006\007\012\022\024\026\027\030")) marker_name = "ACHRES71";
						else marker_name = "ACHRES51";
					}
				}
			}
		}
		else
		{
			if (STRPBRK(restrn, "\003\004\005\006"))
			{
				if (catrea_str != "" && STRPBRK(catrea, "\001\010\011\014\016\023\025\031")) marker_name = "FSHRES51";
				else
				{
					if (STRPBRK(restrn, "\011\012\013\014\015")) marker_name = "FSHRES71";
					else
					{
						if (catrea_str != "" && STRPBRK(catrea, "\004\005\006\007\012\022\024\026\027\030")) marker_name = "FSHRES71";
						else marker_name = "FSHRES51";
					}
				}
			}
			else
			{
				if (STRPBRK(restrn, "\011\012\013\014\015")) marker_name = "INFARE51";
				else marker_name = "RSRDEF51";
			}
		}
	}
	return marker_name;
}

std::string DBH_Base::get_OBSTRN04_marker_name(AreaFeature& area)
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

int DBH_Base::get_OBSTRN04_line_type(AreaFeature& area)
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

int DBH_Base::get_OBSTRN04_line_type(LineFeature& line)
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

std::string DBH_Base::get_CSQUAPNT01(Feature& feature)
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

std::string DBH_Base::get_UDWHAZ03(Feature& feature, double depth_value)
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

void DBH_Base::prepare_point_floating_rigid()
{
    // m_FloatingATONArray 存储了浮动的ATON点要素
    // m_RigidATONArray 存储了固定的ATON点要素
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

bool DBH_Base::judge_floating(PointFeature& p)
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



bool DBH_Base::filter_area_fill(const char* shpName_str)
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
bool DBH_Base::filter_area_line(const char* shpName_str)
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
bool DBH_Base::filter_area_mark(const char* shpName_str)
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
std::string DBH_Base::get_area_marker1(AreaFeature& area, const char* shpName_str)
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
std::string DBH_Base::get_area_marker2(AreaFeature& area, const char* shpName_str)
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

int DBH_Base::get_fill_category(AreaFeature& area, const char* shpName_str)
{
	std::string shpName(shpName_str);
	int fill_type = 0;
	if (shpName == "CAUSWY")
	{
		std::string watlev_str = get_attfs_value(area, "WATLEV");
		if (watlev_str == "4") fill_type = 1; //AC(DEPIT) // 绿色
		else fill_type = 2; //AC(CHBRN) // 棕黄色
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

int DBH_Base::get_line_category(AreaFeature& area, const char* shpName_str)
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

int DBH_Base::get_line_category(LineFeature& line, const char* shpName_str)
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

int DBH_Base::get_marker_category(PointFeature& point, const char* shpName_str)
{
	std::string shpName(shpName_str);
	int icon_type = -1;
    if (shpName == "BCNCAR")
    {
        icon_type = BCNCAR_ICON::BCNCAR_BCNGEN03;

        std::string bcnshp_str = get_attfs_value(point, "BCNSHP");
        std::string colour_str = get_attfs_value(point, "COLOUR");

        if (bcnshp_str == "3" && colour_str == "2,6,2")  icon_type = BCNCAR_ICON::BCNCAR_BCNTOW70;
        else if (bcnshp_str == "3" && colour_str == "6,2,6")  icon_type = BCNCAR_ICON::BCNCAR_BCNTOW71;
        else if (bcnshp_str == "3" && colour_str == "2,6")  icon_type = BCNCAR_ICON::BCNCAR_BCNTOW68;
        else if (bcnshp_str == "3" && colour_str == "6,2")  icon_type = BCNCAR_ICON::BCNCAR_BCNTOW69;
        else if (colour_str == "2,6,2")  icon_type = BCNCAR_ICON::BCNCAR_BCNGEN70;
        else if (colour_str == "6,2,6")  icon_type = BCNCAR_ICON::BCNCAR_BCNGEN71;
        else if (colour_str == "2,6")  icon_type = BCNCAR_ICON::BCNCAR_BCNGEN68;
        else if (colour_str == "6,2")  icon_type = BCNCAR_ICON::BCNCAR_BCNGEN69;

        else if (bcnshp_str == "1")  icon_type = BCNCAR_ICON::BCNCAR_BCNSTK02;
        else if (bcnshp_str == "3")  icon_type = BCNCAR_ICON::BCNCAR_BCNTOW01;
        else if (bcnshp_str == "4")  icon_type = BCNCAR_ICON::BCNCAR_BCNLTC01;
        else if (bcnshp_str == "5")  icon_type = BCNCAR_ICON::BCNCAR_BCNGEN01;
        else if (bcnshp_str == "7")  icon_type = BCNCAR_ICON::BCNCAR_BCNGEN01;
        else icon_type = BCNCAR_ICON::BCNCAR_BCNGEN03;
    }
    if (shpName == "BCNISD")
    {
        icon_type = BCNISD_ICON::BCNISD_BCNGEN03;

        std::string bcnshp_str = get_attfs_value(point, "BCNSHP");
        std::string colour_str = get_attfs_value(point, "COLOUR");

        if (bcnshp_str == "3" && colour_str == "2,3,2")  icon_type = BCNISD_ICON::BCNISD_BCNTOW76;
        else if (colour_str == "2,3,2")  icon_type = BCNISD_ICON::BCNISD_BCNGEN76;
        else if (bcnshp_str == "1")  icon_type = BCNISD_ICON::BCNISD_BCNSTK02;
        else if (bcnshp_str == "3")  icon_type = BCNISD_ICON::BCNISD_BCNTOW01;
        else if (bcnshp_str == "4")  icon_type = BCNISD_ICON::BCNISD_BCNLTC01;
        else if (bcnshp_str == "5")  icon_type = BCNISD_ICON::BCNISD_BCNGEN01;
        else if (bcnshp_str == "7")  icon_type = BCNISD_ICON::BCNISD_BCNGEN01;
        else icon_type = BCNISD_ICON::BCNISD_BCNGEN03;
    }
    if (shpName == "BCNLAT")
    {
        icon_type = BCNLAT_ICON::BCNLAT_BCNDEF13;
        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string bcnshp_str = get_attfs_value(point, "BCNSHP");
        std::string catlam_str = get_attfs_value(point, "CATLAM");
        std::string colpat_str = get_attfs_value(point, "COLPAT");
        std::string convis_str = get_attfs_value(point, "CONVIS");

        if (bcnshp_str == "3" && catlam_str == "4" && colpat_str == "1" && colour_str == "3,4,3")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW74;
        else if (bcnshp_str == "3" && catlam_str == "1" && colpat_str == "1" && colour_str == "1,4")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW66;
        else if (bcnshp_str == "3" && catlam_str == "1" && colpat_str == "1" && colour_str == "4,1")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW65;
        else if (bcnshp_str == "3" && catlam_str == "2" && colpat_str == "1" && colour_str == "1,3")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW63;
        else if (bcnshp_str == "3" && catlam_str == "2" && colpat_str == "1" && colour_str == "3,1")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW64;
        else if (bcnshp_str == "5" && catlam_str == "1" && colour_str == "4")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK61;
        else if (bcnshp_str == "5" && catlam_str == "2" && colour_str == "3")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK60;
        else if (bcnshp_str == "1" && colour_str == "1,4,1")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK77;
        else if (bcnshp_str == "1" && colour_str == "1,3")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK78;
        else if (bcnshp_str == "1" && colour_str == "1,4")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK77;
        else if (bcnshp_str == "1" && colour_str == "3")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK60;
        else if (bcnshp_str == "1" && colour_str == "4")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK61;

        else if (bcnshp_str == "2" && catlam_str == "1")  icon_type = BCNLAT_ICON::BCNLAT_PRICKE03;
        else if (bcnshp_str == "2" && catlam_str == "2")  icon_type = BCNLAT_ICON::BCNLAT_PRICKE04;

        else if (bcnshp_str == "3" && colour_str == "3")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW60;
        else if (bcnshp_str == "3" && colour_str == "4")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW61;

        else if (bcnshp_str == "4" && colour_str == "3")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW60;
        else if (bcnshp_str == "4" && colour_str == "4")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW61;

        else if (bcnshp_str == "5" && catlam_str == "1")  icon_type = BCNLAT_ICON::BCNLAT_BCNGEN01;
        else if (bcnshp_str == "5" && catlam_str == "2")  icon_type = BCNLAT_ICON::BCNLAT_BCNGEN01;

        else if (bcnshp_str == "6" && convis_str == "1")  icon_type = BCNLAT_ICON::BCNLAT_CAIRNS11;

        else if (bcnshp_str == "7" && colour_str == "1")  icon_type = BCNLAT_ICON::BCNLAT_BCNTOW05;
        else if (bcnshp_str == "7" && colour_str == "3")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK60;
        else  if (bcnshp_str == "7" && colour_str == "4")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK61;

        else if (bcnshp_str == "5" && colour_str == "1,3") icon_type = BCNLAT_ICON::BCNLAT_BCNSTK78;
        else if (bcnshp_str == "5" && colour_str == "1,4") icon_type = BCNLAT_ICON::BCNLAT_BCNSTK81;
        else if (bcnshp_str == "5" && colour_str == "3") icon_type = BCNLAT_ICON::BCNLAT_BCNGEN60;
        else if (bcnshp_str == "5" && colour_str == "4") icon_type = BCNLAT_ICON::BCNLAT_BCNGEN61;
        else if (bcnshp_str == "1" && colour_str == "3,4") icon_type = BCNLAT_ICON::BCNLAT_BOYCON79;
        else if (bcnshp_str == "1" && colour_str == "4,3") icon_type = BCNLAT_ICON::BCNLAT_BOYCON68;

        else if (colour_str == "3,1")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK78;
        else if (colour_str == "4,1")  icon_type = BCNLAT_ICON::BCNLAT_BCNSTK77;

        else if (colpat_str == "1" && colour_str == "3,4") icon_type = BCNLAT_ICON::BCNLAT_BCNSTK82;
        else if (colpat_str == "1" && colour_str == "4,3") icon_type = BCNLAT_ICON::BCNLAT_BCNSTK83;

        else if (bcnshp_str == "1") icon_type = BCNLAT_ICON::BCNLAT_BCNSTK02;
        else if (bcnshp_str == "3") icon_type = BCNLAT_ICON::BCNLAT_BCNTOW01;
        else if (bcnshp_str == "4") icon_type = BCNLAT_ICON::BCNLAT_BCNLTC01;
        else if (bcnshp_str == "5") icon_type = BCNLAT_ICON::BCNLAT_BCNGEN01;
        else if (bcnshp_str == "6") icon_type = BCNLAT_ICON::BCNLAT_CAIRNS01;
        else if (bcnshp_str == "7") icon_type = BCNLAT_ICON::BCNLAT_BCNGEN01;

        else if (colour_str == "3")  icon_type = BCNLAT_ICON::BCNLAT_BCNGEN60;
        else if (colour_str == "4")  icon_type = BCNLAT_ICON::BCNLAT_BCNGEN61;

        else icon_type = BCNLAT_ICON::BCNLAT_BCNGEN01;
    }
    if (shpName == "BCNSAW")
    {
        icon_type = BCNSAW_ICON::BCNSAW_BCNGEN03;

        std::string bcnshp_str = get_attfs_value(point, "BCNSHP");
        std::string colour_str = get_attfs_value(point, "COLOUR");

        if (bcnshp_str == "7" && colour_str == "3,1") icon_type = BCNSAW_ICON::BCNSAW_BCNSTK78;
        else if (colour_str == "3,1") icon_type = BCNSAW_ICON::BCNSAW_BCNSTK78;
        else if (bcnshp_str == "1") icon_type = BCNSAW_ICON::BCNSAW_BCNSTK02;
        else if (bcnshp_str == "3") icon_type = BCNSAW_ICON::BCNSAW_BCNTOW01;
        else if (bcnshp_str == "4") icon_type = BCNSAW_ICON::BCNSAW_BCNLTC01;
        else if (bcnshp_str == "5") icon_type = BCNSAW_ICON::BCNSAW_BCNGEN01;
        else if (bcnshp_str == "7") icon_type = BCNSAW_ICON::BCNSAW_BCNGEN01;
        else  icon_type = BCNSAW_ICON::BCNSAW_BCNGEN03;
    }
    if (shpName == "BCNSPP")
    {
        icon_type = BCNSPP_ICON::BCNSPP_BCNGEN01;
        std::string bcnshp_str = get_attfs_value(point, "BCNSHP");
        std::string convis_str = get_attfs_value(point, "CONVIS");
        std::string catspm_str = get_attfs_value(point, "CATSPM");
        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string colpat_str = get_attfs_value(point, "COLPAT");
        /*if (bcnshp_str == "1") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK02;
        else if (bcnshp_str == "3") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW01;
        else if (bcnshp_str == "4") icon_type = BCNSPP_ICON::BCNSPP_BCNLTC01;
        else if (bcnshp_str == "5") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN01;
        else if (bcnshp_str == "6") icon_type = BCNSPP_ICON::BCNSPP_CAIRNS01;
        else if (bcnshp_str == "7") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN01;

        if (catspm_str == "18")  icon_type = BCNSPP_ICON::BCNSPP_NOTBRD11;
        if (catspm_str == "44")  icon_type = BCNSPP_ICON::BCNSPP_BCNGEN01;
        else if (bcnshp_str == "6" && convis_str=="1") icon_type = BCNSPP_ICON::BCNSPP_CAIRNS11;*/

        if (bcnshp_str == "3" && colpat_str == "1" && colour_str == "2,1,2") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW87;
        else if (bcnshp_str == "3" && colpat_str == "2" && colour_str == "4,1") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW85;
        else if (bcnshp_str == "5" && catspm_str == "1" && colour_str == "4") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK61;
        else if (bcnshp_str == "3" && colour_str == "4,1") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW65;
        else if (bcnshp_str == "4" && colour_str == "1,2") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW86;
        else if (bcnshp_str == "4" && colour_str == "2,1") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW88;
        else if (catspm_str == "18" && colour_str == "6") icon_type = BCNSPP_ICON::BCNSPP_NOTBRD12;
        else if (bcnshp_str == "1" && colour_str == "1") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK05;
        else if (bcnshp_str == "1" && colour_str == "3") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK60;
        else if (bcnshp_str == "1" && colour_str == "4") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK61;
        else if (bcnshp_str == "1" && colour_str == "6") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK08;
        else if (bcnshp_str == "3" && colour_str == "1") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW05;
        else if (bcnshp_str == "3" && colour_str == "2") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW89;
        else if (bcnshp_str == "3" && colour_str == "4") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW61;
        else if (bcnshp_str == "3" && colour_str == "6") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW62;
        else if (bcnshp_str == "4" && colour_str == "1") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW91;
        else if (bcnshp_str == "4" && colour_str == "2") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW89;
        else if (bcnshp_str == "4" && colour_str == "3") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW60;
        else if (bcnshp_str == "4" && colour_str == "8") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW90;
        else if (bcnshp_str == "5" && colour_str == "1") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK05;
        else if (bcnshp_str == "5" && colour_str == "6") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK08;
        else if (bcnshp_str == "6" && convis_str == "1") icon_type = BCNSPP_ICON::BCNSPP_CAIRNS11;
        else if (bcnshp_str == "7" && colour_str == "6") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK62;
        else if (colour_str == "3,4,3") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK79;
        else if (colour_str == "4,3,4") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK80;
        else if (colour_str == "3,1") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK78;
        else if (colour_str == "4,1") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK81;
        else if (catspm_str == "18") icon_type = BCNSPP_ICON::BCNSPP_NOTBRD11;
        else if (catspm_str == "44") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN01;
        else if (catspm_str == "11") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN79;
        else if (bcnshp_str == "1") icon_type = BCNSPP_ICON::BCNSPP_BCNSTK02;
        else if (bcnshp_str == "3") icon_type = BCNSPP_ICON::BCNSPP_BCNTOW01;
        else if (bcnshp_str == "4") icon_type = BCNSPP_ICON::BCNSPP_BCNLTC01;
        else if (bcnshp_str == "5") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN01;
        else if (bcnshp_str == "6") icon_type = BCNSPP_ICON::BCNSPP_CAIRNS01;
        else if (bcnshp_str == "7") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN01;
        else if (colour_str == "1") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN05;
        else if (colour_str == "2") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN80;
        else if (colour_str == "3") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN60;
        else if (colour_str == "4") icon_type = BCNSPP_ICON::BCNSPP_BCNGEN61;
        else if (colour_str == "6") icon_type = BCNSPP_ICON::BCNSPP_BCNSPR62;
        else icon_type = BCNSPP_ICON::BCNSPP_BCNGEN01;
    }
    if (shpName == "BOYCAR")
    {
        icon_type = BOYCAR_ICON::BOYCAR_BOYCAN70;

        std::string boyshp_str = get_attfs_value(point, "BOYSHP");
        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string catcam_str = get_attfs_value(point, "CATCAM");

        if (boyshp_str == "1" && colour_str == "2,6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYCON71;
        else if (boyshp_str == "1" && colour_str == "6,2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYCON72;
        else if (boyshp_str == "2" && colour_str == "2,6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYCAN70;
        else if (boyshp_str == "2" && colour_str == "6,2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYCAN71;
        else if (boyshp_str == "3" && colour_str == "2,6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYSPH70;
        else if (boyshp_str == "3" && colour_str == "6,2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYSPH71;
        else if (boyshp_str == "4" && colour_str == "2,6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYPIL70;
        else if (boyshp_str == "4" && colour_str == "6,2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYPIL71;
        else if (boyshp_str == "5" && colour_str == "2,6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR70;
        else if (boyshp_str == "5" && colour_str == "6,2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR71;
        else if (boyshp_str == "1" && colour_str == "2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYCON69;
        else if (boyshp_str == "1" && colour_str == "6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYCON70;
        else if (boyshp_str == "2" && colour_str == "2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYCAN68;
        else if (boyshp_str == "2" && colour_str == "6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYCAN69;
        else if (boyshp_str == "3" && colour_str == "2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYSPH68;
        else if (boyshp_str == "3" && colour_str == "6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYSPH69;
        else if (boyshp_str == "4" && colour_str == "2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYPIL68;
        else if (boyshp_str == "4" && colour_str == "6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYPIL69;
        else if (boyshp_str == "5" && colour_str == "2,6") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR68;
        else if (boyshp_str == "5" && colour_str == "6,2") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR69;

        else if (boyshp_str == "4" && catcam_str == "1") icon_type = BOYCAR_ICON::BOYCAR_BOYPIL68;
        else if (boyshp_str == "4" && catcam_str == "2") icon_type = BOYCAR_ICON::BOYCAR_BOYPIL70;
        else if (boyshp_str == "4" && catcam_str == "3") icon_type = BOYCAR_ICON::BOYCAR_BOYPIL69;
        else if (boyshp_str == "4" && catcam_str == "4") icon_type = BOYCAR_ICON::BOYCAR_BOYPIL71;

        else if (boyshp_str == "5" && catcam_str == "1") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR68;
        else if (boyshp_str == "5" && catcam_str == "2") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR70;
        else if (boyshp_str == "5" && catcam_str == "3") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR69;
        else if (boyshp_str == "5" && catcam_str == "4") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR71;
                    
        else if (boyshp_str == "1") icon_type = BOYCAR_ICON::BOYCAR_BOYCON01;
        else if (boyshp_str == "2") icon_type = BOYCAR_ICON::BOYCAR_BOYCAN01;
        else if (boyshp_str == "3") icon_type = BOYCAR_ICON::BOYCAR_BOYSPH01;
        else if (boyshp_str == "4") icon_type = BOYCAR_ICON::BOYCAR_BOYPIL01;
        else if (boyshp_str == "5") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR01;
        else if (boyshp_str == "6") icon_type = BOYCAR_ICON::BOYCAR_BOYBAR01;
        else if (boyshp_str == "7") icon_type = BOYCAR_ICON::BOYCAR_BOYSUP01;
        else if (boyshp_str == "8") icon_type = BOYCAR_ICON::BOYCAR_BOYSPR01;
        else icon_type = BOYCAR_ICON::BOYCAR_BOYGEN03;
    }
    if (shpName == "BOYINB")
    {
        icon_type = BOYINB_ICON::BOYINB_BOYINB01;

        std::string boyshp_str = get_attfs_value(point, "BOYSHP");
        std::string colour_str = get_attfs_value(point, "COLOUR");

        if (boyshp_str == "6" && colour_str == "6") icon_type = BOYINB_ICON::BOYINB_BOYBAR62;
        else if (boyshp_str == "7" && colour_str == "6") icon_type = BOYINB_ICON::BOYINB_BOYSUP62;
        else icon_type = BOYINB_ICON::BOYINB_BOYINB01;
    }
    if (shpName == "BOYISD")
    {
        icon_type = BOYISD_ICON::BOYISD_BOYGEN03;

        std::string boyshp_str = get_attfs_value(point, "BOYSHP");
        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string colpat_str = get_attfs_value(point, "COLPAT");

        if (boyshp_str == "1" && colpat_str == "1" && colour_str == "1,11") icon_type = BOYISD_ICON::BOYISD_BOYCON77;
        else if (boyshp_str == "1" && colour_str == "2,3,2") icon_type = BOYISD_ICON::BOYISD_BOYCON63;
        else if (boyshp_str == "2" && colour_str == "2,3,2") icon_type = BOYISD_ICON::BOYISD_BOYCAN76;
        else if (boyshp_str == "4" && colour_str == "2,3,2") icon_type = BOYISD_ICON::BOYISD_BOYPIL72;
        else if (boyshp_str == "5" && colour_str == "2,3,2") icon_type = BOYISD_ICON::BOYISD_BOYSPR72;
        else if (boyshp_str == "4" && colour_str == "2,3") icon_type = BOYISD_ICON::BOYISD_BOYPIL72;
        else if (boyshp_str == "1") icon_type = BOYISD_ICON::BOYISD_BOYCON01;
        else if (boyshp_str == "2") icon_type = BOYISD_ICON::BOYISD_BOYCAN01;
        else if (boyshp_str == "3") icon_type = BOYISD_ICON::BOYISD_BOYSPH01;
        else if (boyshp_str == "4") icon_type = BOYISD_ICON::BOYISD_BOYPIL01;
        else if (boyshp_str == "5") icon_type = BOYISD_ICON::BOYISD_BOYSPR01;
        else if (boyshp_str == "6") icon_type = BOYISD_ICON::BOYISD_BOYBAR01;
        else if (boyshp_str == "7") icon_type = BOYISD_ICON::BOYISD_BOYSUP01;
        else if (boyshp_str == "8") icon_type = BOYISD_ICON::BOYISD_BOYSPR01;
        else icon_type = BOYISD_ICON::BOYISD_BOYGEN03;
    }
    if (shpName == "BOYLAT")
    {
        icon_type = BOYLAT_ICON::BOYLAT_BOYGEN03;

        std::string boyshp_str = get_attfs_value(point, "BOYSHP");
        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string colpat_str = get_attfs_value(point, "COLPAT");
        std::string catlam_str = get_attfs_value(point, "CATLAM");
        std::string convis_str = get_attfs_value(point, "CONVIS");

        if (boyshp_str == "2" && catlam_str == "7" && colour_str == "3,1,3,1,3") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN82;
        else if (boyshp_str == "2" && catlam_str == "7" && colour_str == "3,1,3,1") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN83;
        else if (boyshp_str == "1" && colour_str == "3,4,3" && colpat_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYCON66;
        else if (boyshp_str == "2" && colour_str == "4,3,4" && colpat_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN73;
        else if (boyshp_str == "4" && colour_str == "3,4,3" && colpat_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL66;
        else if (boyshp_str == "4" && colour_str == "4,3,4" && colpat_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL67;
        else if (boyshp_str == "1" && colour_str == "3,4" && colpat_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYSPH79;
        else if (boyshp_str == "3" && colour_str == "3,4" && colpat_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYSPH74;
        else if (boyshp_str == "3" && colour_str == "4,3" && colpat_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYSPH75;
        else if (boyshp_str == "4" && colour_str == "4,3" && colpat_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL74;

        else if (boyshp_str == "3" && catlam_str == "23" && colour_str == "6") icon_type = BOYLAT_ICON::BOYLAT_BOYSPH62;
        else if (boyshp_str == "1" && colour_str == "4,1,4,1,4") icon_type = BOYLAT_ICON::BOYLAT_BOYCON65;
        else if (boyshp_str == "2" && colour_str == "3,1,3,1,3") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN74;
        else if (boyshp_str == "3" && colour_str == "3,4,3,4,3") icon_type = BOYLAT_ICON::BOYLAT_BOYSPH74;
        else if (boyshp_str == "1" && colour_str == "4,3,4") icon_type = BOYLAT_ICON::BOYLAT_BOYCON67;
        else if (boyshp_str == "2" && colour_str == "3,4,3") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN72;
        else if (boyshp_str == "4" && colour_str == "3,4,3") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL66;
        else if (boyshp_str == "4" && colour_str == "4,3,4") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL67;
        else if (boyshp_str == "1" && colour_str == "4,1") icon_type = BOYLAT_ICON::BOYLAT_BOYCON73;
        else if (boyshp_str == "1" && colour_str == "4,3") icon_type = BOYLAT_ICON::BOYLAT_BOYCON68;
        else if (boyshp_str == "2" && colour_str == "3,1") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN74;
        else if (boyshp_str == "2" && colour_str == "3,4") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN75;
        else if (boyshp_str == "3" && colour_str == "3,4") icon_type = BOYLAT_ICON::BOYLAT_BOYSPH74;
        else if (boyshp_str == "1" && colour_str == "2") icon_type = BOYLAT_ICON::BOYLAT_BOYCON64;
        else if (boyshp_str == "1" && colour_str == "3") icon_type = BOYLAT_ICON::BOYLAT_BOYCON60;
        else if (boyshp_str == "1" && colour_str == "4") icon_type = BOYLAT_ICON::BOYLAT_BOYCON61;
        else if (boyshp_str == "1" && colour_str == "6") icon_type = BOYLAT_ICON::BOYLAT_BOYCON62;
        else if (boyshp_str == "2" && colour_str == "3") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN60;
        else if (boyshp_str == "2" && colour_str == "4") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN61;
        else if (boyshp_str == "2" && colour_str == "6") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN63;
        else if (boyshp_str == "4" && colour_str == "3") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL60;
        else if (boyshp_str == "4" && colour_str == "4") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL61;
        else if (boyshp_str == "5" && colour_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYSPR05;
        else if (boyshp_str == "5" && colour_str == "3") icon_type = BOYLAT_ICON::BOYLAT_BOYSPR60;
        else if (boyshp_str == "5" && colour_str == "4") icon_type = BOYLAT_ICON::BOYLAT_BOYSPR61;

        else if (colour_str == "3" && catlam_str == "2") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL60;
        else if (colour_str == "4" && catlam_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL61;
                    
        else if (boyshp_str == "1") icon_type = BOYLAT_ICON::BOYLAT_BOYCON01;
        else if (boyshp_str == "2") icon_type = BOYLAT_ICON::BOYLAT_BOYCAN01;
        else if (boyshp_str == "3") icon_type = BOYLAT_ICON::BOYLAT_BOYSPH01;
        else if (boyshp_str == "4") icon_type = BOYLAT_ICON::BOYLAT_BOYPIL01;
        else if (boyshp_str == "5") icon_type = BOYLAT_ICON::BOYLAT_BOYSPR01;
        else if (boyshp_str == "6") icon_type = BOYLAT_ICON::BOYLAT_BOYBAR01;
        else if (boyshp_str == "7") icon_type = BOYLAT_ICON::BOYLAT_BOYSUP01;
        else if (boyshp_str == "8") icon_type = BOYLAT_ICON::BOYLAT_BOYSPR01;
        else icon_type = BOYLAT_ICON::BOYLAT_BOYGEN03;
    }
    if (shpName == "BOYSAW")
    {
        icon_type = BOYSAW_ICON::BOYSAW_BOYGEN03;

        std::string boyshp_str = get_attfs_value(point, "BOYSHP");
        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string colpat_str = get_attfs_value(point, "COLPAT");

        if (boyshp_str == "3" && colour_str == "3,1" && colpat_str == "2") icon_type = BOYSAW_ICON::BOYSAW_BOYSPH65;
        else if (boyshp_str == "4" && colour_str == "3,1" && colpat_str == "2") icon_type = BOYSAW_ICON::BOYSAW_BOYPIL73;
        else if (boyshp_str == "1" && colour_str == "3,1") icon_type = BOYSAW_ICON::BOYSAW_BOYCON78;
        else if (boyshp_str == "3" && colour_str == "3,1") icon_type = BOYSAW_ICON::BOYSAW_BOYSPH65;
        else if (boyshp_str == "4" && colour_str == "3,1") icon_type = BOYSAW_ICON::BOYSAW_BOYPIL73;
        else if (boyshp_str == "4" && colour_str == "3") icon_type = BOYSAW_ICON::BOYSAW_BOYPIL73;
        else if (boyshp_str == "3") icon_type = BOYSAW_ICON::BOYSAW_BOYSPH01;
        else if (boyshp_str == "4") icon_type = BOYSAW_ICON::BOYSAW_BOYPIL01;
        else if (boyshp_str == "5") icon_type = BOYSAW_ICON::BOYSAW_BOYSPR65;
        else if (boyshp_str == "6") icon_type = BOYSAW_ICON::BOYSAW_BOYBAR01;
        else if (boyshp_str == "7") icon_type = BOYSAW_ICON::BOYSAW_BOYSUP01;
        else if (boyshp_str == "8") icon_type = BOYSAW_ICON::BOYSAW_BOYSPR01;
    }
    if (shpName == "BOYSPP")
    {
        icon_type = BOYSPP_ICON::BOYSPP_BOYGEN03;

        std::string boyshp_str = get_attfs_value(point, "BOYSHP");
        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string colpat_str = get_attfs_value(point, "COLPAT");
        std::string catspm_str = get_attfs_value(point, "CATSPM");
        std::string convis_str = get_attfs_value(point, "CONVIS");
        std::string inform_str = get_attfs_value(point, "INFORM");

        if (boyshp_str == "1" && colour_str == "4,1,4,1,4" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCON65;
        else if (boyshp_str == "1" && colour_str == "5,3,1,5" && colpat_str == "1,2") icon_type = BOYSPP_ICON::BOYSPP_BOYCON81;
        else if (boyshp_str == "1" && colour_str == "1,11,1" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCON80;
        else if (boyshp_str == "2" && colour_str == "1,11,1" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN78;
        else if (boyshp_str == "3" && colour_str == "3,4,3" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYSPH66;
        else if (boyshp_str == "2" && colour_str == "1,11" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN77;
        else if (boyshp_str == "2" && colour_str == "11,1" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN81;
        else if (boyshp_str == "3" && colour_str == "1,11" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYSPH77;
        else if (boyshp_str == "4" && colour_str == "1,11" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL81;
        else if (boyshp_str == "1" && colour_str == "4,1" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCON73;
        else if (boyshp_str == "2" && colour_str == "3,1" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN80;
        else if (boyshp_str == "2" && colour_str == "3,4" && colpat_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN75;
        else if (boyshp_str == "4" && colour_str == "3,1" && colpat_str == "2") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL73;
        else if (boyshp_str == "4" && colour_str == "3,1" && colpat_str == "4") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL78;

        else if (boyshp_str == "7" && colpat_str == "1" && colour_str == "3,1") icon_type = BOYSPP_ICON::BOYSPP_BOYSUP66;
        else if (boyshp_str == "7" && colpat_str == "2" && colour_str == "3,1") icon_type = BOYSPP_ICON::BOYSPP_BOYSUP65;

        else if (catspm_str == "14" && boyshp_str == "2" && colour_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYMOR31;
        else if (catspm_str == "8" && boyshp_str == "2" && colour_str == "3") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN60;
        else if (boyshp_str == "1" && colour_str == "1,11") icon_type = BOYSPP_ICON::BOYSPP_BOYCON77;
        else if (boyshp_str == "2" && colour_str == "1,11") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN77;
        else if (boyshp_str == "3" && colour_str == "1,11") icon_type = BOYSPP_ICON::BOYSPP_BOYSPH77;
        else if (boyshp_str == "4" && colour_str == "1,11") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL81;
        else if (boyshp_str == "4" && colour_str == "4,3") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL74;
        else if (boyshp_str == "2" && colour_str == "11") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN79;
        else if (boyshp_str == "4" && colour_str == "11") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL59;
                    
        else if (catspm_str == "14" && boyshp_str == "2") icon_type = BOYSPP_ICON::BOYSPP_BOYMOR03;

        else if (boyshp_str == "1" && colour_str == "3") icon_type = BOYSPP_ICON::BOYSPP_BOYCON60;
        else if (boyshp_str == "1" && colour_str == "6") icon_type = BOYSPP_ICON::BOYSPP_BOYCON62;
        else if (boyshp_str == "2" && colour_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN65;
        else if (boyshp_str == "2" && colour_str == "2") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN64;
        else if (boyshp_str == "2" && colour_str == "3") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN60;
        else if (boyshp_str == "2" && colour_str == "4") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN61;
        else if (boyshp_str == "2" && colour_str == "6") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN63;

        else if (boyshp_str == "2" && inform_str == "white/orange") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN77;

        else if (boyshp_str == "3" && colour_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYSPH05;
        else if (boyshp_str == "3" && colour_str == "3") icon_type = BOYSPP_ICON::BOYSPP_BOYSPH60;
        else if (boyshp_str == "3" && colour_str == "6") icon_type = BOYSPP_ICON::BOYSPP_BOYSPH62;
        else if (boyshp_str == "4" && colour_str == "3") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL60;
        else if (boyshp_str == "4" && colour_str == "4") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL61;
        else if (boyshp_str == "4" && colour_str == "6") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL62;
        else if (boyshp_str == "5" && colour_str == "6") icon_type = BOYSPP_ICON::BOYSPP_BOYSPR62;
        else if (boyshp_str == "6" && colour_str == "3") icon_type = BOYSPP_ICON::BOYSPP_BOYBAR60;
        else if (boyshp_str == "6" && colour_str == "4") icon_type = BOYSPP_ICON::BOYSPP_BOYBAR61;
        else if (boyshp_str == "6" && colour_str == "6") icon_type = BOYSPP_ICON::BOYSPP_BOYBAR62;
        else if (boyshp_str == "7" && colour_str == "6") icon_type = BOYSPP_ICON::BOYSPP_BOYSUP62;

        else if (catspm_str == "15") icon_type = BOYSPP_ICON::BOYSPP_BOYSUP03;
        else if (boyshp_str == "1") icon_type = BOYSPP_ICON::BOYSPP_BOYCON01;
        else if (boyshp_str == "2") icon_type = BOYSPP_ICON::BOYSPP_BOYCAN62;
        else if (boyshp_str == "3") icon_type = BOYSPP_ICON::BOYSPP_BOYSPH01;
        else if (boyshp_str == "4") icon_type = BOYSPP_ICON::BOYSPP_BOYPIL01;
        else if (boyshp_str == "5") icon_type = BOYSPP_ICON::BOYSPP_BOYSPR01;
        else if (boyshp_str == "6") icon_type = BOYSPP_ICON::BOYSPP_BOYBAR01;
        else if (boyshp_str == "7") icon_type = BOYSPP_ICON::BOYSPP_BOYSUP01;
        else if (boyshp_str == "8") icon_type = BOYSPP_ICON::BOYSPP_BOYSPR01;
        else if (boyshp_str == "9") icon_type = BOYSPP_ICON::BOYSPP_BOYSUP01;
        else if (colour_str == "6") icon_type = BOYSPP_ICON::BOYSPP_BOYSPH62;
        else icon_type = BOYSPP_ICON::BOYSPP_BOYGEN03;
    }
    if (shpName == "BUISGL")
    {
        icon_type = BUISGL_ICON::BUISGL_BUISGL01;
        std::string functn_str = get_attfs_value(point, "FUNCTN");
        std::string convis_str = get_attfs_value(point, "CONVIS");
					
		std::string objnam = get_attfs_value(point, "OBJNAM");
		std::string nobjnm = get_natfs_value(point, "NOBJNM");

        if (functn_str == "33" && convis_str == "1" && (objnam!="" || nobjnm!="")) icon_type = BUISGL_ICON::BUISGL_POSGEN03;
        else if (functn_str == "20" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_BUIREL13;
        else if (functn_str == "21" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_BUIREL13;
        else if (functn_str == "22" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_BUIREL14;
        else if (functn_str == "23" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_BUIREL14;
        else if (functn_str == "24" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_BUIREL14;
        else if (functn_str == "25" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_BUIREL14;
        else if (functn_str == "26" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_BUIREL15;
        else if (functn_str == "27" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_BUIREL15;
        else if (functn_str == "33" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_POSGEN03;
        else if (functn_str == "35" && convis_str == "1") icon_type = BUISGL_ICON::BUISGL_TNKCON12;
        else if (functn_str == "33" && (objnam != "" || nobjnm != "")) icon_type = BUISGL_ICON::BUISGL_POSGEN03;

        else if (functn_str == "20") icon_type = BUISGL_ICON::BUISGL_BUIREL01;
        else if (functn_str == "21") icon_type = BUISGL_ICON::BUISGL_BUIREL01;
        else if (functn_str == "22") icon_type = BUISGL_ICON::BUISGL_BUIREL04;
        else if (functn_str == "23") icon_type = BUISGL_ICON::BUISGL_BUIREL04;
        else if (functn_str == "24") icon_type = BUISGL_ICON::BUISGL_BUIREL04;
        else if (functn_str == "25") icon_type = BUISGL_ICON::BUISGL_BUIREL04;
        else if (functn_str == "26") icon_type = BUISGL_ICON::BUISGL_BUIREL05;
        else if (functn_str == "27") icon_type = BUISGL_ICON::BUISGL_BUIREL05;
        else if (functn_str == "33") icon_type = BUISGL_ICON::BUISGL_POSGEN03;
        else if (functn_str == "35") icon_type = BUISGL_ICON::BUISGL_TNKCON02;

        else if (convis_str == "1") icon_type = BUISGL_ICON::BUISGL_BUISGL11;
        else icon_type = 0;
    }
    if (shpName == "CHKPNT")
    {
        icon_type = CHKPNT_ICON::CHKPNT_POSGEN04;

        std::string catchp_str = get_attfs_value(point, "CATCHP");
        if (catchp_str == "1") icon_type = CHKPNT_ICON::CHKPNT_CUSTOM01;
        else if (catchp_str == "2")icon_type = CHKPNT_ICON::CHKPNT_BORDER01;
    }
    if (shpName == "CURENT")
    {
        std::string orient_str = get_attfs_value(point, "ORIENT");
        if (orient_str != "") icon_type = 1;
        else icon_type = 0;
    }
    if (shpName == "DAMCON")
    {
        std::string catdam_str = get_attfs_value(point, "CATDAM");
        if (catdam_str == "3")  icon_type = 3;
    }
    if (shpName == "DAYMAR")
    {
        icon_type = icon_type = DAYMAR_ICON::DAYMAR_DAYSQR21;
        std::string colpat_str = get_attfs_value(point, "COLPAT");
        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string topshp_str = get_attfs_value(point, "TOPSHP");

        if (topshp_str == "12" && colpat_str == "6,4" && colour_str == "1,1,2,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP77;
        else if (topshp_str == "12" && colpat_str == "6,4" && colour_str == "1,1,3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP76;
        else if (topshp_str == "12" && colpat_str == "6,4" && colour_str == "1,1,6,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP78;
        else if (topshp_str == "21" && colpat_str == "5,2" && colour_str == "3,1,3,6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP96;
        else if (topshp_str == "12" && colpat_str == "4" && colour_str == "3,1,1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP74;
        else if (topshp_str == "12" && colpat_str == "6,4" && colour_str == "1,2,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP73;
        else if (topshp_str == "12" && colpat_str == "6,4" && colour_str == "1,3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP72;
        else if (topshp_str == "12" && colpat_str == "6,4" && colour_str == "1,4,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP64;
        else if (topshp_str == "12" && colpat_str == "6" && colour_str == "1,1,3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP76;
        else if (topshp_str == "19" && colpat_str == "6,1" && colour_str == "4,3,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP43;
        else if (topshp_str == "19" && colpat_str == "6,1" && colour_str == "4,4,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP43;
        else if (topshp_str == "21" && colpat_str == "5,2" && colour_str == "3,1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP90;
        else if (topshp_str == "24" && colpat_str == "2" && colour_str == "11,2,11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP17;
        else if (topshp_str == "24" && colpat_str == "6,1" && colour_str == "3,3,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP09;
        else if (topshp_str == "24" && colpat_str == "6,1" && colour_str == "3,3,6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP08;
        else if (topshp_str == "33" && colpat_str == "6,5" && colour_str == "1,1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP00;
        else if (topshp_str == "19" && colpat_str == "2" && colour_str == "1,11,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP34;
        else if (topshp_str == "19" && colpat_str == "6,4" && colour_str == "11,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP38;
        else if (topshp_str == "12" && colpat_str == "6" && colour_str == "1,3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP63;
        else if (topshp_str == "19" && colpat_str == "1" && colour_str == "11,11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP41;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "3,4,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP29;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "4,3,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP33;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "4,4,6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP30;
        else if (topshp_str == "20" && colpat_str == "2" && colour_str == "3,1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP89;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "1,2,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA5;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "1,3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP94;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "2,1,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA1;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "3,1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP90;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "3,2,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA0;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "3,4,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA3;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "4,3,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA7;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "6,2,6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP97;
        else if (topshp_str == "24" && colpat_str == "6" && colour_str == "3,3,6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP15;
        else if (topshp_str == "33" && colpat_str == "6" && colour_str == "1,1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP00;
        else if (topshp_str == "33" && colpat_str == "6" && colour_str == "1,3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPS1;
        else if (topshp_str == "12" && colpat_str == "2" && colour_str == "1,11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP62;
        else if (topshp_str == "12" && colpat_str == "2" && colour_str == "11,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP61;
        else if (topshp_str == "12" && colpat_str == "6" && colour_str == "1,11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP71;
        else if (topshp_str == "12" && colpat_str == "6" && colour_str == "11,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP67;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "1,11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP25;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "11,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP31;
        else if (topshp_str == "20" && colpat_str == "1" && colour_str == "11,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP84;
        else if (topshp_str == "20" && colpat_str == "2" && colour_str == "11,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP81;
        else if (topshp_str == "20" && colpat_str == "6" && colour_str == "11,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP79;
        else if (topshp_str == "22" && colpat_str == "2" && colour_str == "1,11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP03;
        else if (topshp_str == "23" && colpat_str == "2" && colour_str == "1,11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP01;
        else if (topshp_str == "24" && colpat_str == "2" && colour_str == "11,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP13;
        else if (topshp_str == "25" && colpat_str == "2" && colour_str == "11,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP05;
        else if (topshp_str == "6" && colpat_str == "2" && colour_str == "1,2,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPT8;
        else if (topshp_str == "12" && colpat_str == "1" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP58;
        else if (topshp_str == "12" && colpat_str == "2" && colour_str == "1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP65;
        else if (topshp_str == "12" && colpat_str == "4" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP74;
        else if (topshp_str == "12" && colpat_str == "6" && colour_str == "1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP69;
        else if (topshp_str == "12" && colpat_str == "6" && colour_str == "6,6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP70;
        else if (topshp_str == "19" && colpat_str == "1" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP42;
        else if (topshp_str == "19" && colpat_str == "2" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP32;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "1,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP40;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPMA107;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "2,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP37;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "4,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP48;
        else if (topshp_str == "19" && colpat_str == "6" && colour_str == "6,6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP44;
        else if (topshp_str == "20" && colpat_str == "1" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP85;
        else if (topshp_str == "20" && colpat_str == "2" && colour_str == "1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP87;
        else if (topshp_str == "20" && colpat_str == "2" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP88;
        else if (topshp_str == "20" && colpat_str == "6" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP80;
        else if (topshp_str == "21" && colpat_str == "1" && colour_str == "1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA9;
        else if (topshp_str == "21" && colpat_str == "1" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPB0;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "1,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP99;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "1,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP92;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "1,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP93;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP91;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "3,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP98;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "3,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA2;
        else if (topshp_str == "21" && colpat_str == "2" && colour_str == "4,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA8;
        else if (topshp_str == "21" && colpat_str == "6" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA4;
        else if (topshp_str == "22" && colpat_str == "2" && colour_str == "3,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP04;
        else if (topshp_str == "23" && colpat_str == "2" && colour_str == "3,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP02;
        else if (topshp_str == "24" && colpat_str == "6" && colour_str == "3,1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP16;
        else if (topshp_str == "24" && colpat_str == "6" && colour_str == "3,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP20;
        else if (topshp_str == "24" && colpat_str == "6" && colour_str == "3,6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP19;
        else if (topshp_str == "24" && colpat_str == "6" && colour_str == "4,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPU1;
        else if (topshp_str == "24" && colpat_str == "6" && colour_str == "6,6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP18;
        else if (topshp_str == "12" && colpat_str == "6" && colour_str == "11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP55;
        else if (topshp_str == "3" && colpat_str == "6" && colour_str == "4,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPD2;
        else if (topshp_str == "12" && colour_str == "1,4" && colpat_str == "6") icon_type = DAYMAR_ICON::DAYMAR_TOPMA109;
        else if (topshp_str == "20" && colour_str == "1,2" && colpat_str == "4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP83;
        else if (topshp_str == "19" && colour_str == "4,1,2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP28;
        else if (topshp_str == "10" && colour_str == "3,4") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
        else if (topshp_str == "19" && colour_str == "3,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP47;
        else if (topshp_str == "19" && colour_str == "4,4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP48;
        else if (topshp_str == "24" && colour_str == "3,3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP20;
        else if (topshp_str == "12" && colour_str == "11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP55;
        else if (topshp_str == "19" && colour_str == "11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP36;
        else if (topshp_str == "24" && colour_str == "11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP11;
        else if (topshp_str == "26" && colour_str == "11") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPD1;
        else if (topshp_str == "12" && colour_str == "1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP51;
        else if (topshp_str == "19" && colour_str == "1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP21;
        else if (topshp_str == "19" && colour_str == "2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP23;
        else if (topshp_str == "19" && colour_str == "3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP22;
        else if (topshp_str == "19" && colour_str == "4") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP24;
        else if (topshp_str == "19" && colour_str == "6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP35;
        else if (topshp_str == "20" && colour_str == "1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP83;
        else if (topshp_str == "20" && colour_str == "2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP82;
        else if (topshp_str == "21" && colour_str == "1") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP95;
        else if (topshp_str == "21" && colour_str == "3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPA6;
        else if (topshp_str == "24" && colour_str == "3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP12;
        else if (topshp_str == "24" && colour_str == "4") icon_type = DAYMAR_ICON::DAYMAR_TOPMA102;
        else if (topshp_str == "24" && colour_str == "6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHP10;
        else if (topshp_str == "25" && colour_str == "3") icon_type = DAYMAR_ICON::DAYMAR_TOPMA100;
        else if (topshp_str == "1" && colour_str == "3") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
        else if (topshp_str == "3" && colour_str == "3") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPD3;
        else if (topshp_str == "7" && colour_str == "6") icon_type = DAYMAR_ICON::DAYMAR_TOPMA113;
        else if (topshp_str == "8" && colour_str == "6") icon_type = DAYMAR_ICON::DAYMAR_TOPMA111;
        else if (topshp_str == "2" && colour_str == "3") icon_type = DAYMAR_ICON::DAYMAR_TOPMA100;
        else if (topshp_str == "1" && colour_str == "4") icon_type = DAYMAR_ICON::DAYMAR_TOPMA102;
        else if (topshp_str == "7" && colour_str == "2") icon_type = DAYMAR_ICON::DAYMAR_TOPSHPI2;
        else icon_type = -1;

        if (icon_type < 0)
        {
            if (topshp_str == "10") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "11") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "12") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "13") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "14") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "15") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ15;
            else if (topshp_str == "16") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ16;
            else if (topshp_str == "17") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ17;
            else if (topshp_str == "18") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ18;
            else if (topshp_str == "19") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ19;
            else if (topshp_str == "20") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ20;
            else if (topshp_str == "21") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ21;
            else if (topshp_str == "22") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ22;
            else if (topshp_str == "23") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ23;
            else if (topshp_str == "24") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ24;
            else if (topshp_str == "25") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ25;
            else if (topshp_str == "26") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ26;
            else if (topshp_str == "27") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ27;
            else if (topshp_str == "28") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ28;
            else if (topshp_str == "29") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ29;
            else if (topshp_str == "30") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ30;
            else if (topshp_str == "31") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ31;
            else if (topshp_str == "32") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ32;
            else if (topshp_str == "33") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "1") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "2") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "3") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "4") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "5") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else if (topshp_str == "6") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ06;
            else if (topshp_str == "7") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ07;
            else if (topshp_str == "8") icon_type = DAYMAR_ICON::DAYMAR_TOPSHQ08;
            else if (topshp_str == "9") icon_type = DAYMAR_ICON::DAYMAR_TOPMAR01;
            else icon_type = DAYMAR_ICON::DAYMAR_DAYSQR21;
        }
    }
    if (shpName == "DISMAR")
    {
        icon_type = 0;

        std::string catdis_str = get_attfs_value(point, "CATDIS");
        std::string hunits_str = get_attfs_value(point, "HUNITS");

        if (catdis_str == "3" && hunits_str == "3")  icon_type = DISMAR_ICON::DISMAR_HECMTR02;
        else if (catdis_str == "3" && hunits_str == "4")  icon_type = DISMAR_ICON::DISMAR_HECMTR01;
        else if (catdis_str == "5")  icon_type = DISMAR_ICON::DISMAR_HECMTR02;
        else if (catdis_str == "6")  icon_type = DISMAR_ICON::DISMAR_HECMTR01;
    }
    if (shpName == "FSHFAC")
    {
        icon_type = 0;
        std::string catfif_str = get_attfs_value(point, "CATFIF");

        if (catfif_str == "1") icon_type = FSHFAC_ICON::FSHFAC_FSHFAC03;
        else if (catfif_str == "2") icon_type = FSHFAC_ICON::FSHFAC_FSHFAC02;
        else if (catfif_str == "3") icon_type = FSHFAC_ICON::FSHFAC_FSHFAC02;
        else if (catfif_str == "4") icon_type = FSHFAC_ICON::FSHFAC_FSHFAC02;
        else icon_type = FSHFAC_ICON::FSHFAC_FSHHAV01;
    }
    if (shpName == "HRBFAC")
    {
        icon_type = HRBFAC_ICON::HRBFAC_CHINFO07;
        std::string cathaf_str = get_attfs_value(point, "CATHAF");
                    
        if (cathaf_str == "1") icon_type = HRBFAC_ICON::HRBFAC_ROLROL01;
        else if (cathaf_str == "4") icon_type = HRBFAC_ICON::HRBFAC_HRBFAC09;
        else if (cathaf_str == "5") icon_type = HRBFAC_ICON::HRBFAC_SMCFAC02;
        else icon_type = HRBFAC_ICON::HRBFAC_CHINFO07;
    }
    if (shpName == "LIGHTS")
    {
        icon_type = -1;
        std::string catlit_str = get_attfs_value(point, "CATLIT");
        std::string valnmr_str = get_attfs_value(point, "VALNMR");
        std::string litvis_str = get_attfs_value(point, "LITVIS");
        std::string colour_str = get_attfs_value(point, "COLOUR");

        if (catlit_str != "")
        {
            char catlit[1024] = { '\0' };
            _parseList(catlit_str.c_str(), catlit, sizeof(catlit));
            if (STRPBRK(catlit, "\010\013")) icon_type = LIGHTS_ICON::LIGHTS_LIGHTS82;
            if (STRPBRK(catlit, "\011")) icon_type = LIGHTS_ICON::LIGHTS_LIGHTS81;
        }
                    
        if (icon_type<0)
        {
            char colist[1024] = { '\0' };
            if (colour_str != "")
                _parseList(colour_str.c_str(), colist, sizeof(colist));
            else {
                colist[0] = '\014';  // magenta (12)
                colist[1] = '\000';
            }

            std::string sectr1_str = get_attfs_value(point, "SECTR1");
            std::string sectr2_str = get_attfs_value(point, "SECTR2");
            if (sectr1_str == "" || sectr2_str == "")
            {
                double valnmr = atof(valnmr_str.c_str());
                if (valnmr < 10.0)
                {
                    icon_type = LIGHTS_ICON::LIGHTS_LITDEF11; // default

                    // max 1 color
                    if ('\0' == colist[1]) {
                        if (STRPBRK(colist, "\003"))
                            icon_type = LIGHTS_ICON::LIGHTS_LIGHTS11;
                        else if (STRPBRK(colist, "\004"))
                            icon_type = LIGHTS_ICON::LIGHTS_LIGHTS12;
                        else if (STRPBRK(colist, "\001\006\011"))
                            icon_type = LIGHTS_ICON::LIGHTS_LIGHTS13;
                    }
                    else {
                        // max 2 color
                        if ('\0' == colist[2]) {
                            if (STRPBRK(colist, "\001") && STRPBRK(colist, "\003"))
                                icon_type = LIGHTS_ICON::LIGHTS_LIGHTS11;
                            else if (STRPBRK(colist, "\001") && STRPBRK(colist, "\004"))
                                icon_type = LIGHTS_ICON::LIGHTS_LIGHTS12;
                        }
                    }
                }
                else //! ����Բ
                {
                    int radius = 3;
                    if (valnmr > 0) 
                    {
                        if (valnmr < 7.0) radius = 3;
                        else if (valnmr < 15.0) radius = 10;
                        else if (valnmr < 30.0) radius = 15;
                        else radius = 20;
                    }

                    // max 1 color
                    if ('\0' == colist[1]) 
                    {
                        if (STRPBRK(colist, "\003"))
                        {
                            icon_type = 101; // LITRD
                            radius += 1;
                        }
                        else if (STRPBRK(colist, "\004"))
                        {
                            icon_type = 102; // LITGN
                        }
                        else if (STRPBRK(colist, "\001\006\011"))
                        {
                            icon_type = 103; // LITYW
                            radius += 2;
                        }
                        else if (STRPBRK(colist, "\014"))
                        {
                            icon_type = 104; // CHMGD
                            radius += 3;
                        }
                        else
                        {
                            icon_type = 104; // CHMGD
                            radius += 5;
                        }
                    }
                    else if ('\0' == colist[2])  // or 2 color
                    {
                        if (STRPBRK(colist, "\001") && STRPBRK(colist, "\003"))
                        {
                            icon_type = 101; // LITRD
                            radius += 1;
                        }
                        else if (STRPBRK(colist, "\001") && STRPBRK(colist, "\004"))
                        {
                            icon_type = 102; // LITGN
                        }
                        else
                        {
                            icon_type = 104; // CHMGD
                            radius += 5;
                        }
                    }
                    else
                    {
                        icon_type = 104; // CHMGD
                        radius += 5;
                    }
                }
            }
            else // ��Բ
            {
                icon_type = 200;

                double sectr1 = std::stod(sectr1_str);
                double sectr2 = std::stod(sectr2_str);

                //if (sectr2 <= sectr1) sectr2 += 360;

                //if (sectr1 > 180) sectr1 -= 180;
                //else sectr1 += 180;

                //if (sectr2 > 180) sectr2 -= 180;
                //else sectr2 += 180;

                //if (sectr2 > 180) sectr1 -= 180;


                // max 1 color
                if ('\0' == colist[1])
                {
                    if (STRPBRK(colist, "\003"))
                    {
                        icon_type = 201; // LITRD
                    }
                    else if (STRPBRK(colist, "\004"))
                    {
                        icon_type = 202; // LITGN
                    }
                    else if (STRPBRK(colist, "\001\006\013"))
                    {
                        icon_type = 203; // LITYW
                    }
                    else
                    {
                        icon_type = 204; // CHMGD
                    }
                }
                else if ('\0' == colist[2])  // or 2 color
                {
                    if (STRPBRK(colist, "\001") && STRPBRK(colist, "\003"))
                    {
                        icon_type = 201; // LITRD
                    }
                    else if (STRPBRK(colist, "\001") && STRPBRK(colist, "\004"))
                    {
                        icon_type = 202; // LITGN
                    }
                    else
                    {
                        icon_type = 204; // CHMGD
                    }
                }
                else
                {
                    icon_type = 204; // CHMGD
                }
            }
        }
    }
    if (shpName == "LITFLT")
    {
        icon_type = LITFLT_ICON::LITFLT_LITFLT01;

        std::string colour_str = get_attfs_value(point, "COLOUR");
        if (colour_str == "3,1") icon_type = LITFLT_ICON::LITFLT_LITFLT10;
        else if (colour_str == "4") icon_type = LITFLT_ICON::LITFLT_LITFLT61;
        else icon_type = LITFLT_ICON::LITFLT_LITFLT01;
    }
    if (shpName == "LITVES")
    {
        icon_type = LITVES_ICON::LITVES_LITVES01;

        std::string colour_str = get_attfs_value(point, "COLOUR");
        if (colour_str == "3") icon_type = LITVES_ICON::LITVES_LITVES60;
        else if (colour_str == "4") icon_type = LITVES_ICON::LITVES_LITVES61;
        else icon_type = LITVES_ICON::LITVES_LITVES01;
    }
    if (shpName == "LNDMRK")
    {
        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string colpat_str = get_attfs_value(point, "COLPAT");
        std::string catlmk_str = get_attfs_value(point, "CATLMK");
        std::string functn_str = get_attfs_value(point, "FUNCTN");
        std::string convis_str = get_attfs_value(point, "CONVIS");

        icon_type = LNDMRK_ICON::LNDMRK_POSGEN03;

        if (catlmk_str == "17" && colpat_str == "1" && functn_str == "33" && colour_str == "1,2,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS51;
        else if (catlmk_str == "17" && colpat_str == "1" && functn_str == "33" && colour_str == "1,3,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS72;
        else if (catlmk_str == "17" && colpat_str == "1" && functn_str == "33" && colour_str == "2,1,2") icon_type = LNDMRK_ICON::LNDMRK_TOWERS92;
        else if (catlmk_str == "17" && colpat_str == "1" && functn_str == "33" && colour_str == "2,1,7") icon_type = LNDMRK_ICON::LNDMRK_TOWERS93;
        else if (catlmk_str == "17" && colpat_str == "1" && functn_str == "33" && colour_str == "2,3,2") icon_type = LNDMRK_ICON::LNDMRK_TOWERS73;
        else if (catlmk_str == "17" && colpat_str == "1" && functn_str == "33" && colour_str == "1,2") icon_type = LNDMRK_ICON::LNDMRK_TOWERS79;
        else if (catlmk_str == "17" && colpat_str == "1" && functn_str == "33" && colour_str == "1,4") icon_type = LNDMRK_ICON::LNDMRK_TOWERS48;
        else if (catlmk_str == "17" && colpat_str == "1" && functn_str == "33" && colour_str == "2,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS83;
        else if (catlmk_str == "17" && colpat_str == "3" && functn_str == "33" && colour_str == "2,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS94;
        else if (catlmk_str == "17" && colpat_str == "4" && functn_str == "33" && colour_str == "3,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS98;
        else if (catlmk_str == "17" && colpat_str == "1" && colour_str == "3,1,3,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS85;
        else if (catlmk_str == "17" && colpat_str == "1" && colour_str == "1,6,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS67;
        else if (catlmk_str == "17" && colpat_str == "1" && colour_str == "1,7,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS66;
        else if (catlmk_str == "17" && colpat_str == "1" && colour_str == "1,2") icon_type = LNDMRK_ICON::LNDMRK_TOWERS88;
        else if (catlmk_str == "17" && colpat_str == "1" && colour_str == "1,3") icon_type = LNDMRK_ICON::LNDMRK_TOWERS52;
        else if (catlmk_str == "17" && colpat_str == "1" && colour_str == "2,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS83;
        else if (catlmk_str == "17" && colpat_str == "1" && colour_str == "3,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS53;
        else if (catlmk_str == "17" && colpat_str == "2" && colour_str == "1,2") icon_type = LNDMRK_ICON::LNDMRK_TOWERS96;
        else if (catlmk_str == "17" && colpat_str == "2" && colour_str == "1,3") icon_type = LNDMRK_ICON::LNDMRK_TOWERS50;
        else if (catlmk_str == "17" && colpat_str == "2" && colour_str == "2,1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS49;
        else if (catlmk_str == "17" && colpat_str == "3" && colour_str == "1,2") icon_type = LNDMRK_ICON::LNDMRK_TOWERS97;
        else if (catlmk_str == "15" && functn_str == "20" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_BUIREL13;
        else if (catlmk_str == "15" && functn_str == "21" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_BUIREL13;
        else if (catlmk_str == "17" && functn_str == "17" && colour_str == "7") icon_type = LNDMRK_ICON::LNDMRK_TOWERS65;
        else if (catlmk_str == "17" && functn_str == "20" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_BUIREL13;
        else if (catlmk_str == "17" && functn_str == "21" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_BUIREL13;
        else if (catlmk_str == "17" && functn_str == "33" && colour_str == "1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS05;
        else if (catlmk_str == "17" && functn_str == "33" && colour_str == "3") icon_type = LNDMRK_ICON::LNDMRK_TOWERS60;
        else if (catlmk_str == "17" && functn_str == "33" && colour_str == "4") icon_type = LNDMRK_ICON::LNDMRK_TOWERS61;
        else if (catlmk_str == "17" && functn_str == "33" && colour_str == "6") icon_type = LNDMRK_ICON::LNDMRK_TOWERS62;
        else if (catlmk_str == "17" && functn_str == "33" && colour_str == "7") icon_type = LNDMRK_ICON::LNDMRK_TOWERS65;
        else if (catlmk_str == "17" && functn_str == "33" && colour_str == "8") icon_type = LNDMRK_ICON::LNDMRK_TOWERS59;
        else if (catlmk_str == "17" && functn_str == "33" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS03;
        else if (catlmk_str == "20" && functn_str == "20" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_BUIREL13;
        else if (catlmk_str == "20" && functn_str == "21" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_BUIREL13;
        else if (catlmk_str == "20" && functn_str == "26" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_BUIREL15;
        else if (catlmk_str == "20" && functn_str == "27" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_BUIREL15;
        else if (catlmk_str == "17" && functn_str == "31,33") icon_type = LNDMRK_ICON::LNDMRK_TOWERS15;
        else if (catlmk_str == "17" && functn_str == "20") icon_type = LNDMRK_ICON::LNDMRK_BUIREL13;
        else if (catlmk_str == "17" && functn_str == "31") icon_type = LNDMRK_ICON::LNDMRK_TOWERS15;
        else if (catlmk_str == "17" && functn_str == "33") icon_type = LNDMRK_ICON::LNDMRK_TOWERS01;
        else if (catlmk_str == "20" && functn_str == "20") icon_type = LNDMRK_ICON::LNDMRK_BUIREL01;
        else if (catlmk_str == "10" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_MONUMT12;
        else if (catlmk_str == "12" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_MONUMT12;
        else if (catlmk_str == "13" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_MONUMT12;
        else if (catlmk_str == "15" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_DOMES011;
        else if (catlmk_str == "16" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_RASCAN11;
        else if (catlmk_str == "17" && colour_str == "1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS05;
        else if (catlmk_str == "17" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_TOWERS03;
        else if (catlmk_str == "18" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_WNDMIL12;
        else if (catlmk_str == "19" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_WIMCON11;
        else if (catlmk_str == "20" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_POSGEN03;
        else if (catlmk_str == "1" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_CAIRNS11;
        else if (catlmk_str == "3" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_CHIMNY11;
        else if (catlmk_str == "4" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_DSHAER11;
        else if (catlmk_str == "5" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_FLGSTF01;
        else if (catlmk_str == "6" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_FLASTK11;
        else if (catlmk_str == "7" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_MSTCON14;
        else if (catlmk_str == "8" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_POSGEN03;
        else if (catlmk_str == "9" && convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_MONUMT12;
        else if (catlmk_str == "10") icon_type = LNDMRK_ICON::LNDMRK_MONUMT02;
        else if (catlmk_str == "12") icon_type = LNDMRK_ICON::LNDMRK_MONUMT02;
        else if (catlmk_str == "13") icon_type = LNDMRK_ICON::LNDMRK_MONUMT02;
        else if (catlmk_str == "15") icon_type = LNDMRK_ICON::LNDMRK_DOMES001;
        else if (catlmk_str == "16") icon_type = LNDMRK_ICON::LNDMRK_RASCAN01;
        else if (catlmk_str == "17") icon_type = LNDMRK_ICON::LNDMRK_TOWERS01;
        else if (catlmk_str == "18") icon_type = LNDMRK_ICON::LNDMRK_WNDMIL02;
        else if (catlmk_str == "19") icon_type = LNDMRK_ICON::LNDMRK_WIMCON01;
        else if (catlmk_str == "20") icon_type = LNDMRK_ICON::LNDMRK_POSGEN01;
        else if (catlmk_str == "1") icon_type = LNDMRK_ICON::LNDMRK_CAIRNS01;
        else if (catlmk_str == "3") icon_type = LNDMRK_ICON::LNDMRK_CHIMNY01;
        else if (catlmk_str == "4") icon_type = LNDMRK_ICON::LNDMRK_DSHAER01;
        else if (catlmk_str == "5") icon_type = LNDMRK_ICON::LNDMRK_FLGSTF01;
        else if (catlmk_str == "6") icon_type = LNDMRK_ICON::LNDMRK_FLASTK01;
        else if (catlmk_str == "7") icon_type = LNDMRK_ICON::LNDMRK_MSTCON04;
        else if (catlmk_str == "8") icon_type = LNDMRK_ICON::LNDMRK_POSGEN03;
        else if (catlmk_str == "9") icon_type = LNDMRK_ICON::LNDMRK_MONUMT02;
        else if (convis_str == "1") icon_type = LNDMRK_ICON::LNDMRK_POSGEN03;
        else icon_type = LNDMRK_ICON::LNDMRK_POSGEN01;
    }
    if (shpName == "MORFAC")
    {
        icon_type = MORFAC_ICON::MORFAC_MORFAC03;
        std::string catmor_str = get_attfs_value(point, "CATMOR");
        std::string boyshp_str = get_attfs_value(point, "BOYSHP");
        if (catmor_str == "7" && boyshp_str == "3") icon_type = MORFAC_ICON::MORFAC_BOYMOR01;
        else if (catmor_str == "7" && boyshp_str == "6") icon_type = MORFAC_ICON::MORFAC_BOYMOR03;
        else if (catmor_str == "1") icon_type = MORFAC_ICON::MORFAC_MORFAC03;
        else if (catmor_str == "2") icon_type = MORFAC_ICON::MORFAC_MORFAC04;
        else if (catmor_str == "3") icon_type = MORFAC_ICON::MORFAC_PILPNT02;
        else if (catmor_str == "5") icon_type = MORFAC_ICON::MORFAC_PILPNT02;
        else if (catmor_str == "7") icon_type = MORFAC_ICON::MORFAC_BOYMOR11;
        else icon_type = MORFAC_ICON::MORFAC_MORFAC03;
    }
    if (shpName == "NEWOBJ")
    {
        std::string symins_str = get_attfs_value(point, "SYMINS");
        if (symins_str!="") icon_type = 1;
        else icon_type = 2;
    }
    if (shpName == "OBSTRN")
    {
        icon_type = 0;
        std::string valsou_str = get_attfs_value(point, "VALSOU");
        std::string catobs_str = get_attfs_value(point, "CATOBS");
        std::string watlev_str = get_attfs_value(point, "WATLEV");

        if (catobs_str == "10" && valsou_str != "") icon_type = OBSTRN_ICON::OBSTRN_FLTHAZ02;
        else if (catobs_str == "7" && valsou_str != "") icon_type = OBSTRN_ICON::OBSTRN_FOULGND1;
        else if (catobs_str == "8" && valsou_str != "") icon_type = OBSTRN_ICON::OBSTRN_FLTHAZ02;
        else if (catobs_str == "9" && valsou_str != "") icon_type = OBSTRN_ICON::OBSTRN_ACHARE02;
        else if (catobs_str == "10") icon_type = OBSTRN_ICON::OBSTRN_FLTHAZ02;
        else if (catobs_str == "7") icon_type = OBSTRN_ICON::OBSTRN_FOULGND1;
        else if (catobs_str == "8") icon_type = OBSTRN_ICON::OBSTRN_FLTHAZ02;
        else if (catobs_str == "9") icon_type = OBSTRN_ICON::OBSTRN_ACHARE02;
        else if (watlev_str == "7") icon_type = OBSTRN_ICON::OBSTRN_FLTHAZ02;
        else
        {
            if (valsou_str != "")
            {
                int watlev = -9;
                if (watlev_str != "") watlev = atoi(watlev_str.c_str());
                double valsou = atof(valsou_str.c_str());
                bool sounding = false;
                if (valsou <= 20.0) {
                    if (-9 == watlev) {  // default
                        icon_type = OBSTRN_ICON::OBSTRN_DANGER01;
                        sounding = true;
                    }
                    else {
                        switch (watlev) {
                        case 1:
                        case 2:
                            icon_type = OBSTRN_ICON::OBSTRN_LNDARE01;
                            sounding = false;
                            break;
                        case 3:
                            icon_type = OBSTRN_ICON::OBSTRN_DANGER52;
                            sounding = true;
                            break;
                        case 4:
                        case 5:
                            icon_type = OBSTRN_ICON::OBSTRN_DANGER53;
                            sounding = true;
                            break;
                        default:
                            icon_type = OBSTRN_ICON::OBSTRN_DANGER51;
                            sounding = true;
                            break;
                        }
                    }
                }
                else {  // valsou > 20.0
                    icon_type = OBSTRN_ICON::OBSTRN_DANGER52;
                    sounding = true;
                }
            }
            else
            {
                int watlev = -9;
                if (watlev_str != "") watlev = atoi(watlev_str.c_str());
                if (-9 == watlev)  // default
                    icon_type = OBSTRN_ICON::OBSTRN_OBSTRN01;
                else {
                    switch (watlev) {
                    case 1:
                        icon_type = OBSTRN_ICON::OBSTRN_OBSTRN11;
                        break;
                    case 2:
                        icon_type = OBSTRN_ICON::OBSTRN_OBSTRN11;
                        break;
                    case 3:
                        icon_type = OBSTRN_ICON::OBSTRN_OBSTRN01;
                        //icon_type = OBSTRN_ICON::OBSTRN_ISODGR51;
                        break;
                    case 4:
                        icon_type = OBSTRN_ICON::OBSTRN_OBSTRN03;
                        break;
                    case 5:
                        icon_type = OBSTRN_ICON::OBSTRN_OBSTRN03;
                        break;
                    default:
                        icon_type = OBSTRN_ICON::OBSTRN_OBSTRN01;
                        break;
                    }
                }
            }
         
        }
    }
    if (shpName == "PRDARE")
    {
        icon_type = 0;

        std::string catpra_str = get_attfs_value(point, "CATPRA");
        std::string convis_str = get_attfs_value(point, "CONVIS");

        if (catpra_str == "5" && convis_str == "1") icon_type = PRDARE_ICON::PRDARE_FLASTK11;
        else if (catpra_str == "8" && convis_str == "1") icon_type = PRDARE_ICON::PRDARE_TNKCON12;
        else if (catpra_str == "9" && convis_str == "1") icon_type = PRDARE_ICON::PRDARE_WIMCON11;
    }
    if (shpName == "RADSTA")
    {
        icon_type = 0;

        std::string catras_str = get_attfs_value(point, "CATRAS");
        if (catras_str == "2") icon_type = RADSTA_ICON::RADSTA_RDOSTA02;
        else icon_type = RADSTA_ICON::RADSTA_POSGEN01;
    }
    if (shpName == "RDOCAL")
    {
        icon_type = RDOCAL_ICON::RDOCAL_RCLDEF01;
        std::string trafic_str = get_attfs_value(point, "TRAFIC");
        std::string orient_str = get_attfs_value(point, "ORIENT");

        if (trafic_str == "1" && orient_str != "") icon_type = RDOCAL_ICON::RDOCAL_RDOCAL02;
        else if (trafic_str == "2" && orient_str != "") icon_type = RDOCAL_ICON::RDOCAL_RDOCAL02;
        else if (trafic_str == "3" && orient_str != "") icon_type = RDOCAL_ICON::RDOCAL_RDOCAL02;
        else if (trafic_str == "4" && orient_str != "") icon_type = RDOCAL_ICON::RDOCAL_RDOCAL03;
        else icon_type = RDOCAL_ICON::RDOCAL_RCLDEF01;
    }
    if (shpName == "RCTLPT")
    {
        icon_type = RCTLPT_ICON::RCTLPT_RTLDEF51;
        std::string orient_str = get_attfs_value(point, "ORIENT");
        if (orient_str != "") icon_type = RCTLPT_ICON::RCTLPT_RCTLPT52;
        else icon_type = RCTLPT_ICON::RCTLPT_RTLDEF51;
    }
    if (shpName == "SLCONS")
    {
        std::string cat_str = get_attfs_value(point, "CATSLC");
        if (cat_str != "4") cat_str = "1";
        icon_type = atoi(cat_str.c_str());
    }
    if (shpName == "SILTNK")
    {
        icon_type = SILTNK_ICON::SILTNK_TNKCON02;
        std::string catsil_str = get_attfs_value(point, "CATSIL");
        std::string convis_str = get_attfs_value(point, "CONVIS");
        if (catsil_str == "1" && convis_str == "1") icon_type = SILTNK_ICON::SILTNK_SILBUI11;
        else if (catsil_str == "2" && convis_str == "1") icon_type = SILTNK_ICON::SILTNK_TNKCON12;
        else if (catsil_str == "3" && convis_str == "1") icon_type = SILTNK_ICON::SILTNK_TOWERS03;
        else if (catsil_str == "4" && convis_str == "1") icon_type = SILTNK_ICON::SILTNK_TOWERS12;
        else if (catsil_str == "1") icon_type = SILTNK_ICON::SILTNK_SILBUI01;
        else if (catsil_str == "2") icon_type = SILTNK_ICON::SILTNK_TNKCON02;
        else if (catsil_str == "3") icon_type = SILTNK_ICON::SILTNK_TOWERS01;
        else if (catsil_str == "4") icon_type = SILTNK_ICON::SILTNK_TOWERS02;
        else if (convis_str == "1") icon_type = SILTNK_ICON::SILTNK_TNKCON12;
        else icon_type = SILTNK_ICON::SILTNK_TNKCON02;
    }
    if (shpName == "TOPMAR")
    {
        icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;

        std::string colour_str = get_attfs_value(point, "COLOUR");
        std::string colpat_str = get_attfs_value(point, "COLPAT");
        std::string topshp_str = get_attfs_value(point, "TOPSHP");
        std::string inform_str = get_attfs_value(point, "INFORM");

        if (topshp_str == "12" && colpat_str == "6" && colour_str == "1,11") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR98;
        else if (topshp_str == "24" && colpat_str == "3" && colour_str == "1,3") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPU2;
        else if (topshp_str == "3" && colpat_str == "1" && colour_str == "3,4") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "3" && colpat_str == "1" && colour_str == "3,6") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "6" && colpat_str == "2" && colour_str == "3,1") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPT2;
        else if (topshp_str == "7" && colpat_str == "3" && colour_str == "1,3") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPI3;
        else if (topshp_str == "6" && colpat_str == "1" && colour_str == "3,1,3") icon_type = TOPMAR_ICON::TOPMAR_TOPMA116;
        else if (topshp_str == "5" && colpat_str == "1" && colour_str == "3,1,3") icon_type = TOPMAR_ICON::TOPMAR_TOPMA116;
        else if (topshp_str == "26" && colpat_str == "1" && colour_str == "3,4") icon_type = TOPMAR_ICON::TOPMAR_TOPMA117;
        else if (topshp_str == "12" && colour_str == "1,11") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR99;
        else if (topshp_str == "6" && colour_str == "11,2") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPT6;
        else if (topshp_str == "6" && colour_str == "2,11") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPT7;
        else if (topshp_str == "23" && colour_str == "11") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP07;
        else if (topshp_str == "6" && colour_str == "1,2") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPT5;
        else if (topshp_str == "6" && colour_str == "2,1") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPT4;
        else if (topshp_str == "10" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "11" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "12" && colour_str == "1") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP51;
        else if (topshp_str == "12" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP52;
        else if (topshp_str == "12" && colour_str == "3") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP54;
        else if (topshp_str == "12" && colour_str == "6") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP53;
        else if (topshp_str == "13" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "14" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "17" && colour_str == "1") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPJ3;
        else if (topshp_str == "17" && colour_str == "6") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPJ1;
        else if (topshp_str == "19" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP23;
        else if (topshp_str == "22" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPR1;
        else if (topshp_str == "24" && colour_str == "6") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP10;
        else if (topshp_str == "1" && colour_str == "3") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "1" && colour_str == "4") icon_type = TOPMAR_ICON::TOPMAR_TOPMA115;
        else if (topshp_str == "2" && colour_str == "3") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "3" && colour_str == "3") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "3" && colour_str == "6") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "4" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "5" && colour_str == "3") icon_type = TOPMAR_ICON::TOPMAR_TOPMA114;
        else if (topshp_str == "5" && colour_str == "4") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "6" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPT3;
        else if (topshp_str == "6" && colour_str == "3") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPT1;
        else if (topshp_str == "7" && colour_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPI2;
        else if (topshp_str == "7" && colour_str == "6") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPI1;
        else if (topshp_str == "8" && colour_str == "6") icon_type = TOPMAR_ICON::TOPMAR_TOPSHPP2;
        else if (topshp_str == "5" && colour_str == "6") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP53;
        else if (topshp_str == "10") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "11") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "12") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "13") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "14") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "17") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ17;
        else if (topshp_str == "19") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ19;
        else if (topshp_str == "20") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ20;
        else if (topshp_str == "21") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ21;
        else if (topshp_str == "22") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ22;
        else if (topshp_str == "23") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ23;
        else if (topshp_str == "24") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ24;
        else if (topshp_str == "25") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ25;
        else if (topshp_str == "26") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ26;
        else if (topshp_str == "27") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ27;
        else if (topshp_str == "28") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ28;
        else if (topshp_str == "29") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ29;
        else if (topshp_str == "30") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ30;
        else if (topshp_str == "31") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ31;
        else if (topshp_str == "32") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ32;
        else if (topshp_str == "33") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "98") icon_type = TOPMAR_ICON::TOPMAR_ZZZZZZ01;
        else if (topshp_str == "99") icon_type = TOPMAR_ICON::TOPMAR_ZZZZZZ01;
        else if (colour_str == "3") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR91;
        else if (colour_str == "3" && inform_str == "triangle, point up|TR") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP20;
        else if (colour_str == "4") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR92;
        else if (colour_str == "4" && inform_str == "square|SG") icon_type = TOPMAR_ICON::TOPMAR_TOPSHP48;
        else if (topshp_str == "1") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "2") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "3") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "4") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "5") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "6") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ06;
        else if (topshp_str == "7") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ07;
        else if (topshp_str == "8") icon_type = TOPMAR_ICON::TOPMAR_TOPSHQ08;
        else if (topshp_str == "9") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;
        else if (topshp_str == "16") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR90;
        else if (topshp_str == "15") icon_type = TOPMAR_ICON::TOPMAR_TOPMAR93;
        else icon_type = TOPMAR_ICON::TOPMAR_TOPMAR01;

        if (icon_type == TOPMAR_ICON::TOPMAR_TOPMAR01)
        {
            bool floating = judge_floating(point);
            int topshp = atoi(topshp_str.c_str());
            if (floating)
            {
                switch (topshp) {
                case 1:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR02;
                    break;
                case 2:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR04;
                    break;
                case 3:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR10;
                    break;
                case 4:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR12;
                    break;

                case 5:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR13;
                    break;
                case 6:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR14;
                    break;
                case 7:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR65;
                    break;
                case 8:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR17;
                    break;

                case 9:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR16;
                    break;
                case 10:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR08;
                    break;
                case 11:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR07;
                    break;
                case 12:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR14;
                    break;

                case 13:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR05;
                    break;
                case 14:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR06;
                    break;
                case 17:
                    icon_type = TOPMAR_ICON::TOPMAR_TMARDEF2;
                    break;
                case 18:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR10;
                    break;

                case 19:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR13;
                    break;
                case 20:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR14;
                    break;
                case 21:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR13;
                    break;
                case 22:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR14;
                    break;

                case 23:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR14;
                    break;
                case 24:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR02;
                    break;
                case 25:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR04;
                    break;
                case 26:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR10;
                    break;

                case 27:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR17;
                    break;
                case 28:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR18;
                    break;
                case 29:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR02;
                    break;
                case 30:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR17;
                    break;

                case 31:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR14;
                    break;
                case 32:
                    icon_type = TOPMAR_ICON::TOPMAR_TOPMAR10;
                    break;
                case 33:
                    icon_type = TOPMAR_ICON::TOPMAR_TMARDEF2;
                    break;
                default:
                    icon_type = TOPMAR_ICON::TOPMAR_TMARDEF2;
                    break;
                }
            }
            else
            {
				switch (topshp) {
				case 1:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR22;
					break;
				case 2:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR24;
					break;
				case 3:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR30;
					break;
				case 4:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR32;
					break;

				case 5:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR33;
					break;
				case 6:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR34;
					break;
				case 7:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR85;
					break;
				case 8:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR86;
					break;

				case 9:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR36;
					break;
				case 10:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR28;
					break;
				case 11:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR27;
					break;
				case 12:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR14;
					break;

				case 13:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR25;
					break;
				case 14:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR26;
					break;
				case 15:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR88;
					break;
				case 16:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR87;
					break;

				case 17:
					icon_type = TOPMAR_ICON::TOPMAR_TMARDEF1;
					break;
				case 18:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR30;
					break;
				case 19:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR33;
					break;
				case 20:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR34;
					break;

				case 21:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR33;
					break;
				case 22:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR34;
					break;
				case 23:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR34;
					break;
				case 24:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR22;
					break;

				case 25:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR24;
					break;
				case 26:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR30;
					break;
				case 27:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR86;
					break;
				case 28:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR89;
					break;

				case 29:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR22;
					break;
				case 30:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR86;
					break;
				case 31:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR14;
					break;
				case 32:
					icon_type = TOPMAR_ICON::TOPMAR_TOPMAR30;
					break;
				case 33:
					icon_type = TOPMAR_ICON::TOPMAR_TMARDEF1;
					break;
				default:
					icon_type = TOPMAR_ICON::TOPMAR_TMARDEF1;
					break;
				}
            }
        }
    }
    if (shpName == "VEGATN")
    {
        icon_type = 0;
        std::string catveg_str = get_attfs_value(point, "CATVEG");

        if (catveg_str == "13") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "14") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "15") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "16") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "17") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "18") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "19") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "20") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "21") icon_type = VEGATN_ICON::VEGATN_TREPNT05;
        else if (catveg_str == "22") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "3") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "4") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "5") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "6") icon_type = VEGATN_ICON::VEGATN_TREPNT04;
        else if (catveg_str == "7") icon_type = VEGATN_ICON::VEGATN_TREPNT05;
    }
    if (shpName == "UWTROC")
    {
        icon_type = 0;
        std::string catobs_str = get_attfs_value(point, "CATOBS");
        std::string watlev_str = get_attfs_value(point, "WATLEV");
        std::string expsou_str = get_attfs_value(point, "EXPSOU");
        std::string valsou_str = get_attfs_value(point, "VALSOU");

        int catobs = 0;
        if (catobs_str != "") catobs = atoi(catobs_str.c_str());
        int watlev = 0;
        if (watlev_str != "") watlev = atoi(watlev_str.c_str());
        int expsou = 0;
        if (expsou_str != "") expsou = atoi(expsou_str.c_str());

        double valsou = 1e6;
        double depth_value = 1e6;
        double least_depth = 1e6;

        if (valsou_str != "")
        {
            valsou = atof(valsou_str.c_str());
            depth_value = valsou;
        }
        else
        {
            if (expsou != 1) {
                if (6 == catobs)
                    depth_value = 0.01;
                else if (0 == watlev)  // default
                    depth_value = -15.0;
                else {
                    switch (watlev) {
                    case 5:
                        depth_value = 0.0;
                        break;
                    case 3:
                        depth_value = 0.01;
                        break;
                    case 4:
                    case 1:
                    case 2:
                    default:
                        depth_value = -15.0;
                        break;
                    }
                }
            }
        }

        std::string udwhaz03str = get_UDWHAZ03(point, depth_value);
        int sounding = false;
        std::string quapnt01str = get_CSQUAPNT01(point);

        if (0 != udwhaz03str.size()) {
            icon_type = UWTROC_ICON::UWTROC_ISODGR51;
        }
        else
        {
            if (valsou_str != "")
            {
                if (valsou <= 20.0) {
                    {
                        if (-9 == watlev) {  // default
                            icon_type = UWTROC_ICON::UWTROC_DANGER51;
                            sounding = true;
                        }
                        else {
                            switch (watlev) {
                            case 3:
                                icon_type = UWTROC_ICON::UWTROC_DANGER51;
                                sounding = true;
                                break;
                            case 4:
                            case 5:
                                icon_type = UWTROC_ICON::UWTROC_UWTROC04;
                                sounding = false;
                                break;
                            default:
                                icon_type = UWTROC_ICON::UWTROC_DANGER51;
                                sounding = false;
                                break;
                            }
                        }
                    }
                }
                else {  // valsou > 20.0
                    icon_type = UWTROC_ICON::UWTROC_DANGER52;
                    sounding = TRUE;
                }
            }
            else
            {
                if (watlev == -9)  // default
                    icon_type = UWTROC_ICON::UWTROC_UWTROC04;
                else {
                    switch (watlev) {
                    case 2:
                        icon_type = UWTROC_ICON::UWTROC_LNDARE01;
                        break;
                    case 3:
                        icon_type = UWTROC_ICON::UWTROC_UWTROC03;
                        break;
                    default:
                        icon_type = UWTROC_ICON::UWTROC_UWTROC04;
                        break;
                    }
                }
            }
        }
    }
    if (shpName == "WRECKS")
    {
        icon_type = 0;
        std::string valsou_str = get_attfs_value(point, "VALSOU");
        std::string catwrk_str = get_attfs_value(point, "CATWRK");
        std::string watlev_str = get_attfs_value(point, "WATLEV");
        std::string quasou_str = get_attfs_value(point, "QUASOU");

        if (catwrk_str == "3") icon_type = WRECKS_ICON::WRECKS_FOULGND1;
        else
        {
            double depth_value = 1e6;
            double valsou = 1e6;
            int watlev = -9;
            if (watlev_str != "") watlev = atoi(watlev_str.c_str());
            int catwrk = -9;
            if (catwrk_str != "") catwrk = atoi(catwrk_str.c_str());
            int quasou = -9;
            if (quasou_str != "") quasou = atoi(quasou_str.c_str());

            if (valsou_str != "")
            {
                valsou = atof(valsou_str.c_str());
                depth_value = valsou;
            }
            else
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

            char quasouchar[1024] = { '\0' };
            if (quasou_str != "") _parseList(quasou_str.c_str(), quasouchar, sizeof(quasouchar));
                        
            std::string udwhaz03str = "";
            if (quasouchar[0] == 0 || NULL == STRPBRK(quasouchar, "\07")) {
                udwhaz03str = get_UDWHAZ03(point, depth_value);
            }
            else {
                quasou = 7;
                udwhaz03str = "";
            }
            std::string quapnt01str = get_CSQUAPNT01(point);

            if (0 != udwhaz03str.size()) {
                icon_type = WRECKS_ICON::WRECKS_ISODGR51;
            }
            else
            {
                if (valsou_str != "") {
                    if (valsou < 30) icon_type = WRECKS_ICON::WRECKS_DANGER51;
                    else icon_type = WRECKS_ICON::WRECKS_DANGER52;

                    if (7 == quasou) icon_type = WRECKS_ICON::WRECKS_WRECKS07;
                }
                else {
                    if (-9 != catwrk && -9 != watlev) {
                        if (1 == catwrk && 3 == watlev)
                            icon_type = WRECKS_ICON::WRECKS_WRECKS04;
                        else {
                            if (2 == catwrk && 3 == watlev)
                                icon_type = WRECKS_ICON::WRECKS_WRECKS05;
                            else {
                                if (4 == catwrk || 5 == catwrk)
                                    icon_type = WRECKS_ICON::WRECKS_WRECKS01;
                                else {
                                    if (1 == watlev || 2 == watlev || 5 == watlev || 4 == watlev) {
                                        icon_type = WRECKS_ICON::WRECKS_WRECKS01;
                                    }
                                    else
                                        icon_type = WRECKS_ICON::WRECKS_WRECKS05;  // default
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (shpName == "TS_FEB")
    {
        icon_type = 0;
        std::string cat_ts_str = get_attfs_value(point, "CAT_TS");
        std::string orient_str = get_attfs_value(point, "ORIENT");

        if (cat_ts_str == "1" && orient_str != "") icon_type = TS_FEB_ICON::TS_FEB_FLDSTR01;
        else if (cat_ts_str == "2" && orient_str != "") icon_type = TS_FEB_ICON::TS_FEB_EBBSTR01;
        else if (cat_ts_str == "3" && orient_str != "") icon_type = TS_FEB_ICON::TS_FEB_CURENT01;
        else icon_type = TS_FEB_ICON::TS_FEB_CURDEF01;
    }
	
	return icon_type;
}
