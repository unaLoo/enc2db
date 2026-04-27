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

#include "isolatednodevector.h"
#include "util.h"

/**
 * Constructor
 */
IsolatedNodeVector::IsolatedNodeVector()
{
    sg2d.long_lat[0] = sg2d.long_lat[1] = 0;
	sg2d.save_flag = true;
	is2d = true;
}

/**
 * Destructor
 */
IsolatedNodeVector::~IsolatedNodeVector()
{
}

/**
 * Load isolated node data from a record
 * @param record Record to read
 * @return 0 on success, -1 on failure
 */
int IsolatedNodeVector::Load(DDFRecord *record, double& extent_xmax, double& extent_xmin, double& extent_ymax, double& extent_ymin)
{
    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

    int field_count = record->GetFieldCount();

	_sizeFieldPos = record->getFieldPos();
	_sizeFieldLength = record->getFieldLength();

	//////////////////////////////////////////////////////////////////////////
	//
    // Read the VRID field
	//
	//////////////////////////////////////////////////////////////////////////
    field = record->GetField(1);
    if (!field) return -1;
    field_defn = field->GetFieldDefn();
    if (!field_defn) return -1;
    field_name = field_defn->GetName();
    field_data = field->GetData();
    if (!field_data) return -1;
    bytes_remaining = field->GetDataSize();

    // record name
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn) return -1;
    vrid.rcnm = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	// record id
    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn) return -1;
    vrid.rcid = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	// record version
	subfield_defn = field_defn->GetSubfield(2);
	if (!subfield_defn) return -1;
	vrid.rver = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

	// update instruction
	subfield_defn = field_defn->GetSubfield(3);
	if (!subfield_defn) return -1;
	vrid.ruin = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

	//////////////////////////////////////////////////////////////////////////
    // Now read the next fields
    for (int i = 2; i < field_count; i++) {
        field = record->GetField(i);
        if (!field) return -1;
        field_defn = field->GetFieldDefn();
        if (!field_defn) return -1;
        field_data = field->GetData();
        if (!field_data) return -1;
        bytes_remaining = field->GetDataSize();

        field_name = field_defn->GetName();

        if (strncmp(field_name, "ATTV", 4) == 0) {
            int repeat_count = field->GetRepeatCount();

            for (int j = 0; j < repeat_count; j++) {
                attv_t attv;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) return -1;
                attv.attl = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) return -1;
                attv.atvl = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                attvs.push_back(attv); 
            }
        }

		else if (strncmp(field_name, "SG2D", 4) == 0) {
			is2d = true;
            int repeat_count = field->GetRepeatCount();
            if (repeat_count > 1) {
                std::cerr << "IsolatedNodeVector::Load(): More than one" << "sg2d" << std::endl;
                return -1;
            }

            subfield_defn = field_defn->GetSubfield(0);
            if (!subfield_defn) return -1;
            int ycoo = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
            bytes_remaining -= bytes_consumed;
            field_data += bytes_consumed;
            
            subfield_defn = field_defn->GetSubfield(1);
            if (!subfield_defn) return -1;
            int xcoo = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
            bytes_remaining -= bytes_consumed;
            field_data += bytes_consumed;

            sg2d.long_lat[1] = (double)ycoo; //60 * ((double)ycoo);
            sg2d.long_lat[0] = (double)xcoo; //60 * ((double)xcoo);
        }

        else if (strncmp(field_name, "SG3D", 4) == 0) {
			is2d = false;
            int repeat_count = field->GetRepeatCount();

            for (int j = 0; j < repeat_count; j++) {
                sg3d_t sg3d;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) return -1;
                int ycoo = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;
                
                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) return -1;
                int xcoo = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                sg3d.long_lat[1] = (double)ycoo; //60 * ((double)ycoo);
                sg3d.long_lat[0] = (double)xcoo; //60 * ((double)xcoo);

                subfield_defn = field_defn->GetSubfield(2);
                if (!subfield_defn) return -1;
                sg3d.depth = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

				sg3d.save_flag = true;
                sg3ds.push_back(sg3d);
            }
        }
        else {
            std::cerr << "IsolatedNodeVector::Load(): Unknown field " << field_name << std::endl;
            return -1;
        }
	}
	//////////////////////////////////////////////////////////////////////////
	int num_sg3ds = sg3ds.size();
	if (num_sg3ds==0)
	{
		if(extent_xmax == 0.0 && extent_xmin==0.0 && 
			extent_ymax == 0.0 && extent_ymin==0.0)
		{
			extent_xmin =  sg2d.long_lat[0];
			extent_xmax = sg2d.long_lat[0];
			extent_ymin =  sg2d.long_lat[1];
			extent_ymax = sg2d.long_lat[1];
		}
		else
		{
			if (extent_xmax < sg2d.long_lat[0]) extent_xmax = sg2d.long_lat[0];
			if (extent_xmin > sg2d.long_lat[0]) extent_xmin = sg2d.long_lat[0];
			if (extent_ymax < sg2d.long_lat[1]) extent_ymax = sg2d.long_lat[1];
			if (extent_ymin > sg2d.long_lat[1]) extent_ymin = sg2d.long_lat[1];
		}
	}
	else
	{
		if(extent_xmax == 0.0 && extent_xmin==0.0 && 
			extent_ymax == 0.0 && extent_ymin==0.0)
		{
			extent_xmin =  sg3ds[0].long_lat[0];
			extent_xmax = sg3ds[0].long_lat[0];
			extent_ymin =  sg3ds[0].long_lat[1];
			extent_ymax = sg3ds[0].long_lat[1];
		}
		for (int i = 1; i < num_sg3ds; i++) {
			if (extent_xmax < sg3ds[i].long_lat[0]) extent_xmax = sg3ds[i].long_lat[0];
			if (extent_xmin > sg3ds[i].long_lat[0]) extent_xmin = sg3ds[i].long_lat[0];
			if (extent_ymax < sg3ds[i].long_lat[1]) extent_ymax = sg3ds[i].long_lat[1];
			if (extent_ymin > sg3ds[i].long_lat[1]) extent_ymin = sg3ds[i].long_lat[1];
		}
	}
    return vrid.rcid;

}

/**
 * Divide our coordinates by a factor
 * @param coord_mult_factor factor
 */
void IsolatedNodeVector::Normalize(float coord_mult_factor, float sounding_mult_factor)
{
    sg2d.long_lat[0] /= coord_mult_factor;
    sg2d.long_lat[1] /= coord_mult_factor;

    int num_sg3ds = sg3ds.size();
    for (int i = 0; i < num_sg3ds; i++) {
        sg3ds[i].long_lat[0] /= coord_mult_factor;
        sg3ds[i].long_lat[1] /= coord_mult_factor;
		sg3ds[i].depth /= sounding_mult_factor;
    }
}
