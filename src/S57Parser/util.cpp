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

#include "../compat.h"
#include "util.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <math.h>

#include "iso8211.h"

char GetHEXChar( const char *pszSrcHEXString )
{
	int nResult = 0;

	if( pszSrcHEXString[0] == '\0' || pszSrcHEXString[1] == '\0' )
		return (char) 0;

	if( pszSrcHEXString[0] >= '0' && pszSrcHEXString[0] <= '9' )
		nResult += (pszSrcHEXString[0] - '0') * 16;
	else if( pszSrcHEXString[0] >= 'a' && pszSrcHEXString[0] <= 'f' )
		nResult += (pszSrcHEXString[0] - 'a' + 10) * 16;
	else if( pszSrcHEXString[0] >= 'A' && pszSrcHEXString[0] <= 'F' )
		nResult += (pszSrcHEXString[0] - 'A' + 10) * 16;

	if( pszSrcHEXString[1] >= '0' && pszSrcHEXString[1] <= '9' )
		nResult += pszSrcHEXString[1] - '0';
	else if( pszSrcHEXString[1] >= 'a' && pszSrcHEXString[1] <= 'f' )
		nResult += pszSrcHEXString[1] - 'a' + 10;
	else if( pszSrcHEXString[1] >= 'A' && pszSrcHEXString[1] <= 'F' )
		nResult += pszSrcHEXString[1] - 'A' + 10;

	return (char) nResult;
}

std::string GetFormattedLongLatString(double minutes)
{
    int degrees = 0;
    double minute_flop;

    if ((minutes >= 60) || (minutes <= 60)) {
        degrees += int(minutes/60);
        minute_flop = minutes - int(minutes);
        minutes = (int)minutes % 60;
        minutes += minute_flop;
    }

    // do something about degrees.

    std::ostringstream strstr;

    if ((minutes < 0) || (degrees < 0)) {
        if (minutes == 0) {
            strstr << "- " << std::setfill('0') << std::setw(3) << -degrees
                   << " " << std::setiosflags(std::ios_base::left |
                                              std::ios_base::showpoint)
                   << std::setw(9) << minutes;
        } else {
            strstr << "- " << std::setfill('0') << std::setw(3) << -degrees
                   << " " << std::setiosflags(std::ios_base::left |
                                              std::ios_base::showpoint)
                   << std::setw(9) << -minutes;
        }
    } else {
        strstr << "+ " << std::setfill('0') << std::setw(3) << degrees
               << " "
               << std::setiosflags(std::ios_base::left |
                                   std::ios_base::showpoint)
               << std::setw(9) << minutes;
    }
    return strstr.str();
}

std::pair<std::string, std::string> GetLongLatStringPair(double minutes)
{
    int degrees = 0;
    double minute_flop;

    if ((minutes >= 60) || (minutes <= 60)) {
        degrees += int(minutes/60);
        minute_flop = minutes - int(minutes);
        minutes = (int)minutes % 60;
        minutes += minute_flop;
    }

    // do something about degrees.

    std::ostringstream degstrstr;
    std::ostringstream minstrstr;

    degstrstr << degrees;
    minstrstr << minutes;

    return std::make_pair(degstrstr.str(), minstrstr.str());
}

std::string decode_binary_string(unsigned char *bstr, int bcount)
{
    std::string str;
    char buf[2];
    for (int i = 0; i < MINIMUM(bcount, 24); i++) {
        sprintf_s(buf, "%02X", bstr[i]);
        str += buf;
    }
    return str;
}

void decode_lnam_string(unsigned char *bstr, int bcount, int *agen_find_fids)
{
    char buf[4];
    for (int i = 0; i < MINIMUM(bcount, 24); i++) {
        sprintf_s(buf, "%02X", bstr[i]);
    }

    agen_find_fids[0] = bstr[0] + (bstr[1] * 256);
    agen_find_fids[1] = bstr[2] + (bstr[3] * 256) +
                        (bstr[4] * 65536) + (bstr[5] * 16777216);
    agen_find_fids[2] = bstr[6] + (bstr[7] * 256);
}

char* encode_lnam_string(int agen, int find, int fids)
{
	char szLNAM[9];
	// AGEN
	szLNAM[1] = agen & 0xff;
	szLNAM[0] = (char) ((agen & 0xff00) >> 8);

	// FIDN
	szLNAM[5] = find & 0xff;
	szLNAM[4] = (char) ((find & 0xff00) >> 8);
	szLNAM[3] = (char) ((find & 0xff0000) >> 16);
	szLNAM[2] = (char) ((find & 0xff000000) >> 24);

	// FIDS
	szLNAM[7] = fids & 0xff;
	szLNAM[6] = (char) ((fids & 0xff00) >> 8);

	szLNAM[8] = '\0';

	return szLNAM;
}

void decode_name_string(unsigned char *bstr, int bcount, int *rcnm_rcid)
{
    char buf[4];
    for (int i = 0; i < MINIMUM(bcount, 24); i++) {
        sprintf_s(buf, "%02X", bstr[i]);
    }

    rcnm_rcid[0] = bstr[0];
    rcnm_rcid[1] = bstr[1] + (bstr[2] * 256) +
        (bstr[3]*65536) + (bstr[4] * 16777216);
}

unsigned char* encode_name_string(int rcnm, int rcid)
{
	unsigned char ret[5];
	ret[0] = (unsigned char)rcnm;
	ret[1] = (unsigned char)(rcid & 0xff);
	ret[2] = (unsigned char) ((rcid & 0xff00) >> 8);
	ret[3] = (unsigned char) ((rcid & 0xff0000) >> 16);
	ret[4] = (unsigned char) ((rcid & 0xff000000) >> 24);
	return ret;
}

std::string remove_whitespace(std::string str)
{
    while (isspace(str[str.length()-1]))
        str.erase(str.length()-1);

    while (isspace(*(str.begin())))
        str.erase(str.begin());

    return str;
}


int hex_to_dec(std::string str)
{
    const char *hexStg = str.c_str();
    int n = 0;         // position in string
    int m = 0;         // position in digit[] to shift
    int count;         // loop index
    int intValue = 0;  // integer value of hex string
    int digit[5];      // hold values to convert
    while (n < 4) {
     if (hexStg[n]=='\0')
        break;
     if (hexStg[n] > 0x29 && hexStg[n] < 0x40 ) //if 0 to 9
        digit[n] = hexStg[n] & 0x0f;            //convert to int
     else if (hexStg[n] >='a' && hexStg[n] <= 'f') //if a to f
        digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
     else if (hexStg[n] >='A' && hexStg[n] <= 'F') //if A to F
        digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
     else break;
    n++;
    }
    count = n;
    m = n - 1;
    n = 0;
    while(n < count) {
     // digit[n] is value of hex digit at position n
     // (m << 2) is the number of positions to shift
     // OR the bits into return value
     intValue = intValue | (digit[n] << (m << 2));
     m--;   // adjust the position to set
     n++;   // next digit to process
    }
    return (intValue);
}

bool is_catalog(const char *filename)
{
    DDFModule ddf;
    if (ddf.Open(filename) == 0) { // DDFModule returns 0 on fail
        ddf.Close();
        return false;
    }
    DDFRecord *record;

    while ((record = ddf.ReadRecord()) != NULL) {
        int field_count = record->GetFieldCount();
        if (field_count != 2) {
            ddf.Close();
            return false;
        }
        DDFField *field = record->GetField(1);
        if (!field) {
            ddf.Close();
            return false;
        }

        const char *field_name;
        DDFFieldDefn *field_defn;

        field_defn = field->GetFieldDefn();
        field_name = field_defn->GetName();

        if (strncmp(field_name, "CATD", 4) != 0) {
            ddf.Close();
            return false;
        }

        if (field_defn->GetSubfieldCount() != 12) {
            ddf.Close();
            return false;
        }
    }

    ddf.Close();
    return true;
}

bool is_chart(const char *filename)
{
    DDFModule ddf;
    if (ddf.Open(filename) == 0) { // DDFModule returns 0 on fail
        ddf.Close();
        return false;
    }
    DDFRecord *record;

    record = ddf.ReadRecord();
    if (!record) {
        ddf.Close();
        return false;
    }

    int field_count = record->GetFieldCount();
    if (field_count != 3) {
        ddf.Close();
        return false;
    }
    DDFField *field = record->GetField(1);
    if (!field) {
        ddf.Close();
        return false;
    }

    const char *field_name;
    DDFFieldDefn *field_defn;

    field_defn = field->GetFieldDefn();
    field_name = field_defn->GetName();

    if (!field_name) {
        ddf.Close();
        return false;
    }

    if (!field_defn) {
        ddf.Close();
        return false;
    }

    if (strncmp(field_name, "DSID", 4) != 0) {
        ddf.Close();
        return false;
    }

    if (field_defn->GetSubfieldCount() != 16) {
        ddf.Close();
        return false;
    } 

    ddf.Close();
    return true;
}

/**
 * Given 3 points calculate the angle between the created lines.
 */
float calc_angle(double *a, double *b, double *c)
{
    double A[2];
    A[0] = a[0] - b[0];
    A[1] = a[1] - b[1];

    double B[2];
    B[0] = b[0] - c[0];
    B[1] = b[1] - c[1];

    double dotproduct = A[0] * B[0] + A[1] * B[1];
    double lengths[2];
    lengths[0] = sqrt(A[0] * A[0] + A[1] * A[1]);
    lengths[1] = sqrt(B[0] * B[0] + B[1] * B[1]);

    if (lengths[0] * lengths[1] == 0)
        return 0;

    double tmp = dotproduct / (lengths[0] * lengths[1]);
    return PI - acos(tmp);
}

float calc_length(double *a, double *b)
{
    double tmp0 = a[0] - b[0];
    double tmp1 = a[1] - b[1];
    return sqrt(tmp0 * tmp0 + tmp1 * tmp1);
}

//boost::filesystem::path decompose_win32_path(std::string &input)
//{
//    std::vector<std::string> elements; 
//    std::string::size_type i;
//    std::string::size_type last_valid_i = 0;
//    bool found = false;
//    while ((i = input.find("\\")) != std::string::npos) {
//        elements.push_back(input.substr(0, i));
//        last_valid_i = i;
//        found = true;
//        input.replace(i, 1, "/");
//    }
//    if (found) {
//        elements.push_back(input.substr(last_valid_i+1));
//    }
//
//    boost::filesystem::path output;
//    std::vector<std::string>::const_iterator it;
//    std::vector<std::string>::const_iterator end(elements.end());
//    for (it = elements.begin(); it != end; ++it) {
//        output /= *it;
//    } 
//
//    return output;
//
//}
