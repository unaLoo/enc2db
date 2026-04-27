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

#ifndef FEATURE_H_
#define FEATURE_H_

#include <iostream>
#include <vector>

#include "s57.h"
#include "iso8211.h"

/**
 * Abstract base class for chart features.
 * A feature does not contain the actual longitude/latitudes of points/lines.
 * Instead it holds one or more references to another record (which may
 * be an isolated node, edge node, connected node, etc).  We often pass
 * an std::map with the mapping between the reference id and the actual
 * node object for methods in child classes.
 */
class Feature
{
    public:
        Feature();
        virtual ~Feature() = 0;
        int Load(DDFRecord *record);
    protected:

        std::string name;

	public:

		std::vector<int> ref_record_ids;
		//////////////////////////////////////////////////////////////////////////
		int _sizeFieldPos;
		int _sizeFieldLength;

		//////////////////////////////////////////////////////////////////////////
		frid_t frid;
		foid_t foid;
        std::vector<attf_t> attfs;
        std::vector<attf_t> natfs;
        std::vector<ffpt_t> ffpts;
		std::vector<fspt_t> fspts;
		fspc_t fspc;
		ffpc_t ffpc;

		//////////////////////////////////////////////////////////////////////////
		bool save_flag;
};

#endif
