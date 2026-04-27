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

#ifndef UTIL_H_
#define UTIL_H_

#define MINIMUM(a,b)      ((a<b) ? a : b)
#define PI 3.14159265359

#include <string>
#include <vector>


std::string GetFormattedLongLatString(double minutes);
std::pair<std::string, std::string> GetLongLatStringPair(double minutes);
std::string decode_binary_string(unsigned char *bstr, int bcount);
void decode_lnam_string(unsigned char *bstr, int bcount, int *agen_find_fids);
void decode_name_string(unsigned char *bstr, int bcount, int *rcnm_rcid);
std::string remove_whitespace(std::string str);
bool is_catalog(const char *filename);
bool is_chart(const char *filename);
float calc_angle(double *a, double *b, double *c);
float calc_length(double *a, double *b);
//boost::filesystem::path decompose_win32_path(std::string &input);

char* encode_lnam_string(int agen, int find, int fids);
unsigned char* encode_name_string(int rcnm, int rcid);

#endif
