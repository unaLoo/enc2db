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

#include <map>
#include "linefeature.h"
#include "util.h"

/**
 * Constructor
 */
LineFeature::LineFeature()
{
}

/**
 * Destructor
 */
LineFeature::~LineFeature()
{
}

/**
 * Get an std::vector containing the sg2ds for this line.
 * @param edge_vectors_map Edge vector map to use
 * @return std::vector of sg2d_t
 */
std::vector<sg2d_t> LineFeature::GetSG2DsVector(int index, 
        std::map<int, EdgeVector> &edge_vectors_map)
{
    std::vector<sg2d_t> sg2ds;

	std::vector<sg2d_t> sg2ds_tmp = edge_vectors_map[ref_record_ids[index]].GetSG2Ds();
    sg2ds.insert(sg2ds.end(), sg2ds_tmp.begin(), sg2ds_tmp.end()); 
    
    return sg2ds;
}

/**
 * Draw a line feature
 */
//void LineFeature::Draw(std::map<int, EdgeVector> &edge_vectors_map)
//{
//    int ref_count = ref_record_ids.size();
//    for (int i = 0; i < ref_count; i++)
//        edge_vectors_map[ref_record_ids[i]].Draw();
//}

std::pair<int, int> LineFeature::GetBegEndNodes(
                        std::map<int, EdgeVector> &edge_vectors_map)
{
    int beg, end;
    std::vector<int>::iterator it;
    it = ref_record_ids.begin();
    beg = edge_vectors_map[*it].beg_node;

    it = ref_record_ids.end();
    end = edge_vectors_map[*it].end_node;
    
    return std::make_pair(beg, end);
}
