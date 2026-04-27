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

#include "feature.h"
#include "util.h"

/**
 * Constructor
 */
Feature::Feature() : save_flag(true)
{
}

/**
 * Destructor
 */
Feature::~Feature()
{
}

/**
 * Load feature data from a record
 * @param record Record to read
 * @return -1 on failure, 0 on success
 */
int Feature::Load(DDFRecord *record)
{
    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

    int field_count = record->GetFieldCount();
    if (field_count < 2) {
        std::cerr << "Feature::Load(): Field count < 2" << std::endl;
        return -1;
    }

	_sizeFieldPos = record->getFieldPos();
	_sizeFieldLength = record->getFieldLength();

	//////////////////////////////////////////////////////////////////////////
	//
    // Read the FRID field and associated subfields
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

    // rcnm
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn) return -1;
    frid.rcnm = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	// record id
    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn) return -1;
    frid.rcid = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    if (frid.rcid == 5150)
    {
        int debuggggg = 0;

    }

    // object prim
    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn) return -1;
    frid.prim = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	// group
    subfield_defn = field_defn->GetSubfield(3);
    if (!subfield_defn) return -1;
    frid.grup = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	// objec label
    subfield_defn = field_defn->GetSubfield(4);
    if (!subfield_defn) return -1;
    frid.objl = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	if (frid.rcid==33)
	{
		int a = 0;
	}

	// record ver
	subfield_defn = field_defn->GetSubfield(5);
	if (!subfield_defn) return -1;
	frid.rver = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

	// update ins
	subfield_defn = field_defn->GetSubfield(6);
	if (!subfield_defn) return -1;
	frid.ruin = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

    //delete
    if (frid.ruin == 2) return frid.objl;

	//////////////////////////////////////////////////////////////////////////
    // read the FOID 
    field = record->GetField(2);
    if (!field) return -1;
    field_defn = field->GetFieldDefn();
    if (!field_defn) return -1;
    field_name = field_defn->GetName();
    field_data = field->GetData();
    if (!field_data) return -1;
    bytes_remaining = field->GetDataSize();

    // skip agen
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn) return -1;
    foid.agen = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	// feature_id
    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn) return -1;
    foid.find = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	// feature_subid
    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn) return -1;
    foid.fids = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//////////////////////////////////////////////////////////////////////////
    // Now read the next fields. They could be ATTF<R>, NATF<R>, FFPT<R>, FSPT<R>
    for (int i = 3; i < field_count; i++) 
	{
        field = record->GetField(i);
        if (!field) return -1;
        field_defn = field->GetFieldDefn();
        if (!field_defn) return -1;
        field_data = field->GetData();
        if (!field_data) return -1;
        bytes_remaining = field->GetDataSize();

        field_name = field_defn->GetName();
        if (strncmp(field_name, "ATTF", 4) == 0) 
		{
            int repeat_count = field->GetRepeatCount();
            for (int j = 0; j < repeat_count; j++) {
                attf_t attf;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) return -1;
                attf.attl = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) return -1;
                attf.atvl = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                if (attf.attl == 116)
                    name = attf.atvl;

                attfs.push_back(attf);
            }
        }
        else if (strncmp(field_name, "NATF", 4) == 0) 
		{
            int repeat_count = field->GetRepeatCount();
            for (int j = 0; j < repeat_count; j++) {
                attf_t natf;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) return -1;
                natf.attl = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) return -1;
                natf.atvl="";
				const char* val = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
				for (int i=0; i<bytes_consumed-1; i++)
				{
					natf.atvl.push_back(val[i]);
				}
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                natfs.push_back(natf);
            }
        }
        else if (strncmp(field_name, "FFPT", 4) == 0) 
		{
            int repeat_count = field->GetRepeatCount();
            for (int j = 0; j < repeat_count; j++) {
                ffpt_t ffpt;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) return -1;
                
                unsigned char *bstr = (unsigned char *) subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
                int agen_find_fids[3];
                decode_lnam_string(bstr, bytes_consumed, agen_find_fids);
                field_data += bytes_consumed;
                bytes_remaining -= bytes_consumed;
				ffpt.agen = agen_find_fids[0];
				ffpt.find = agen_find_fids[1];
				ffpt.fids = agen_find_fids[2];

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) return -1;
                ffpt.rind = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(2);
                if (!subfield_defn) return -1;
                ffpt.comt = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                ffpts.push_back(ffpt);
            }
        }
        else if (strncmp(field_name, "FSPT", 4) == 0) 
		{
            int repeat_count = field->GetRepeatCount();
			if (repeat_count>1)
			{
				int a = 0;
			}
            for (int j = 0; j < repeat_count; j++) {
                fspt_t fspt;

                subfield_defn = field_defn->GetSubfield(0);
                if (!subfield_defn) return -1;
                unsigned char *bstr = (unsigned char *)subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
                int rcnm_rcid[2];
                decode_name_string(bstr, bytes_consumed, rcnm_rcid);
                fspt.rcnm = rcnm_rcid[0];
                fspt.rcid = rcnm_rcid[1];
                field_data += bytes_consumed;
                bytes_remaining -= bytes_consumed;

                ref_record_ids.push_back(fspt.rcid);

                subfield_defn = field_defn->GetSubfield(1);
                if (!subfield_defn) return -1;
                fspt.ornt = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                subfield_defn = field_defn->GetSubfield(2);
                if (!subfield_defn) return -1;
                fspt.usag = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;
                
                subfield_defn = field_defn->GetSubfield(3);
                if (!subfield_defn) return -1;
                fspt.mask = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
                bytes_remaining -= bytes_consumed;
                field_data += bytes_consumed;

                fspts.push_back(fspt);
            }
        }
        else if (strncmp(field_name, "FSPC", 4) == 0)
		{
			subfield_defn = field_defn->GetSubfield(0);
			if (!subfield_defn) return -1;
            // fsui: Feature Spatial Unique Identifier → 对应的 Vector Record ID（VRID）或索引
			fspc.fsui = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
			bytes_remaining -= bytes_consumed;
			field_data += bytes_consumed;
            
			subfield_defn = field_defn->GetSubfield(1);
			if (!subfield_defn) return -1;
            // fsix: Feature Spatial Index → Feature Spatial Index → 在 VR 序列中的位置 / 指针
            fspc.fsix = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
			bytes_remaining -= bytes_consumed;
			field_data += bytes_consumed;

			subfield_defn = field_defn->GetSubfield(2);
			if (!subfield_defn) return -1;
            // nspt: Number of Spatial Points → 该要素的空间对象数目
			fspc.nspt = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
			bytes_remaining -= bytes_consumed;
			field_data += bytes_consumed;
        } 
		else if (strncmp(field_name, "FFPC", 4) == 0)
		{
			subfield_defn = field_defn->GetSubfield(0);
			if (!subfield_defn) return -1;
			ffpc.ffui = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
			bytes_remaining -= bytes_consumed;
			field_data += bytes_consumed;

			subfield_defn = field_defn->GetSubfield(1);
			if (!subfield_defn) return -1;
			ffpc.ffix = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
			bytes_remaining -= bytes_consumed;
			field_data += bytes_consumed;

			subfield_defn = field_defn->GetSubfield(2);
			if (!subfield_defn) return -1;
			ffpc.nfpt = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
			bytes_remaining -= bytes_consumed;
			field_data += bytes_consumed;
		}
		else 
		{
            std::cerr << "Feature::Load(): Unknown field " << field_name << std::endl;
            return -1;
        }
    }
    
    return frid.objl;
}
