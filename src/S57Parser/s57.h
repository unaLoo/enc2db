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

#ifndef S57_H_
#define S57_H_

#define SLAT 0
#define WLON 1
#define NLAT 2
#define ELON 3

/**
 * waypoint
 */
struct waypoint_t
{
    std::string name;
    double longlat[2];
};

//
// feature draw properties
//

/**
 * Point object drawing properties
 */
struct point_drawprop_t
{
    float rgb[3];
    float size;
};

/**
 * Line object drawing properties
 */
struct line_drawprop_t
{
    float rgb[3];
    float size;
};

/**
 * Area object drawing properties
 */
struct area_drawprop_t
{
    float rgb[3];
};

//
// feature record stuff
//
struct frid_t
{
	int rcnm;
	int rcid;
	int prim;
	int grup;
	int objl;
	int rver;
	int ruin; // 记录的状态或更新类型。 1 - Insert/New，2 - update，3 - Delete
};

struct foid_t
{
	int agen;
	int find;
	int fids;
};

/**
 * attf
 */
struct attf_t
{
    int attl;
    std::string atvl;
};

/**
 * ffpt
 */
struct ffpt_t
{
    int agen;
    int find;
    int fids;
    int rind;
    std::string comt;
};

/**
 * fspt
 */
struct fspt_t
{
    int rcnm;
    int rcid;
    int ornt;
    int usag;
    int mask;
};

struct fspc_t
{
	int fsui;
	int fsix;
	int nspt;
};

struct ffpc_t
{
	int ffui;
	int ffix;
	int nfpt;
};

//
// vector record stuff
//
struct vrid_t
{
	int rcnm;
	int rcid;
	int rver;
	int ruin;
};

/**
 * attv
 */
struct attv_t
{
    int attl;
    std::string atvl;
};

/**
 * vrpc
 */
struct vrpc_t
{
    int vpui;
    int vpix;
    int nvpt;
};

/**
 * vrpt
 */
struct vrpt_t
{
    int rcnm;
    int rcid;
    int ornt;
    int usag;
    int topi;
    int mask;
};

/**
 * sg2d
 */
struct sg2d_t
{
    double long_lat[2];
	bool save_flag;
	bool is_mid_point;
	int conn_idx;

	sg2d_t()
	{
		long_lat[0] = 0.0;
		long_lat[1] = 0.0;
		save_flag = true;
		is_mid_point = false;
		conn_idx = -1;
	}
};

/**
 * sg3d
 */
struct sg3d_t
{
    double long_lat[2];
	double depth;
	bool save_flag;
};

enum object_classes_e {
    // geo object classes
    ADMARE = 1,
    AIRARE, ACHBRT, ACHARE, BCNCAR, BCNISD, BCNLAT, BCNSAW, BCNSPP,
    BERTHS, BRIDGE, BUISGL, BUAARE, BOYCAR, BOYINB, BOYISD, BOYLAT,
    BOYSAW, BOYSPP, CBLARE, CBLOHD, CBLSUB, CANALS, CANBNK, CTSARE,
    CAUSWY, CTNARE, CHKPNT, CGUSTA, COALNE, CONZNE, COSARE, CTRPNT,
    CONVYR, CRANES, CURENT, CUSZNE, DAMCON, DAYMAR, DWRTCL, DWRTPT,
    DEPARE, DEPCNT, DISMAR, DOCARE, DRGARE, DRYDOC, DMPGRD, DYKCON,
    EXEZNE, FAIRWY, FNCLNE, FERYRT, FSHZNE, FSHFAC, FSHGRD, FLODOC,
    FOGSIG, FORSTC, FRPARE, GATCON, GRIDRN, HRBARE, HRBFAC, HULKES,
    ICEARE, ICNARE, ISTZNE, LAKARE, LAKSHR, LNDARE, LNDELV, LNDRGN,
    LNDMRK, LIGHTS, LITFLT, LITVES, LOCMAG, LOKBSN, LOGPON, MAGVAR,
    MARCUL, MIPARE, MORFAC, NAVLNE, OBSTRN, OFSPLF, OSPARE, OILBAR,
    PILPNT, PILBOP, PIPARE, PIPOHD, PIPSOL, PONTON, PRCARE, PRDARE,
    PYLONS, RADLNE, RADRNG, RADRFL, RADSTA, RTPBCN, RDOCAL, RDOSTA,
    RAILWY, RAPIDS, RCRTCL, RECTRC, RCTLPT, RSCSTA, RESARE, RETRFL,
    RIVERS, RIVBNK, ROADWY, RUNWAY, SNDWAV, SEAARE, SPLARE, SBDARE,
    SLCONS, SISTAT, SISTAW, SILTNK, SLOTOP, SLOGRD, SMCFAC, SOUNDG,
    SPRING, SQUARE, STSLNE, SUBTLN, SWPARE, TESARE,
    TS_PRH, TS_PNH, TS_PAD, TS_TIS, T_HMON, T_NHMN, T_TIMS, TIDEWY,
    TOPMAR, TSELNE, TSSBND, TSSCRS, TSSLPT, TSSRON, TSEZNE, TUNNEL,
    TWRTPT, UWTROC, UNSARE, VEGATN, WATTUR, WATFAL, WEDKLP, WRECKS,
    TS_FEB,
    // meta object classes
    M_ACCY = 300, M_CSCL, M_COVR, M_HDAT, M_HOPA, M_NPUB, M_NSYS,
    M_PROD, M_QUAL, M_SDAT, M_SREL, M_UNIT, M_VDAT,
    // collection object classes
    C_AGGR = 400, C_ASSO, C_STAC,
    // cartographic object classes
    _AREAS = 500, _LINES, _CSYMB, _COMPS, _TEXTS
};

#endif
