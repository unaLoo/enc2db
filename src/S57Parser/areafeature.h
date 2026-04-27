#ifndef AREAFEATURE_H_
#define AREAFEATURE_H_

#include <iostream>
#include <vector>
#include <map>


#include "feature.h"
#include "edgevector.h"
#include "s57.h"
#include "iso8211.h"

class AreaFeature : public Feature
{
    public:
        AreaFeature();
        ~AreaFeature();
        int CompleteContours(std::map<int, EdgeVector> &edge_vectors_map);
        //! void Draw(std::map<int, EdgeVector> &edge_vectors_map);
    
        std::vector<std::vector<int > > contours;
		std::vector<int> relatedLineFeatureId;

		bool hasChecked;
};

#endif
