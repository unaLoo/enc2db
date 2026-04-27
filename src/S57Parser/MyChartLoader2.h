#ifndef CHART_H_
#define CHART_H_

#include <map>
#include <vector>

#include "iso8211.h"
#include "dsid.h"
#include "dspm.h"
#include "isolatednodevector.h"
#include "connectednodevector.h"
#include "edgevector.h"

#include "feature.h"
#include "areafeature.h"
#include "linefeature.h"
#include "pointfeature.h"

class MyChartLoader
{
public:
	MyChartLoader(void);
	~MyChartLoader(void);

	DDFModule current_module;

	std::map<int, std::vector<LineFeature> > line_features_map;
	std::map<int, std::vector<PointFeature> > point_features_map;
	std::map<int, std::vector<AreaFeature> > area_features_map;
	
public:
	std::vector<std::string> get_file_list_in_folder(const char* fFolder);
	int get_id_from_attribute_name(std::string name_str);
	int get_id_from_object_name(std::string name_str);
	void get_edge_id_in_ring(OGRLinearRing* ring, bool isExterior, std::vector<fspt_t>& ring_fspts);
	int get_edge_with_node_point(int node1, int node2, int idx1, int idx2, std::vector<int>& x_list, std::vector<int>& y_list, bool& reversed);

	std::map<std::string, int> connected_node_coord_cache;

public:
	int S57Load(const char* fpath);
	int S57LoadUpdate(const char* fpath);
	
	int SetupFeatures();
	int ReBuildIdReference();

	void LoadCodeNameSystem(std::string resourceFolder);

	void GetChartExtent();
	int GetSuitableLevel();
private:
	int S57ReadDSIDRecord(DDFRecord *record);
	int S57ReadDSPMRecord(DDFRecord *record);

	int S57ReadFeatureRecord(DDFRecord *record);
	int S57LoadPointFeature(DDFRecord *record);
	int S57LoadLineFeature(DDFRecord *record);
	int S57LoadAreaFeature(DDFRecord *record);

	int S57ReadVectorRecord(DDFRecord *record);
	int S57LoadIsolatedNodeVectorRecord(DDFRecord *record);
	int S57LoadConnectedNodeVectorRecord(DDFRecord *record);
	int S57LoadEdgeVectorRecord(DDFRecord *record);

	void S57NormalizeCoordinates();
	int S57SetupEdgeVectors();
	int S57SetupAreaFeatures();

public:
	// vectors maps
	std::map<int, IsolatedNodeVector> isolated_node_vectors_map;
	std::map<int, ConnectedNodeVector> connected_node_vectors_map;
	std::map<int, EdgeVector> edge_vectors_map;

	std::vector<IsolatedNodeVector> isolated_node_vec;
	std::vector<ConnectedNodeVector> connected_node_vec;
	std::vector<EdgeVector> edge_node_vec;
	std::vector<Feature*> feature_vec;

	// meta data loading
	DSID dsid;
	DSPM dspm;
	bool dsid_loaded;
	bool dspm_loaded;

	double extent_xmax, extent_xmin, extent_ymax, extent_ymin;
	std::string chart_file_name;
	int m_cscl_level;

	// code-name system
	std::map<int, std::string> code_name_system;
	std::map<int, std::string> code_name_attribute_system;
};

#endif