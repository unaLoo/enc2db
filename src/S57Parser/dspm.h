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

#ifndef DSPM_H_
#define DSPM_H_

#include <string>
#include <iostream>
#include "iso8211.h"

/**
 * Class for loading/holding DSPM data
 */
class DSPM
{
    public:
        DSPM();
        ~DSPM();
        int Load(DDFRecord *record);
        int GetCoordMultFactor() { return coord_mult_factor; }
        int GetSoundingMultFactor() { return sounding_mult_factor; }

		//////////////////////////////////////////////////////////////////////////
		int _sizeFieldPos;
		int _sizeFieldLength;

		//////////////////////////////////////////////////////////////////////////
		int RCNM;										//! RCNM 记录名
		int RCID;										//! RCID 记录标识号
		int horz_gd_datum;						//! HDAT 水平测量基准面
        int vert_datum;								//! VDAT 垂直基准面
        int sounding_datum;						//! SDAT 测深基准面
        int comp_sod;								//! CSCL 数据编辑比例
        int depth_unit;								//! DUNI 深度计量单位
        int height_unit;								//! HUNI 高度计算单位
        int accuracy_unit;							//! PUNI 位置精度单位
		int coord_unit;								//! COUN 坐标单位
		int coord_mult_factor;					//! COMF 坐标放大系数
        int sounding_mult_factor;				//! SOMF 3-D (测深)放大系数
		std::string comment;						//! COMT 注释
};

#endif
