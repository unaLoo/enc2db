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

#ifndef EDGEVECTOR_H_
#define EDGEVECTOR_H_

#include <map>
#include <vector>
#include <list>
#include <string>
#include <iostream>

#include "s57.h"
#include "iso8211.h"
#include "connectednodevector.h"
#include "ogr_geometry.h"

/**
 * An edge is a string of SG2Ds with a pointer to a connected node at each
 * end.
 */
class EdgeVector 
{
    public:
        EdgeVector(); 
		//EdgeVector(const EdgeVector& C);
        ~EdgeVector();
        int Load(DDFRecord *record, double& extent_xmax, double& extent_xmin, double& extent_ymax, double& extent_ymin);
        int CompleteEdge(std::map<int, ConnectedNodeVector>
                                    &connected_node_vectors_map);
        
        void Normalize(float coord_mult_factor);
        std::vector<sg2d_t> GetSG2Ds() { return sg2ds; }
        sg2d_t *GetNode(int n) { return &(sg2ds[n]); }
        int GetSize() { return sg2ds.size(); }
        // FIXME find nicer way..
        std::vector<sg2d_t> *GetSG2DsPtr() { return &sg2ds; }
        
        int beg_node;
        int end_node;

		bool getIsClosedEdge()
		{
			sg2d_t firstNode = sg2ds[0];
			sg2d_t lastNode = sg2ds[sg2ds.size()-1];
			if ((firstNode.long_lat[0] == lastNode.long_lat[0])  &&
				(firstNode.long_lat[1] == lastNode.long_lat[1]) )
				return true;
			return false;
		}

		bool getIsSame(EdgeVector& edge1)
		{
			if (this->sg2ds.size() ==0 || edge1.sg2ds.size() ==0)
				return false;
			if (this->sg2ds.size() != edge1.sg2ds.size())
				return false;
			for (int i=0; i<sg2ds.size(); i++)
			{
				if (this->sg2ds[i].long_lat[0] != edge1.sg2ds[i].long_lat[0] ||
					this->sg2ds[i].long_lat[1] != edge1.sg2ds[i].long_lat[1] ||
					this->beg_node != edge1.beg_node || 
					this->end_node != edge1.end_node)
				{
					return false;
				}
			}
			return true;
		}

		void constructLineString()
		{
			mLineString = (OGRLineString*)(OGRGeometryFactory::createGeometry(wkbLineString));
			for (int i=0; i<sg2ds.size(); i++)
			{
				mLineString->addPoint((double)(sg2ds[i].long_lat[0]), (double)(sg2ds[i].long_lat[1]));
			}
		}

		bool judgeContainedInALine(OGRLineString* lineString)
		{
			for (int i=0; i<sg2ds.size(); i++)
			{
				OGRPoint temp_point((double)(sg2ds[i].long_lat[0]), (double)(sg2ds[i].long_lat[1]));
				if (lineString->Contains(&temp_point) == false)
				{
					return false;
				}
			}
			return true;
		}
		
		//////////////////////////////////////////////////////////////////////////
		bool hasCheckedSame;
		int checkedId;

		//////////////////////////////////////////////////////////////////////////
		int _sizeFieldPos;
		int _sizeFieldLength;

		//////////////////////////////////////////////////////////////////////////
		vrid_t vrid;
		std::vector<vrpt_t> vrpts;
        std::vector<attv_t> attvs;
        std::vector<sg2d_t> sg2ds;

		std::vector<sg2d_t> sg2ds_simp;

		//////////////////////////////////////////////////////////////////////////
		bool hasSimplified;
		int simplifyFlag;

		bool hasCompleted;

		bool hasLinked;
		bool hasSplited;

		bool isBoundary;
		//////////////////////////////////////////////////////////////////////////
		OGRLineString* mLineString;
		bool mWithinRect;
};

#endif
