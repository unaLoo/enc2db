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

#include "dsid.h"
#include "util.h"
#include "iso8211.h"

/**
 * Constructor
 */
DSID::DSID() :
    PRSP(0), producing_agency(0), data_struct(0), attf_lexl(0),
    natf_lexl(0), num_meta_rec(0), num_cart_rec(0), num_geo_rec(0),
    num_coll_rec(0), num_isonode_rec(0), num_connnode_rec(0),
    num_edge_rec(0), num_face_rec(0)
{
}

/**
 * Destructor
 */
DSID::~DSID()
{
}

/** Load dsid data from a record
 * @param record Record to read
 * @return 0 on success, -1 on failure.
 */
int DSID::Load(DDFRecord *record)
{
    int bytes_remaining;
    int bytes_consumed;
    const char *field_data;
    const char *field_name;
    DDFField *field;
    DDFFieldDefn *field_defn;
    DDFSubfieldDefn *subfield_defn;

    int field_count = record->GetFieldCount();
    if (field_count != 3) {
        std::cerr << "DSID::LoadDSIDRecord(): bad field_count" << std::endl;
        return -1;
    }

	_sizeFieldPos = record->getFieldPos();
	_sizeFieldLength = record->getFieldLength();

	//////////////////////////////////////////////////////////////////////////
	//
	// Read the DSID field and associated subfields
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

    //! RCNM: record name 记录名
    subfield_defn = field_defn->GetSubfield(0);
    if (!subfield_defn) return -1;
    record_name = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! RCID: record Identifier 记录标识号
	subfield_defn = field_defn->GetSubfield(1);
	if (!subfield_defn) return -1;
	RCID = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

	//! EXPP: 交换用途
	subfield_defn = field_defn->GetSubfield(2);
	if (!subfield_defn) return -1;
	EXPP = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

	//! INTU: 预期用途
	subfield_defn = field_defn->GetSubfield(3);
	if (!subfield_defn) return -1;
	INTU = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

	//! DSNM: dataset name 数据集名
    subfield_defn = field_defn->GetSubfield(4);
    if (!subfield_defn) return -1;
    dataset_name = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! EDTN: edition number 版本号
    subfield_defn = field_defn->GetSubfield(5);
    if (!subfield_defn) return -1;
    edition_num = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! UPDN: update num 更新号
	subfield_defn = field_defn->GetSubfield(6);
	if (!subfield_defn) return -1;
	update_num = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

	//! UADT: update app date 更新应用日期
	subfield_defn = field_defn->GetSubfield(7);
	if (!subfield_defn) return -1;
	update_app_date = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

	//! ISDT: issue date 出版日期
    subfield_defn = field_defn->GetSubfield(8);
    if (!subfield_defn) return -1;
    issue_date = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! STED:   《数据传输》版本号
    subfield_defn = field_defn->GetSubfield(9);
    if (!subfield_defn) return -1;
    STED = subfield_defn->ExtractFloatData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

    //! PRSP: product spec 产品规范
    subfield_defn = field_defn->GetSubfield(10);
    if (!subfield_defn) return -1;
    PRSP = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! PSDN: product spec descr 产品规范描述
	subfield_defn = field_defn->GetSubfield(11);
	if (!subfield_defn) return -1;
	PSDN = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

	//! PRED: product spec 
	subfield_defn = field_defn->GetSubfield(12);
	if (!subfield_defn) return -1;
	PRED = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;

    //! PROF: app profile id 应用简档标识
    subfield_defn = field_defn->GetSubfield(13);
    if (!subfield_defn) return -1;
    PROF = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! AGEN: producing agency 应用简档标识
    subfield_defn = field_defn->GetSubfield(14);
    if (!subfield_defn) return -1;
    producing_agency = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
    bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! COMT: dsid comment 注释
	subfield_defn = field_defn->GetSubfield(15);
	if (!subfield_defn) return -1;
	comment = subfield_defn->ExtractStringData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
	field_data += bytes_consumed;
 
	//////////////////////////////////////////////////////////////////////////
	//
	// Now read the dssi record
	//
	//////////////////////////////////////////////////////////////////////////
    field = record->GetField(2);
    field_defn = field->GetFieldDefn();
    field_name = field_defn->GetName();
    field_data = field->GetData();
    bytes_remaining = field->GetDataSize();
	std::string field_name_str = field_name;
    if (field_name_str != "DSSI") {
        std::cerr << "DSID::LoadDSIDRecord(): Invalid DSSI field name"<< std::endl;
        return -1;
    }
	
	//! DSTR  数据结构
	subfield_defn = field_defn->GetSubfield(0);
	if (!subfield_defn) return -1;
	data_struct = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! AALL  ATTF词汇级
	subfield_defn = field_defn->GetSubfield(1);
	if (!subfield_defn) return -1;
	attf_lexl = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! NALL  NATF词汇级
	subfield_defn = field_defn->GetSubfield(2);
	if (!subfield_defn) return -1;
	natf_lexl = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! NOMR  元记录数
	subfield_defn = field_defn->GetSubfield(3);
	if (!subfield_defn) return -1;
	num_meta_rec = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! NOCR  制图记录数
	subfield_defn = field_defn->GetSubfield(4);
	if (!subfield_defn) return -1;
	num_cart_rec = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! NOGR  地理记录数
	subfield_defn = field_defn->GetSubfield(5);
	if (!subfield_defn) return -1;
	num_geo_rec = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! NOLR  集合记录数
	subfield_defn = field_defn->GetSubfield(6);
	if (!subfield_defn) return -1;
	num_coll_rec = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! NOIN  孤立节点数
	subfield_defn = field_defn->GetSubfield(7);
	if (!subfield_defn) return -1;
	num_isonode_rec = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! NOCN  连接节点数
	subfield_defn = field_defn->GetSubfield(8);
	if (!subfield_defn) return -1;
	num_connnode_rec = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! NOED  边线数
	subfield_defn = field_defn->GetSubfield(9);
	if (!subfield_defn) return -1;
	num_edge_rec = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

	//! NOFA  面数
	subfield_defn = field_defn->GetSubfield(10);
	if (!subfield_defn) return -1;
	num_face_rec = subfield_defn->ExtractIntData(field_data, bytes_remaining, &bytes_consumed);
	bytes_remaining -= bytes_consumed;
    field_data += bytes_consumed;

/**/
    return 0;
}
