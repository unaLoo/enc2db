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

#include "edgevector.h"
#include "util.h"
#include <math.h>

/**
 * Constructor
 */
EdgeVector::EdgeVector() :
    beg_node(-1), end_node(-1), hasSimplified(false), simplifyFlag(-1), hasCompleted(false), hasLinked(false), hasSplited(false)
{
	hasCheckedSame = false;
	checkedId = -1;
	mLineString = NULL;
	mWithinRect = false;
	isBoundary = false;
}

//EdgeVector::EdgeVector(const EdgeVector& C)
//{
//	mLineString = NULL;
//}
/**
 * Destructor
 */
EdgeVector::~EdgeVector()
{
	if (mLineString) 
		OGRGeometryFactory::destroyGeometry(mLineString);
}

/**
 * Load edge vector data from a record
 * @param record Record to read
 * @return 0 on success, -1 on failure
 */
int EdgeVector::Load(DDFRecord *record, double& extent_xmax, double& extent_xmin, double& extent_ymax, double& extent_ymin)
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

	if (vrid.rcid == 805)
	{
		int  aa  = 0;	
	}


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

	bool vrpt_read = false;
    for (int i = 2; i < field_count; i++) {
        field = record->GetField(i);
        if (!field) return -1;
        field_defn = field->GetFieldDefn();
        if (!field_defn) return -1;
        field_data = field->GetData();
        if (!field_data) return -1;
        bytes_remaining = field->GetDataSize();

        field_name = field_defn->GetName();

        if (strncmp(field_name, "VRPT", 4) == 0) 
		{
			if (vrpt_read==true) continue;
			vrpt_read = true;

            int repeat_count = field->GetRepeatCount();

            for (int j = 0; j < repeat_count; j++) {
                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) return -1;
                unsigned char *bstr;
                bstr = (unsigned char *)subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
                int rcnm_rcid[2];
                decode_name_string(bstr, bytes_consumed, rcnm_rcid);
				vrpt_t temp_vrpt;
				temp_vrpt.rcnm = rcnm_rcid[0];
                temp_vrpt.rcid = rcnm_rcid[1];
                field_data += bytes_consumed;
                bytes_remaining -= bytes_consumed;

				// ornt
				subfield_defn = field_defn->GetSubfield(1);
				if (!subfield_defn) return -1;
				temp_vrpt.ornt = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
				bytes_remaining -= bytes_consumed;
				field_data += bytes_consumed;

				// usag
				subfield_defn = field_defn->GetSubfield(2);
				if (!subfield_defn) return -1;
				temp_vrpt.usag = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
				bytes_remaining -= bytes_consumed;
				field_data += bytes_consumed;

				// topi
                subfield_defn = field_defn->GetSubfield(3);
                if (!subfield_defn) return -1;
                temp_vrpt.topi = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                if (temp_vrpt.topi == 1)
                    beg_node = temp_vrpt.rcid;
                else if (temp_vrpt.topi == 2)
                    end_node = temp_vrpt.rcid;
                else {
                    std::cerr << "EdgeVector::Load(): Bad TOPI number " << temp_vrpt.topi << std::endl;
                    return -1;
                }

                // mask
                subfield_defn = field_defn->GetSubfield(4);
                if (!subfield_defn)  return -1;
                temp_vrpt.mask = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

				vrpts.push_back(temp_vrpt);
            }
        } 
        
        else if (strncmp(field_name, "ATTV", 4) == 0) {
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
            int repeat_count = field->GetRepeatCount();

            for (int j = 0; j < repeat_count; j++) {
                sg2d_t sg2d;

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
				
				sg2d.save_flag = true;
                sg2ds.push_back(sg2d);
            }
        } 

        else {
            std::cerr << "EdgeVector::Load(): Unknown field "
                      << field_name << std::endl;
            return -1;
        }

    }
	hasLinked = false;
	//////////////////////////////////////////////////////////////////////////
	int num_sg2ds = sg2ds.size();
	if(extent_xmax == 0.0 && extent_xmin==0.0 && 
		extent_ymax == 0.0 && extent_ymin==0.0)
	{
		extent_xmin =  sg2ds[0].long_lat[0];
		extent_xmax = sg2ds[0].long_lat[0];
		extent_ymin =  sg2ds[0].long_lat[1];
		extent_ymax = sg2ds[0].long_lat[1];
	}
	for (int i = 1; i < num_sg2ds; i++) {
		if (extent_xmax < sg2ds[i].long_lat[0]) extent_xmax = sg2ds[i].long_lat[0];
		if (extent_xmin > sg2ds[i].long_lat[0]) extent_xmin = sg2ds[i].long_lat[0];
		if (extent_ymax < sg2ds[i].long_lat[1]) extent_ymax = sg2ds[i].long_lat[1];
		if (extent_ymin > sg2ds[i].long_lat[1]) extent_ymin = sg2ds[i].long_lat[1];
	}
    return vrid.rcid;
}

/**
 * Complete an edge given a connected node vectors map
 * @param connected_node_vectors_map Map to use
 * @return 0 on success, -1 on failure
 */
int EdgeVector::CompleteEdge(std::map<int, ConnectedNodeVector>
                                    &connected_node_vectors_map)
{
	std::map<int, ConnectedNodeVector>::iterator it_begin = connected_node_vectors_map.find(beg_node);
	std::map<int, ConnectedNodeVector>::iterator it_end = connected_node_vectors_map.find(end_node);
	if (connected_node_vectors_map.size() == 0 ||
		it_begin == connected_node_vectors_map.end() ||
		it_end == connected_node_vectors_map.end() )
	{
		hasCompleted = false;
		return -1;
	}
    sg2d_t *tmp = connected_node_vectors_map[beg_node].GetSG2DPtr();
    if (tmp) {
        sg2ds.insert(sg2ds.begin(), *tmp);

        tmp = connected_node_vectors_map[end_node].GetSG2DPtr();
        if (!tmp) {
            std::cerr << "EdgeVector::CompleteEdge(): Couldn't find end"
                      << " node in connected node vector map" << std::endl;
            return -1;
        }
        sg2ds.push_back(*tmp);
		hasCompleted = true;
    }

    return 0;
}

/**
 * Draw tesselation verticies (for areas)
 * @param tobj GLUtesselator object to use
 */
//void EdgeVector::DrawTessVertices(GLUtesselator *tobj)
//{
//    int node_count = sg2ds.size();
//    for (int i = 0; i < node_count-1; i++)
//        gluTessVertex(tobj, sg2ds[i].long_lat, sg2ds[i].long_lat);
//}

/**
 * Divide coordinates by a factor
 * @param coord_mult_factor factor
 */
void EdgeVector::Normalize(float coord_mult_factor)
{
    int num_sg2ds = sg2ds.size();
    for (int i = 0; i < num_sg2ds; i++) {
        sg2ds[i].long_lat[0] /= coord_mult_factor;
        sg2ds[i].long_lat[1] /= coord_mult_factor;
    }
}

/**
 * Draw an edge
 */
//void EdgeVector::Draw()
//{
//    int node_count = sg2ds.size();
//    //double last_drawn_line[2][2];
//
//    glBegin(GL_LINE_STRIP);
//
//    //if (lod == HIGH) {
//        for (int i = 0; i < node_count; i++)
//            glVertex2dv(sg2ds[i].long_lat);
//    /*} else if (lod == LOW) {
//        glVertex2dv(sg2ds[0].long_lat);
//        glVertex2dv(sg2ds[1].long_lat);
//        last_drawn_line[0][0] = sg2ds[0].long_lat[0];
//        last_drawn_line[0][1] = sg2ds[0].long_lat[1];
//        last_drawn_line[1][0] = sg2ds[1].long_lat[0];
//        last_drawn_line[1][1] = sg2ds[1].long_lat[1];
//        for (int i = 2; i < node_count-1; i++) {
//            double theta = calc_angle(last_drawn_line[0],
//                                      last_drawn_line[1],
//                                      sg2ds[i].long_lat);
//
//            double distance = calc_length(last_drawn_line[1],
//                                          sg2ds[i].long_lat);
//
//            if (theta/distance < 6) {
//                glVertex2dv(sg2ds[i].long_lat);
//                last_drawn_line[0][0] = last_drawn_line[1][0];
//                last_drawn_line[0][1] = last_drawn_line[1][1];
//                last_drawn_line[1][0] = sg2ds[i].long_lat[0];
//                last_drawn_line[1][1] = sg2ds[i].long_lat[1];
//            }
//        }
//        glVertex2dv(sg2ds[node_count-1].long_lat);
//    }
//    */
//
//    glEnd();
//}
