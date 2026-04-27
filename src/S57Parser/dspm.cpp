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

#include "dspm.h"
#include "util.h"
#include "iso8211.h"

/**
 * Constructor
 */
DSPM::DSPM() :
    coord_mult_factor(0), horz_gd_datum(0), vert_datum(0), sounding_datum(0),
    comp_sod(0), depth_unit(0), height_unit(0), accuracy_unit(0),
    coord_unit(0), sounding_mult_factor(0)
{
}

/**
 * Destructor
 */
DSPM::~DSPM()
{
}

/** Load dspm data from a record
 * @param record Record to read
 * @return 0 on success, -1 on failure.
 */
int DSPM::Load(DDFRecord *record)
{
    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

	_sizeFieldPos = record->getFieldPos();
	_sizeFieldLength = record->getFieldLength();

	//////////////////////////////////////////////////////////////////////////
	//
    // Read the DSPM field and associated subfields
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

	//! RCNM 记录名
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn) return -1;
    RCNM = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    if (RCNM != 20) {
        std::cerr << "DSPM::LoadDSPMRecord(): bad rcnm" << std::endl;
        return -1;
    }

    //! RCID 记录标识号
    subfield_defn = field_defn->GetSubfield(1);
    if (!subfield_defn) return -1;
    RCID = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! HDAT 水平测量基准面
    subfield_defn = field_defn->GetSubfield(2);
    if (!subfield_defn) return -1;
    horz_gd_datum = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! VDAT 垂直基准面
    subfield_defn = field_defn->GetSubfield(3);
    if (!subfield_defn) return -1;
    vert_datum = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! SDAT 测深基准面
    subfield_defn = field_defn->GetSubfield(4);
    if (!subfield_defn) return -1;
    sounding_datum = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! CSCL 数据编辑比例
    subfield_defn = field_defn->GetSubfield(5);
    if (!subfield_defn) return -1;
    comp_sod = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! DUNI 深度计量单位
    subfield_defn = field_defn->GetSubfield(6);
    if (!subfield_defn) return -1;
    depth_unit = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! HUNI 高度计算单位
    subfield_defn = field_defn->GetSubfield(7);
    if (!subfield_defn) return -1;
    height_unit = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! PUNI 位置精度单位
    subfield_defn = field_defn->GetSubfield(8);
    if (!subfield_defn) return -1;
    accuracy_unit = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! COUN 坐标单位
    subfield_defn = field_defn->GetSubfield(9);
    if (!subfield_defn) return -1;
    coord_unit = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! COMF 坐标放大系数
    subfield_defn = field_defn->GetSubfield(10);
    if (!subfield_defn) return -1;
    coord_mult_factor = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! SOMF 3-D (测深)放大系数
    subfield_defn = field_defn->GetSubfield(11);
    if (!subfield_defn) return -1;
    sounding_mult_factor = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! COMT 注释
	subfield_defn = field_defn->GetSubfield(12);
	if (!subfield_defn) return -1;
	comment = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

    // TODO There is more stuff in the DSPM record!

    return 0;
}
