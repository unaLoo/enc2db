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

#ifndef ISOLATEDNODEVECTOR_H_
#define ISOLATEDNODEVECTOR_H_

#include <vector>
#include <string>
#include <iostream>

#include "s57.h"
#include "iso8211.h"

/**
 * An isolated node represents a spatial point; it is not connected
 * to any other nodes.
 */
class IsolatedNodeVector
{
    public:
        IsolatedNodeVector(); 
        ~IsolatedNodeVector();
        int Load(DDFRecord *record, double& extent_xmax, double& extent_xmin, double& extent_ymax, double& extent_ymin);
        void Normalize(float coord_mult_factor, float sounding_mult_factor);
        sg2d_t *GetSG2DPtr() { return &sg2d; }
		std::vector<sg3d_t> GetSG3DVector() { return sg3ds; }
	

		//////////////////////////////////////////////////////////////////////////
		int _sizeFieldPos;
		int _sizeFieldLength;

		//////////////////////////////////////////////////////////////////////////
		bool is2d;
		vrid_t vrid;
		std::vector<attv_t> attvs;
        sg2d_t sg2d;
        std::vector<sg3d_t> sg3ds;

};

#endif
