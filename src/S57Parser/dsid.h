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

#ifndef DSID_H_
#define DSID_H_

#include <string>
#include <iostream>
#include "iso8211.h"

/**
 * Class for loading/holding DSID data
 */
class DSID
{
    public:
        DSID();
        ~DSID();
        int Load(DDFRecord *record);
        int GetNumConnNodeRec() { return num_connnode_rec; }
        int GetNumEdgeRec() { return num_edge_rec; }

		//////////////////////////////////////////////////////////////////////////
		int _sizeFieldPos;
		int _sizeFieldLength;
    
		//////////////////////////////////////////////////////////////////////////
        // from dsid
		int record_name;						//! RCNM: record name 记录名
		int RCID;									//! RCID: record Identifier 记录标识号
		int EXPP;									//! EXPP: 交换用途
		int INTU;									//! INTU: 预期用途
        std::string dataset_name;			//! DSNM: dataset name 数据集名
        std::string edition_num;				//! EDTN: edition number 版本号
		std::string update_num;				//! UPDN: update num 更新号
		std::string update_app_date;		//! UADT: update app date 更新应用日期
        std::string issue_date;				//! ISDT: issue date 出版日期
		double STED;							//! STED:   《数据传输》版本号
        int PRSP;									//! PRSP: product spec 产品规范
		std::string PSDN;						//! PSDN: product spec descr 产品规范描述
		std::string PRED;						//! PRED: product spec 
		int PROF;									//! PROF: app profile id 应用简档标识
        int producing_agency;				//! AGEN: producing agency 应用简档标识
		std::string comment;					//! COMT: dsid comment 注释

		//////////////////////////////////////////////////////////////////////////
        // from dssi
        int data_struct;							//! DSTR  数据结构
        int attf_lexl;								//! AALL  ATTF词汇级
        int natf_lexl;								//! NALL  NATF词汇级
        int num_meta_rec;					//! NOMR  元记录数
        int num_cart_rec;						//! NOCR  制图记录数
        int num_geo_rec;						//! NOGR  地理记录数
        int num_coll_rec;						//! NOLR  集合记录数
        int num_isonode_rec;				//! NOIN  孤立节点数
        int num_connnode_rec;				//! NOCN  连接节点数
        int num_edge_rec;						//! NOED  边线数
        int num_face_rec;						//! NOFA  面数
};

#endif
