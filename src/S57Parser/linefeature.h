/* GHelm - Nautical Navigation Software
 * Copyright (C) 2004 Jon Michaelchuck
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
 */

#ifndef LINEFEATURE_H_
#define LINEFEATURE_H_

#include <iostream>
#include <vector>

#include "feature.h"
#include "edgevector.h"
#include "s57.h"
#include "iso8211.h"

/**
 * Line feature
 */
class LineFeature : public Feature
{
    public:
        LineFeature();
        ~LineFeature();
        //! void Draw(std::map<int, EdgeVector> &edge_vectors_map);
		int GetLineCount() {return ref_record_ids.size(); }
        std::vector<sg2d_t> GetSG2DsVector(int index, 
                                std::map<int, EdgeVector> &edge_vectors_map);
        std::pair<int, int> GetBegEndNodes(
                                std::map<int, EdgeVector> &edge_vectors_map);

		std::vector<int> relatedAreaFeatureId;
};

#endif
