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
#include "pointfeature.h"
#include "util.h"

 /**
  * Constructor
  */
PointFeature::PointFeature() :
	ref_record_id(0)
{
	long_lat[0] = long_lat[1] = 0;
	depth = 0;
	is_3d = false;
}

/**
 * Destructor
 */
PointFeature::~PointFeature()
{
}

/**
 * Set longitude/latitude of this point
 * @param isolated_node_vectors_map node:vector map to find point with
 * @return 0 on success -1 on failure
 */
int PointFeature::SetLongLat(std::map<int, IsolatedNodeVector>& isolated_node_vectors_map,
	std::map<int, ConnectedNodeVector>& connected_node_vectors_map)
{
	sg2d_t* sg2d = NULL;
	if (isolated_node_vectors_map.find(ref_record_ids[0]) == isolated_node_vectors_map.end())
	{
		if (connected_node_vectors_map.find(ref_record_ids[0]) != connected_node_vectors_map.end())
		{
			sg2d = connected_node_vectors_map[ref_record_ids[0]].GetSG2DPtr();
		}
	}
	else
	{
		sg2d = isolated_node_vectors_map[ref_record_ids[0]].GetSG2DPtr();
	}
	if (sg2d == NULL) return -1;

	if (this->fspts[0].rcnm == 120) sg2d = connected_node_vectors_map[ref_record_ids[0]].GetSG2DPtr();

	if (sg2d->long_lat[0]==0.0 && sg2d->long_lat[1]==0.0)
	{
		std::vector<sg3d_t> sg3dList = isolated_node_vectors_map[ref_record_ids[0]].GetSG3DVector();
		long_lat[0] = sg3dList[0].long_lat[0];
		long_lat[1] = sg3dList[0].long_lat[1];
		is_3d= true;
		depth = sg3dList[0].depth;
	}
	else
	{
		long_lat[0] = sg2d->long_lat[0];
		long_lat[1] = sg2d->long_lat[1];
	}
    return 0;
}

bool PointFeature::CheckIs3D(std::map<int, IsolatedNodeVector>
	&isolated_node_vectors_map)
{
	if (isolated_node_vectors_map.find(ref_record_ids[0]) == isolated_node_vectors_map.end())
	{
		return false;
	}
	if (isolated_node_vectors_map[ref_record_ids[0]].is2d==false) is_3d = true;
	else is_3d = false;
	return is_3d;
}

//void PointFeature::Draw(double zoom)
//{
//    if (cvg) {
//        glPushMatrix();
//            glTranslatef(long_lat[0], long_lat[1], 0);
//            glScalef(1/zoom, 1/zoom, 0);
//            cvg->Render();
//        glPopMatrix();
//    }
//}
