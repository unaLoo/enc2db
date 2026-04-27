#include "../compat.h"
#include "MyChartLoader2.h"
#include "ogr_api.h"
#include "util.h"
#include "psimpl.h"
#include <iterator>

#ifdef _WIN32
	#include <io.h>        // _findfirst, _findnext, _findclose
#else
	#include <dirent.h>
	#include <sys/types.h>
#endif
#include <stdio.h>
#include <string>


#define ATTV_FIELD_STRING_LENGHT 255

MyChartLoader::MyChartLoader(void): dsid_loaded(false), dspm_loaded(false), m_cscl_level(10)
{
}

MyChartLoader::~MyChartLoader(void)
{
}

int MyChartLoader::S57Load(const char* fpath)
{   
    // current_module 是 DDFModule，用于读取 ISO 8211 文件中的记录。
    // S-57 正是基于 ISO 8211 格式来封装的
    if (current_module.Open(fpath) == 0) {
        std::cerr << "Chart::S57Load(): Failed loading file "<< fpath << std::endl;
        current_module.Close();
        return -1;
    }

	std::string path_str = fpath;
	int idx = path_str.find_last_of('\\');
	chart_file_name = path_str.substr(idx+1);

    DDFRecord *record;
    
    while ((record = current_module.ReadRecord()) != NULL) {
        // 处理一条 record

        // 读取字段
        DDFField *field = record->GetField(1);
        if (!field) {
            std::cerr << "Chart::S57Load(): Error getting field"<< std::endl;
            current_module.Close();
            return -1;
        }
        DDFFieldDefn *field_defn = field->GetFieldDefn();
        const char *field_name = field_defn->GetName();

        // DSID --> Data Set Identification（数据集标识）包含版本号、发布日期等元数据
        if (strncmp(field_name, "DSID", 4) == 0) {
            if (S57ReadDSIDRecord(record) < 0) {
                std::cerr << "Chart::S57Load():"<< " error loading dsid record"<< std::endl;
                current_module.Close();
                return -1;
            }
        // DSPM --> Data Set Parameter Map（数据集参数映射）包含DUNI数据单位、CSCL编图比例尺等信息
        } else if (strncmp(field_name, "DSPM", 4) == 0) {
            if (S57ReadDSPMRecord(record) < 0) {
                std::cerr << "Chart::S57Load():"<< " error loading dspm record"<< std::endl;
                current_module.Close();
                return -1;
            }
        } else if (strncmp(field_name, "CATD", 4) == 0) { // Catalogue Directory
        } else if (strncmp(field_name, "DDDF", 4) == 0) { // Data Directory Definition Field
        } else if (strncmp(field_name, "DDSI", 4) == 0) { // Data Set Structure Information
        } else if (strncmp(field_name, "FRID", 4) == 0) { // Feature Record Identification 要素记录!! 描述语义属性
            if (S57ReadFeatureRecord(record) < 0) { 
                // 这里会把所有的要素都存在一个 feature_vec 里, 注意：这里指代的要素是只包含语义属性，不包含空间信息
                std::cerr << "Chart::S57Load():"<< " error loading feature record"<< std::endl;
                current_module.Close();
                return -1;
            }
        } else if (strncmp(field_name, "VRID", 4) == 0) { // Vector Record Identifier 向量记录！！ 描述空间信息
            if (S57ReadVectorRecord(record) < 0) {
                // isolated node 的信息存在 isolated_node_vec 和 isolated_node_vector_map
                // connected node 的信息存在 connected_node_vec 和 connected_node_vector_map
                // edge 的信息存在 edge_node_vec 和 edge_vector_map
                std::cerr << "Chart::S57Load():"<< " error loading vector record"<< std::endl;
                current_module.Close();
                return -1;
            }
        } else {
            std::cerr << "Chart::S57Load(): Unknown leading record name "<< field_name << std::endl;
            current_module.Close();
            return -1;
        }
    }

    // we've read the file.
    if (!dsid_loaded) {
        std::cerr << "Chart::S57Load(): DSID not loaded" << std::endl;
        return -1;
    } 

    if (!dspm_loaded) {
        std::cerr << "Chart::S57Load(): DSPM not loaded" << std::endl;
        return -1;
    }

	//ReBuildIdReference();

    return 0;
}

int MyChartLoader::S57LoadUpdate(const char* fpath)
{
	DDFModule update_module;
	if (update_module.Open(fpath) == 0) {
		std::cerr << "Chart::S57Load(): Failed loading file " << fpath << std::endl;
		update_module.Close();
		return -1;
	}

	DDFRecord* record;
	while ((record = update_module.ReadRecord()) != NULL) {
		DDFField* field = record->GetField(1);
		if (!field) {
			std::cerr << "Chart::S57Load(): Error getting field" << std::endl;
			update_module.Close();
			return -1;
		}
		DDFFieldDefn* field_defn = field->GetFieldDefn();
		const char* field_name = field_defn->GetName();

		if (strncmp(field_name, "DSID", 4) == 0) {
		}
		else if (strncmp(field_name, "DSPM", 4) == 0) {
		}
		else if (strncmp(field_name, "CATD", 4) == 0) {
		}
		else if (strncmp(field_name, "DDDF", 4) == 0) {
		}
		else if (strncmp(field_name, "DDSI", 4) == 0) {
		}
		else if (strncmp(field_name, "FRID", 4) == 0) {
			if (S57ReadFeatureRecord(record) < 0) {
				std::cerr << "Chart::S57Load():" << " error loading feature record" << std::endl;
				update_module.Close();
				return -1;
			}
		}
		else if (strncmp(field_name, "VRID", 4) == 0) {
			if (S57ReadVectorRecord(record) < 0) {
				std::cerr << "Chart::S57Load():" << " error loading vector record" << std::endl;
				update_module.Close();
				return -1;
			}
		}
		else {
			std::cerr << "Chart::S57Load(): Unknown leading record name " << field_name << std::endl;
			update_module.Close();
			return -1;
		}
	}

	return 0;
}

int MyChartLoader::SetupFeatures()
{
	if (S57SetupEdgeVectors() < 0) {
		std::cerr << "Chart::S57Load(): Error setting up edge records" << std::endl;
		return -1;
	}

	if (S57SetupAreaFeatures() < 0) {
		std::cerr << "Chart::S57Load(): Error setting up area features" << std::endl;
		return -1;
	}

	// This MUST be after SetupEdgeVectors and SetupAreaFeatures otherwise
	// our connected nodes won't be multiplied.
	S57NormalizeCoordinates();
}

/**
 * Read dsid data from a record
 * @param record The record to read
 * @return 0 on success, -1 on failure.
 */
int MyChartLoader::S57ReadDSIDRecord(DDFRecord *record)
{
    if (dsid_loaded) {
        std::cerr << "Chart::S57ReadDSIDRecord(): DSID already loaded"<< std::endl;
        return -1;
    }
    if (dsid.Load(record) < 0) {
        std::cerr << "Chart::S57ReadDSIDRecord(): Error loading dsid record"<< std::endl;
        return -1;
    }
    dsid_loaded = true;
    return 0;
}

/**
 * Read dspm data from a record
 * @param record The record to read
 * @return 0 on success, -1 on failure.
 */
int MyChartLoader::S57ReadDSPMRecord(DDFRecord *record)
{
    if (dspm_loaded) {
        std::cerr << "Chart::S57ReadDSPMRecord(): DSPM already loaded"<< std::endl;
        return -1;
    }
    if (dspm.Load(record) < 0) {
        std::cerr << "Chart::S57ReadDSPMRecord(): Error loading dspm record"<< std::endl;
        return -1;
    }
    dspm_loaded = true;
    return 0;
}

//*****************************************************************************
//
// Feature record loading
//
//*****************************************************************************

/**
 * Read feature data from a record.
 * @param record The record to read
 * @return 0 on success, -1 on failure.
 */
int MyChartLoader::S57ReadFeatureRecord(DDFRecord *record)
{
    DDFField *field = record->GetField(1);
    if (!field)  return -1;
         
    DDFFieldDefn *field_defn = field->GetFieldDefn();
    if (!field_defn) return -1;

    const char *field_data = field->GetData();
    if (!field_data) return -1;

    int bytes_remaining = field->GetDataSize();
    int bytes_consumed = 0;

    DDFSubfieldDefn *subfield_defn;
    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn) return -1;

    // skip rcnm, rcid
    for (int i = 0; i < 2; i++) {
        subfield_defn = field_defn->GetSubfield(i);
        if (!subfield_defn) return -1;
        subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
        bytes_remaining -= bytes_consumed;
        field_data += bytes_consumed;
    }

    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn) return -1;
    int prim = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);

    switch (prim)
    {
        case 1:
            if (S57LoadPointFeature(record) < 0) {
                std::cerr << "Chart::S57ReadFeatureRecord(): "<< "Error loading point feature" << std::endl;
                return -1;
            }
            break;
        case 2:
            if (S57LoadLineFeature(record) < 0) {
                std::cerr << "Chart::S57ReadFeatureRecord(): "<< "Error loading line feature" << std::endl;
                return -1;
            }
            break;
        case 3:
            if (S57LoadAreaFeature(record) < 0) {
                std::cerr << "Chart::S57ReadFeatureRecord(): "<< "Error loading area feature" << std::endl;
                return -1;
            }
            break;
        case 255:
            break;
        default:
            //std::cerr << "Chart::S57ReadFeatureRecord(): Unknown prim"<< prim << std::endl;
			//return -1;
			break;
    }

    return 0;
}

/**
 * Read point feature data from a record
 * @param record The record to read
 * @return 0 on success, -1 on failure.
 */
int MyChartLoader::S57LoadPointFeature(DDFRecord *record)
{   
    // 语义属性上的要素
    PointFeature* point_feature = new PointFeature();
    int result;
    // Load 方法里封装了  Read the FRID field and associated subfields 的逻辑
    if ((result = point_feature->Load(record)) == -1) {
        std::cerr << "Chart::S57LoadPointFeature(): Error loading" << std::endl;
        return -1;
    }

	if (point_feature->frid.ruin == 1) // ruin 1 --> new record
	{
		point_features_map[result].push_back(*point_feature); // point_features_map<OBJL, Array of Feature>
		feature_vec.push_back(point_feature);  // feature_vec 存放了所有的要素
	}
	else if (point_feature->frid.ruin == 2 || point_feature->frid.ruin == 3) // ruin 2,3 --> update or change
	{
		for (int iP = 0; iP < feature_vec.size(); iP++)
		{
			if (feature_vec[iP]->frid.rcid == point_feature->frid.rcid &&
				feature_vec[iP]->frid.objl == point_feature->frid.objl)
			{
				feature_vec[iP]->frid.rver = point_feature->frid.rver;
				feature_vec[iP]->frid.ruin = point_feature->frid.ruin;

				int k = feature_vec[iP]->frid.objl;
                // 遍历 point_features_map[objl] 里的所有要素，找到 rcid 相同的进行更新
				std::vector<PointFeature>& temp_point_vec = point_features_map[k];
				for (int iK = 0; iK < temp_point_vec.size(); iK++)
				{
					if (temp_point_vec[iK].frid.rcid == point_feature->frid.rcid)
					{
						temp_point_vec[iK].frid.rver = point_feature->frid.rver;
						temp_point_vec[iK].frid.ruin = point_feature->frid.ruin;
						break;
					}
				}
				break;
			}
		}
	}
    return 0;
}

/**
 * Read line feature data from a record
 * @param record The record to read
 * @return 0 on success, -1 on failure.
 */
int MyChartLoader::S57LoadLineFeature(DDFRecord *record)
{
    LineFeature* line_feature = new LineFeature();
    int result;
    if ((result = line_feature->Load(record)) == -1) {
        std::cerr << "Chart::S57LoadLineFeature(): Error loading" << std::endl;
        return -1;
    }

	if (line_feature->frid.ruin == 1)
	{
		line_features_map[result].push_back(*line_feature); // line_features_map<OBJL, Array of Feature>
		feature_vec.push_back(line_feature);  // 存储了所有的要素
	}
	else if (line_feature->frid.ruin == 2 || line_feature->frid.ruin == 3)
	{
		std::vector<LineFeature>& temp_line_vec = line_features_map[result];
		for (int iP = 0; iP < temp_line_vec.size(); iP++)
		{
			if (temp_line_vec[iP].frid.rcid == line_feature->frid.rcid)
			{
				line_features_map[result][iP].frid.rver = line_feature->frid.rver;
				line_features_map[result][iP].frid.ruin = line_feature->frid.ruin;
				break;
			}
		}
	}
    return 0;
}

/**
 * Read area feature data from a record
 * @param record The record to read
 * @return 0 on success, -1 on failure.
 */
int MyChartLoader::S57LoadAreaFeature(DDFRecord *record)
{
    AreaFeature* area_feature = new AreaFeature();
    int result;
    if ((result = area_feature->Load(record)) == -1) {
        std::cerr << "Chart::S57LoadAreaFeature(): Error loading" << std::endl;
        return -1;
    }

	if (area_feature->frid.ruin == 1)
	{
		area_features_map[result].push_back(*area_feature); // area_features_map<OBJL, Array of Feature>
		feature_vec.push_back(area_feature); // 存储了所有的要素
	}
	else if (area_feature->frid.ruin == 2 || area_feature->frid.ruin == 3)
	{
		std::vector<AreaFeature>& temp_area_vec = area_features_map[result];
		for (int iP = 0; iP < temp_area_vec.size(); iP++)
		{ 
			if (temp_area_vec[iP].frid.rcid == area_feature->frid.rcid)
			{
				area_features_map[result][iP].frid.rver = area_feature->frid.rver;
				area_features_map[result][iP].frid.ruin = area_feature->frid.ruin;
				break;
			}
		}
	}
    return 0;
}

//*****************************************************************************
//
// Vector record loading
//
//*****************************************************************************

/**
 * Read vector data from  a record
 * @param record The record to read
 * @return -1 on failure, 0 on success.
 */
int MyChartLoader::S57ReadVectorRecord(DDFRecord *record)
{
    DDFField *field = record->GetField(1);
    if (!field) return -1;
    DDFFieldDefn *field_defn = field->GetFieldDefn();
    if (!field_defn) return -1;
    const char *field_data = field->GetData();
    if (!field_data) return -1;

    int bytes_remaining = field->GetDataSize();
    int bytes_consumed = 0;

    DDFSubfieldDefn *subfield_defn;

    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn) return -1;
    int rcnm = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);

    switch (rcnm) // record name
    {
        case 110:
            // 处理 Isolated Node
            if (S57LoadIsolatedNodeVectorRecord(record) < 0) {  
                std::cerr << "Chart::S57ReadVectorRecord(): "<< "Error loading isolated node vector" << std::endl;
                return -1;
            }
            break;
        case 120:
            // 处理 Connected Node
            if (S57LoadConnectedNodeVectorRecord(record) < 0) {
                std::cerr << "Chart::S57ReadVectorRecord(): "<< "Error connected node vector" << std::endl;
                return -1;
            }
            break;
        case 130:
            // 处理 Edge
            if (S57LoadEdgeVectorRecord(record) < 0) {
                std::cerr << "Chart::S57ReadVectorRecord(): "<< "Error loading edge vector" << std::endl;
                return -1;
            }
            break;
        case 140:
            // 处理 Face
            break;
        default:
            std::cerr << "Chart::S57ReadVectorRecord(): Unknown rcnm"<< rcnm << std::endl;
            return -1;
    }

    return 0;
}

/**
 * Read isolated node vector data from a record
 * @param record The record to read
 * @return -1 on failure, 0 on success.
 */
int MyChartLoader::S57LoadIsolatedNodeVectorRecord(DDFRecord *record)
{
    IsolatedNodeVector isolated_node_vector;
    int record_id;
    if ((record_id = isolated_node_vector.Load(record, extent_xmax, extent_xmin, extent_ymax, extent_ymin)) == -1) {
        std::cerr << "Chart::S57LoadIsolatedNodeVectorRecord(): Error loading"<< std::endl;
        return -1;
    }

	if (isolated_node_vector.vrid.ruin != 2)
	{
		isolated_node_vectors_map[record_id] = isolated_node_vector;

		isolated_node_vec.push_back(isolated_node_vector);
	}
    return 0;
}

/**
 * Read connected node vector data from a record
 * @param record The record to read
 * @return -1 on failure, 0 on success.
 */
int MyChartLoader::S57LoadConnectedNodeVectorRecord(DDFRecord *record)
{
    ConnectedNodeVector connected_node_vector;
    int record_id;
    if ((record_id = connected_node_vector.Load(record, extent_xmax, extent_xmin, extent_ymax, extent_ymin)) == -1) {
        std::cerr << "Chart::S57LoadConnectedNodeVectorRecord(): Error loading"<< std::endl;
        return -1;
    }

	if (connected_node_vector.vrid.ruin != 2)
	{
		connected_node_vectors_map[record_id] = connected_node_vector;

		connected_node_vec.push_back(connected_node_vector);
	}
    return 0;
}

/**
 * Read edge node vector data from a record
 * @param record The record to read
 * @return -1 on failure, 0 on success.
 */
int MyChartLoader::S57LoadEdgeVectorRecord(DDFRecord *record)
{
    EdgeVector edge_vector;
    int record_id;
    if ((record_id = edge_vector.Load(record, extent_xmax, extent_xmin, extent_ymax, extent_ymin)) == -1) {
        std::cerr << "Chart::S57LoadEdgeVectorRecord(): Error loading"<< std::endl;
        return -1;
    }

	if (edge_vector.vrid.ruin != 2)
	{
		edge_vectors_map[record_id] = edge_vector;

		edge_node_vec.push_back(edge_vector);
	}
    return 0;
}


//*****************************************************************************
//
// Setup of edges, areas, display lists
//
//*****************************************************************************

/**
 * Multiply all coordinates by the normalization factor. 
 */
void MyChartLoader::S57NormalizeCoordinates()
{
    int coord_mult_factor = dspm.GetCoordMultFactor();
	int sounding_mult_factor = dspm.GetSoundingMultFactor();

    std::map<int, EdgeVector>::iterator ev;
    std::map<int, EdgeVector>::iterator edge_end(edge_vectors_map.end());
    for (ev = edge_vectors_map.begin(); ev != edge_end; ++ev)
	{
        ev->second.Normalize(coord_mult_factor);
	}

    std::map<int, IsolatedNodeVector>::iterator inv;
    std::map<int, IsolatedNodeVector>::iterator iso_end(isolated_node_vectors_map.end());
    for (inv = isolated_node_vectors_map.begin(); inv != iso_end; ++inv) 
	{
        inv->second.Normalize(coord_mult_factor, sounding_mult_factor);
	}

	std::map<int, ConnectedNodeVector>::iterator cnv;
	std::map<int, ConnectedNodeVector>::iterator con_end(connected_node_vectors_map.end());
	for (cnv = connected_node_vectors_map.begin(); cnv != con_end; ++cnv)
	{
		cnv->second.Normalize(coord_mult_factor, sounding_mult_factor);
	}
}

/**
 * Complete edge vectors
 * @return -1 on failure, 0 on success
 */
int MyChartLoader::S57SetupEdgeVectors()
{
    std::map<int, EdgeVector>::iterator pos;
    std::map<int, EdgeVector>::iterator end(edge_vectors_map.end());
    for (pos = edge_vectors_map.begin(); pos != end; ++pos)
	{
		pos->second.hasSplited = false;
        pos->second.CompleteEdge(connected_node_vectors_map);
	}
	for (int i=0; i<edge_node_vec.size(); i++)
	{
		edge_node_vec[i].hasSplited = false;
		edge_node_vec[i].CompleteEdge(connected_node_vectors_map);
	}
    return 0;
}

/**
 * Setup area features
 * @return 0 on success, -1 on failure
 */
int MyChartLoader::S57SetupAreaFeatures()
{
	/*std::map<int, std::vector<AreaFeature> >::iterator af;
	std::map<int, std::vector<AreaFeature> >::iterator end = area_features_map.end();

	for (af = area_features_map.begin(); af != end; ++af) 
	{
		int area_count = af->second.size();
		for (int i = 0; i < area_count; i++)
		{
			af->second[i].CompleteContours(edge_vectors_map);
		}
	}*/

    return 0;
}

void MyChartLoader::GetChartExtent()
{
	extent_xmax = 0.0;
	extent_xmin = 0.0;
	extent_ymax = 0.0;
	extent_ymin = 0.0;

	std::vector<AreaFeature>& temp_area_vec = area_features_map[302];

	//////////////////////////////////////////////////////////////////////////
	for (int iA = 0; iA < temp_area_vec.size(); iA++)
	{
		int edgeCount = temp_area_vec[iA].fspts.size();
		int feature_rcid = temp_area_vec[iA].frid.rcid;

		for (int iE = 0; iE < edgeCount; iE++)
		{
			fspt_t temp_fspt = temp_area_vec[iA].fspts[iE];
			int edgeId = temp_fspt.rcid;
			int orient = temp_fspt.ornt;
			int usage = temp_fspt.usag;
			int mask = temp_fspt.mask;

			std::vector<sg2d_t> sg2ds = edge_vectors_map[edgeId].GetSG2Ds();
			for (int iP = sg2ds.size() - 1; iP >= 0; iP--)
			{
				double x = sg2ds[iP].long_lat[0];
				double y = sg2ds[iP].long_lat[1];

				if (extent_xmax == 0.0 && extent_xmin == 0.0 &&
					extent_ymax == 0.0 && extent_ymin == 0.0)
				{
					extent_xmin = x;
					extent_xmax = x;
					extent_ymin = y;
					extent_ymax = y;
				}
				else
				{
					if (extent_xmax < x) extent_xmax = x;
					if (extent_xmin > x) extent_xmin = x;
					if (extent_ymax < y) extent_ymax = y;
					if (extent_ymin > y) extent_ymin = y;
				}
			}
		}
	}
	/*extent_xmax = extent_xmax / dspm.coord_mult_factor;
	extent_xmin = extent_xmin / dspm.coord_mult_factor;
	extent_ymax = extent_ymax / dspm.coord_mult_factor;
	extent_ymin = extent_ymin / dspm.coord_mult_factor;*/
}

int MyChartLoader::GetSuitableLevel()
{
	int level = 10;
	if (this->dspm.comp_sod >= 10000000) level = 4;
	else if (this->dspm.comp_sod >= 5000000) level = 5;
	else if (this->dspm.comp_sod >= 2000000) level = 6;
	else if (this->dspm.comp_sod >= 1000000) level = 7;
	else if (this->dspm.comp_sod >= 500000) level = 8;
	else if (this->dspm.comp_sod >= 200000) level = 9;
	else if (this->dspm.comp_sod >= 100000) level = 10;
	else if (this->dspm.comp_sod >= 50000) level = 11;
	else level = 12;

	m_cscl_level = level;

	return level;
}

int MyChartLoader::ReBuildIdReference()
{
	std::map<int, int> id_map;
	for (int p1=0; p1<edge_node_vec.size()-1; p1++)
	{
		if (edge_node_vec[p1].hasCheckedSame)
			continue;
		for (int p2=p1+1; p2<edge_node_vec.size(); p2++)
		{
			if (p1==p2) continue;
			if (edge_node_vec[p1].getIsSame(edge_node_vec[p2]) )
			{
				edge_node_vec[p2].hasCheckedSame = true;
				int same_id = edge_node_vec[p2].vrid.rcid;
				int ok_id = edge_node_vec[p1].vrid.rcid;
				id_map[same_id] = ok_id;

				edge_vectors_map.erase(same_id);
			}
		}
	}
	std::vector<EdgeVector> new_edge_node_vec;
	for (int iEdge=0; iEdge<edge_node_vec.size(); iEdge++)
	{
		if (edge_node_vec[iEdge].hasCheckedSame==false)
		{
			new_edge_node_vec.push_back(edge_node_vec[iEdge]);
		}
	}
	edge_node_vec.clear();
	for (int iEdge=0; iEdge<new_edge_node_vec.size(); iEdge++)
	{
		edge_node_vec.push_back(new_edge_node_vec[iEdge]);
	}

	for (int iFeature=0; iFeature<feature_vec.size(); iFeature++)
	{
		Feature* temp_feature = feature_vec[iFeature];
		int ref_count = temp_feature->fspts.size();
		for (int iRef=0; iRef<ref_count; iRef++)
		{
			int temp_id = temp_feature->fspts[iRef].rcid;
			std::map<int, int>::iterator temp_it = id_map.find(temp_id);
			if (temp_it != id_map.end())
			{
				temp_feature->fspts[iRef].rcid = id_map[temp_id];
			}
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	std::map<int, std::vector<LineFeature> >::iterator lineIter;
	std::map<int, std::vector<LineFeature> >::iterator lineEnd = line_features_map.end();
	for (lineIter = line_features_map.begin(); lineIter != lineEnd; ++lineIter) 
	{
		std::vector<LineFeature>& temp_line_vec = lineIter->second;
		for (int iP=0; iP<temp_line_vec.size(); iP++)
		{
			Feature& temp_feature = temp_line_vec[iP];
			int ref_count = temp_feature.fspts.size();
			for (int iRef=0; iRef<ref_count; iRef++)
			{
				int temp_id = temp_feature.fspts[iRef].rcid;
				std::map<int, int>::iterator temp_it = id_map.find(temp_id);
				if (temp_it != id_map.end())
				{
					temp_feature.fspts[iRef].rcid = id_map[temp_id];
					for (int m=0; m<temp_feature.ref_record_ids.size(); m++)
					{
						if (temp_feature.ref_record_ids[m] == temp_id)
						{
							temp_feature.ref_record_ids[m] = id_map[temp_id];
							break;
						}
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	std::map<int, std::vector<AreaFeature> >::iterator areaIter;
	std::map<int, std::vector<AreaFeature> >::iterator areaEnd = area_features_map.end();
	for (areaIter = area_features_map.begin(); areaIter != areaEnd; ++areaIter) 
	{
		std::vector<AreaFeature>& temp_area_vec = areaIter->second;
		for (int iP=0; iP<temp_area_vec.size(); iP++)
		{
			Feature& temp_feature = temp_area_vec[iP];
			int ref_count = temp_feature.fspts.size();
			for (int iRef=0; iRef<ref_count; iRef++)
			{
				int temp_id = temp_feature.fspts[iRef].rcid;
				std::map<int, int>::iterator temp_it = id_map.find(temp_id);
				if (temp_it != id_map.end())
				{
					temp_feature.fspts[iRef].rcid = id_map[temp_id];
				}
			}
		}
	}
	
	return 0;
}

void MyChartLoader::LoadCodeNameSystem(std::string resourceFolder)
{
	FILE* fp;
	std::string filePath = resourceFolder + "s57objectclasses.csv";
	fopen_s(&fp, filePath.c_str() , "r");
	char line[1024];
	fgets(line, 1024, fp);
	
	while (!feof(fp)) 
	{ 
		int code;
		std::string ObjectClass;
		std::string Acronym;
		std::string Attribute_A;
		std::string Attribute_B;
		std::string Attribute_C;
		std::string Class;
		std::string Primitives;
		fgets(line, 1024, fp);
		std::string lineStr = line;

		int place0 = lineStr.find_first_of(',');
		std::string codeStr = lineStr.substr(0, place0);
		code=atoi(codeStr.c_str());

		int placeS = lineStr.find_first_of('"');
		int place1 = lineStr.find_first_of(',', place0+1);
		if (placeS<0)
		{
			int place2 = lineStr.find_first_of(',', place1+1);
			Acronym = lineStr.substr(place1+1, place2-place1-1);
		}
		else
		{
			int placeS1 = lineStr.find_first_of('"', placeS+1);
			int place2 = lineStr.find_first_of(',', placeS1+2);
			Acronym = lineStr.substr(placeS1+2, place2-placeS1-2);
		}
		//fscanf(fp, "%d,%s,%s,%s,%s,%s,%s,%s", &code, ObjectClass, Acronym, Attribute_A, Attribute_B, Attribute_C, Class, Primitives);
		code_name_system[code] = Acronym;
	}
	fclose(fp);

	//////////////////////////////////////////////////////////////////////////
	filePath = resourceFolder + "s57attributes.csv";
	fopen_s(&fp, filePath.c_str() , "r");
	//fp	= fopen("..\\data\\s57attributes.csv", "r");
	fgets(line, 1024, fp);

	while (!feof(fp)) 
	{ 
		int code;
		std::string Acronym;
		fgets(line, 1024, fp);
		std::string lineStr = line;

		int place0 = lineStr.find_first_of(',');
		std::string codeStr = lineStr.substr(0, place0);
		code=atoi(codeStr.c_str());

		int placeS = lineStr.find_first_of('"');
		int place1 = lineStr.find_first_of(',', place0+1);
		if (placeS<0)
		{
			int place2 = lineStr.find_first_of(',', place1+1);
			Acronym = lineStr.substr(place1+1, place2-place1-1);
		}
		else
		{
			int placeS1 = lineStr.find_first_of('"', placeS+1);
			int place2 = lineStr.find_first_of(',', placeS1+2);
			Acronym = lineStr.substr(placeS1+2, place2-placeS1-2);
		}
		//fscanf(fp, "%d,%s,%s,%s,%s,%s,%s,%s", &code, ObjectClass, Acronym, Attribute_A, Attribute_B, Attribute_C, Class, Primitives);
		code_name_attribute_system[code] = Acronym;
	}
	fclose(fp);
}


//std::vector<std::string> MyChartLoader::get_file_list_in_folder(const char* fFolder)
//{
//	std::vector<std::string> file_list;
//	long Handle;
//	struct _finddata_t FileInfo;
//	std::string folder_file_pattern(fFolder);
//	folder_file_pattern += "*.shp";
//	std::string folder_str(fFolder);
//	if((Handle=_findfirst(folder_file_pattern.c_str(),&FileInfo))==-1L)
//	{
//		printf("û���ҵ�ƥ�����Ŀ\n");
//	}
//	else
//	{
//		file_list.push_back((folder_str+std::string(FileInfo.name)).c_str());
//		while(_findnext(Handle,&FileInfo)==0)
//		{
//			file_list.push_back((folder_str+std::string(FileInfo.name)).c_str());
//		}
//		_findclose(Handle);
//	}
//	return file_list;
//}



std::vector<std::string> MyChartLoader::get_file_list_in_folder(const char* fFolder)
{
	std::vector<std::string> file_list;
	if (!fFolder) return file_list;

#ifdef _WIN32
	// Windows 原逻辑
	long Handle;
	struct _finddata_t FileInfo;
	std::string folder_file_pattern(fFolder);
	folder_file_pattern += "*.shp";
	std::string folder_str(fFolder);

	if ((Handle = _findfirst(folder_file_pattern.c_str(), &FileInfo)) == -1L) {
		printf("No matching files found\n");
	}
	else {
		file_list.push_back(folder_str + FileInfo.name);
		while (_findnext(Handle, &FileInfo) == 0) {
			file_list.push_back(folder_str + FileInfo.name);
		}
		_findclose(Handle);
	}
#else
	// Linux / POSIX 实现
	DIR* dir = opendir(fFolder);
	if (!dir) {
		printf("Cannot open directory %s\n", fFolder);
		return file_list;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
		// 只处理普通文件
		if (entry->d_type == DT_REG) {
			std::string name(entry->d_name);
			// 只匹配 .shp 文件
			if (name.size() >= 4 && name.substr(name.size() - 4) == ".shp") {
				std::string full_path(fFolder);
				if (full_path.back() != '/') full_path += '/';
				full_path += name;
				file_list.push_back(full_path);
			}
		}
	}
	closedir(dir);
#endif

	return file_list;
}

int MyChartLoader::get_id_from_attribute_name(std::string name_str)
{
	int code = 0;
	for (int i=0; i<code_name_attribute_system.size(); i++)
	{
		if (code_name_attribute_system[i] == name_str)
		{
			code = i;
			break;
		}
	}
	//if (code==0) printf("Unkonw attribute name\n");
	return code;
}

int MyChartLoader::get_id_from_object_name(std::string name_str)
{
	int code = 0;
	for (int i=0; i<code_name_system.size(); i++)
	{
		if (code_name_system[i] == name_str)
		{
			code = i;
			break;
		}
	}
	if (code==0) printf("Unkonw object name\n");
	return code;
}

void MyChartLoader::get_edge_id_in_ring(OGRLinearRing* ring, bool isExterior, std::vector<fspt_t>& ring_fspts)
{
	static char coord_str[100];
	std::vector<int> ring_x_list, ring_y_list;
	std::vector<std::string> ring_coord_str_list;
	int ring_point_count = ring->getNumPoints();

	for (int iPoint=0; iPoint<ring_point_count; iPoint++)
	{
		int temp_x = (int)(ring->getX(iPoint) * dspm.coord_mult_factor);
		int temp_y = (int)(ring->getY(iPoint) * dspm.coord_mult_factor);
		if (iPoint!=0 && temp_x==ring_x_list[ring_x_list.size()-1] && temp_y==ring_y_list[ring_y_list.size()-1])
			continue;

		ring_x_list.push_back(temp_x);
		ring_y_list.push_back(temp_y);

		sprintf_s(coord_str, "%ld_%ld", temp_x, temp_y);
		ring_coord_str_list.push_back(std::string(coord_str));
	}
	std::vector<int> ring_conn_node_list;
	std::vector<int> ring_conn_point_idx_list;
	//////////////////////////////////-- ����ȷ���˵���Connected Node --////////////////////////////////////////
	std::string temp_coord_str =ring_coord_str_list[0];
	if (connected_node_coord_cache.find(temp_coord_str) == connected_node_coord_cache.end())
	{
		int temp_x = (int)(ring->getX(0) * dspm.coord_mult_factor);
		int temp_y = (int)(ring->getY(0) * dspm.coord_mult_factor);
		sprintf_s(coord_str, "%ld_%ld", temp_x, temp_y);

		ConnectedNodeVector temp_node;
		temp_node.sg2d.long_lat[0] = temp_x;
		temp_node.sg2d.long_lat[1] = temp_y;

		temp_node.vrid.rcnm = 120;
		temp_node.vrid.rcid = connected_node_vec.size();
		temp_node.vrid.rver = 1;
		temp_node.vrid.ruin = 1;

		temp_node._sizeFieldPos = 2;
		temp_node._sizeFieldLength = 2;
		connected_node_vec.push_back(temp_node);
		connected_node_coord_cache[coord_str] = temp_node.vrid.rcid;
	}
	int interval_count = 0;
	for (int iPoint=0; iPoint<ring_coord_str_list.size(); iPoint++)
	{
		std::string temp_coord_str =ring_coord_str_list[iPoint];
		if (connected_node_coord_cache.find(temp_coord_str) != connected_node_coord_cache.end())
		{
			ring_conn_point_idx_list.push_back(iPoint);
			ring_conn_node_list.push_back(connected_node_coord_cache[temp_coord_str]);
			interval_count=0;
		}
		else
		{
			interval_count++;
		}
		if (interval_count>=1000)
		{
			int temp_x = ring_x_list[iPoint];
			int temp_y = ring_y_list[iPoint];
			sprintf_s(coord_str, "%ld_%ld", temp_x, temp_y);

			ConnectedNodeVector temp_node;
			temp_node.sg2d.long_lat[0] = temp_x;
			temp_node.sg2d.long_lat[1] = temp_y;

			temp_node.vrid.rcnm = 120;
			temp_node.vrid.rcid = connected_node_vec.size();
			temp_node.vrid.rver = 1;
			temp_node.vrid.ruin = 1;

			temp_node._sizeFieldPos = 2;
			temp_node._sizeFieldLength = 2;
			connected_node_vec.push_back(temp_node);
			connected_node_coord_cache[coord_str] = temp_node.vrid.rcid;
			
			iPoint--;
			interval_count--;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	std::vector<int> edge_id_list;
	std::vector<bool> edge_reverse_list;
	if (ring_conn_node_list.size()>0)
	{
		for (int iConn=0; iConn<ring_conn_node_list.size()-1; iConn++)
		{
			int temp_conn_idx1 = ring_conn_node_list[iConn];
			int temp_conn_idx2 = ring_conn_node_list[iConn+1];
			int temp_point_idx1 = ring_conn_point_idx_list[iConn];
			int temp_point_idx2 = ring_conn_point_idx_list[iConn+1];
			bool reversed = false;
			int temp_edge_id = get_edge_with_node_point(temp_conn_idx1, temp_conn_idx2, temp_point_idx1, temp_point_idx2, ring_x_list, ring_y_list, reversed);
			if (temp_edge_id>=0)
			{
				edge_id_list.push_back(temp_edge_id);
				edge_reverse_list.push_back(reversed);
			}
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		int front_node_id, end_node_id;
		{
			int temp_x = (int)(ring->getX(0) * dspm.coord_mult_factor);
			int temp_y = (int)(ring->getY(0) * dspm.coord_mult_factor);
			sprintf_s(coord_str, "%ld_%ld", temp_x, temp_y);

			ConnectedNodeVector temp_node;
			temp_node.sg2d.long_lat[0] = temp_x;
			temp_node.sg2d.long_lat[1] = temp_y;

			temp_node.vrid.rcnm = 120;
			temp_node.vrid.rcid = connected_node_vec.size();
			temp_node.vrid.rver = 1;
			temp_node.vrid.ruin = 1;

			temp_node._sizeFieldPos = 2;
			temp_node._sizeFieldLength = 2;
			connected_node_vec.push_back(temp_node);
			connected_node_coord_cache[coord_str] = temp_node.vrid.rcid;

			front_node_id = temp_node.vrid.rcid;
			end_node_id = temp_node.vrid.rcid;
		}
		
		std::vector<int> x_list;
		std::vector<int> y_list;
		for (int iPoint=1; iPoint<ring_point_count-1; iPoint++)
		{
			int temp_x = (int)(ring->getX(iPoint) * dspm.coord_mult_factor);
			int temp_y = (int)(ring->getY(iPoint) * dspm.coord_mult_factor);
			x_list.push_back(temp_x);
			y_list.push_back(temp_y);
		}//end for points

		//////////////////////////////////////////////////////////////////////////
		EdgeVector temp_edge;
		temp_edge.vrid.rcnm = 130;
		temp_edge.vrid.rcid = edge_node_vec.size();
		temp_edge.vrid.rver = 1;
		temp_edge.vrid.ruin = 1;

		vrpt_t temp_vrpt_front;
		temp_vrpt_front.rcnm = connected_node_vec[front_node_id].vrid.rcnm;
		temp_vrpt_front.rcid = connected_node_vec[front_node_id].vrid.rcid;
		temp_vrpt_front.topi = 1;
		temp_vrpt_front.mask = 255;
		temp_vrpt_front.usag = 255;
		temp_vrpt_front.ornt = 255;
		temp_edge.vrpts.push_back(temp_vrpt_front);

		vrpt_t temp_vrpt_end;
		temp_vrpt_end.rcnm = connected_node_vec[end_node_id].vrid.rcnm;
		temp_vrpt_end.rcid = connected_node_vec[end_node_id].vrid.rcid;
		temp_vrpt_end.topi = 2;
		temp_vrpt_end.mask = 255;
		temp_vrpt_end.usag = 255;
		temp_vrpt_end.ornt = 255;
		temp_edge.vrpts.push_back(temp_vrpt_end);

		for (int iNode=0; iNode<x_list.size(); iNode++)
		{
			sg2d_t temp_sg2d;
			temp_sg2d.long_lat[0] = x_list[iNode];
			temp_sg2d.long_lat[1] = y_list[iNode];
			temp_edge.sg2ds.push_back(temp_sg2d);
		}

		temp_edge._sizeFieldPos = 2;
		temp_edge._sizeFieldLength = 5;
		edge_node_vec.push_back(temp_edge);

		//////////////////////////////////////////////////////////////////////////
		edge_id_list.push_back(temp_edge.vrid.rcid);
		edge_reverse_list.push_back(false);
	}
	//////////////////////////////////////////////////////////////////////////
	for (int iEdge=0; iEdge<edge_id_list.size(); iEdge++)
	{
		fspt_t temp_fspt;
		temp_fspt.rcnm = 130;
		temp_fspt.rcid = edge_id_list[iEdge];
		if (edge_reverse_list[iEdge]==false) temp_fspt.ornt = 1;
		else temp_fspt.ornt = 2;
		if (isExterior) temp_fspt.usag = 1;
		else temp_fspt.usag = 2;
		temp_fspt.mask = 2;
		ring_fspts.push_back(temp_fspt);
	}
}

int MyChartLoader::get_edge_with_node_point(int node1, int node2, int idx1, int idx2, std::vector<int>& x_list, std::vector<int>& y_list, bool& reversed)
{
	int cur_point_count = idx2-idx1+1;
	bool existing=false;
	reversed=false;
	int edge_id = -1;
	for (int iEdge=0; iEdge<edge_node_vec.size(); iEdge++)
	{
		EdgeVector temp_edge = edge_node_vec[iEdge];
		if (temp_edge.sg2ds.size() + 2 != cur_point_count)
		{
			continue;
		}

		int temp_node_id1, temp_node_id2;
		if (temp_edge.vrpts[0].topi==1)
		{
			temp_node_id1 = temp_edge.vrpts[0].rcid;
			temp_node_id2 = temp_edge.vrpts[1].rcid;
		}
		else
		{
			temp_node_id1 = temp_edge.vrpts[1].rcid;
			temp_node_id2 = temp_edge.vrpts[0].rcid;
		}

		if (temp_node_id1 == node1 && temp_node_id2 == node2)
		{
			bool difference=false;
			int ring_point_acc = 1;
			for (int iPoint=0; iPoint<temp_edge.sg2ds.size(); iPoint++)
			{
				int temp_edge_x = temp_edge.sg2ds[iPoint].long_lat[0];
				int temp_edge_y = temp_edge.sg2ds[iPoint].long_lat[1];
				int temp_ring_x = x_list[idx1+ring_point_acc];
				int temp_ring_y = y_list[idx1+ring_point_acc];
				ring_point_acc++;
				if (temp_edge_x!=temp_ring_x || temp_edge_y!=temp_ring_y)
				{
					difference = true;
					break;
				}
			}
			existing = !difference;
		}
		else if (temp_node_id2 == node1 && temp_node_id1 == node2)
		{
			bool difference=false;
			int ring_point_acc = 1;
			for (int iPoint=temp_edge.sg2ds.size()-1; iPoint>=0; iPoint--)
			{
				int temp_edge_x = temp_edge.sg2ds[iPoint].long_lat[0];
				int temp_edge_y = temp_edge.sg2ds[iPoint].long_lat[1];
				int temp_ring_x = x_list[idx1+ring_point_acc];
				int temp_ring_y = y_list[idx1+ring_point_acc];
				ring_point_acc++;
				if (temp_edge_x!=temp_ring_x || temp_edge_y!=temp_ring_y)
				{
					difference = true;
					break;
				}
			}
			reversed = true;
			existing = !difference;
		}
		if (existing) 
		{
			edge_id = iEdge;
			break;
		}
	}
	if (existing==false)
	{
		EdgeVector temp_edge;
		temp_edge.vrid.rcnm = 130;
		temp_edge.vrid.rcid = edge_node_vec.size();
		temp_edge.vrid.rver = 1;
		temp_edge.vrid.ruin = 1;

		vrpt_t temp_vrpt_front;
		temp_vrpt_front.rcnm = connected_node_vec[node1].vrid.rcnm;
		temp_vrpt_front.rcid = connected_node_vec[node1].vrid.rcid;
		temp_vrpt_front.topi = 1;
		temp_vrpt_front.mask = 255;
		temp_vrpt_front.usag = 255;
		temp_vrpt_front.ornt = 255;
		temp_edge.vrpts.push_back(temp_vrpt_front);

		vrpt_t temp_vrpt_end;
		temp_vrpt_end.rcnm = connected_node_vec[node2].vrid.rcnm;
		temp_vrpt_end.rcid = connected_node_vec[node2].vrid.rcid;
		temp_vrpt_end.topi = 2;
		temp_vrpt_end.mask = 255;
		temp_vrpt_end.usag = 255;
		temp_vrpt_end.ornt = 255;
		temp_edge.vrpts.push_back(temp_vrpt_end);

		for (int iNode=idx1+1; iNode<idx2; iNode++)
		{
			sg2d_t temp_sg2d;
			temp_sg2d.long_lat[0] = x_list[iNode];
			temp_sg2d.long_lat[1] = y_list[iNode];
			temp_edge.sg2ds.push_back(temp_sg2d);
		}

		temp_edge._sizeFieldPos = 2;
		temp_edge._sizeFieldLength = 5;
		edge_node_vec.push_back(temp_edge);

		edge_id = temp_edge.vrid.rcid;
		reversed = false;
	}
	return edge_id;
}

