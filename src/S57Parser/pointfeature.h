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

#ifndef POINTFEATURE_H_
#define POINTFEATURE_H_

#include <iostream>
#include <vector>

#include "feature.h"
#include "isolatednodevector.h"
#include "connectednodevector.h"
#include "s57.h"
#include "iso8211.h"
//! #include "cvg.h"

/**
 * Point feature
 */
class PointFeature : public Feature
{
    public:
        PointFeature();
		~PointFeature();
		//! void Draw(double zoom);
        int SetLongLat(std::map<int, IsolatedNodeVector>
                                        &isolated_node_vectors_map, 
                                std::map<int, ConnectedNodeVector> 
                                        &connected_node_vectors_map);
		bool CheckIs3D(std::map<int, IsolatedNodeVector>
										&isolated_node_vectors_map);
        //! CVG *cvg;
        double long_lat[2];
		double depth;
		bool is_3d;
    private:
        int ref_record_id; 
};

#endif
