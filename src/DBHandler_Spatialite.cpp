#include "DBHandler_Spatialite.h"
#include "compat.h"

DBHandler_Spatialite::DBHandler_Spatialite()
{
	m_chart_db_uid = -1;
	m_chart_tables_initialized = false; 
}

DBHandler_Spatialite::~DBHandler_Spatialite() {}

//! 准备好空间数据库
void DBHandler_Spatialite::prepare_spatial_database(const char* saveDatabaseName)
{
    int ret = sqlite3_open_v2(saveDatabaseName, &handle,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

    cache = spatialite_alloc_connection();
    spatialite_init_ex(handle, cache, 0);

    printf("SQLite version: %s\n", sqlite3_libversion());
    printf("SpatiaLite version: %s\n", spatialite_version());

    //! 首先确实是否已经初始化Spatial模式了
    std::string sql_check_init = "SELECT CheckSpatialMetaData()";
    ret = sqlite3_prepare_v2(handle, sql_check_init.c_str(), sql_check_init.size(), &stmt, NULL);
    int n_columns = sqlite3_column_count(stmt);
    ret = sqlite3_step(stmt);
    int val = sqlite3_column_int(stmt, 0);
    if (val <= 0) //! 没有初始化Spatial扩展就是执行初始化
    {
        std::string sql_init = "SELECT InitSpatialMetadata(1)";
        char* err_msg = NULL;
        ret = sqlite3_exec(handle, sql_init.c_str(), NULL, NULL, &err_msg);
        if (ret != SQLITE_OK) printf("InitSpatialMetadata SQL error: %s\n", sqlite3_errmsg(handle));
    }
}

//! 关联一个海图图幅
void DBHandler_Spatialite::bind_chart_loader(MyChartLoader* loader)
{
    ////////////////////////////////// 建图幅表<CHART_FILE_NAME> ///////////////////////////
    m_chart = loader;

    char* err_msg = NULL;
    std::string table_name = "CHART_FILE_NAME"; 
    //! 如果没有对应的表，就创建表，并以PK_UID作为主键
    std::string sql_create_table =
        "CREATE TABLE IF NOT EXISTS " + table_name +
        " (CHART_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
        " CHART_NAME TEXT," + 
        " CHART_SCALE INTEGER, " +
        " CHART_LEVEL INTEGER)";
    int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
    
    std::string sql_select_chart =
        "SELECT count(*) FROM " + table_name + " WHERE CHART_NAME = '" + loader->chart_file_name + "'";
    ret = sqlite3_prepare_v2(handle, sql_select_chart.c_str(), sql_select_chart.size(), &stmt, NULL);
    if (ret != SQLITE_OK) printf("sql_check_geom_column SQL error: %s\n", sqlite3_errmsg(handle));
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int val = sqlite3_column_int(stmt, 0);
        if (val == 0)
        {
            std::string sql_insert_item = "INSERT INTO " + table_name +
                " (CHART_NAME, CHART_SCALE, CHART_LEVEL) VALUES (?, ?, ?)";
            ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);

            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            sqlite3_bind_text(stmt, 1, loader->chart_file_name.c_str(), loader->chart_file_name.size(), SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 2, loader->dspm.comp_sod);
            sqlite3_bind_int64(stmt, 3, loader->m_cscl_level);

            ret = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    std::string sql_select_chart_id =
        "SELECT CHART_UID FROM " + table_name + " WHERE CHART_NAME = '" + loader->chart_file_name + "'";
    ret = sqlite3_prepare_v2(handle, sql_select_chart_id.c_str(), sql_select_chart_id.size(), &stmt, NULL);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        m_chart_db_uid = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }
    char id_str[50];
    sprintf_s(id_str, "%d", m_chart_db_uid);
    m_chart_db_uid_str = id_str;
}

//! 判断一个表的几何字段、空间索引是否已经准备好
void DBHandler_Spatialite::check_table_geom_index_prepared(const char* table_name, const char* geom_type)
{
    std::string table_name_str = table_name;
    int ret = -1;
    char* err_msg = NULL;

    //! 判断geom几何字段是否已经创建
    std::string geom_type_str = geom_type;
    std::string sql_check_geom_column =
        "select count(*) from sqlite_master where name = '" + table_name_str + "' and sql LIKE '%Geometry%'";
    ret = sqlite3_prepare_v2(handle, sql_check_geom_column.c_str(), sql_check_geom_column.size(), &stmt, NULL);
    if (ret != SQLITE_OK) printf("sql_check_geom_column SQL error: %s\n", sqlite3_errmsg(handle));
    if (sqlite3_step(stmt) == SQLITE_ROW) //! 没有找到geom几何字段，添加几何字段
    {
        int val = sqlite3_column_int(stmt, 0);
        if (val == 0)
        {
            int loc = table_name_str.find("POINT_SOUNDG");
            if (loc <0)
            {
                std::string sql_add_gem_column =
                    "SELECT AddGeometryColumn('" + table_name_str + "', 'Geometry', 4326, '" + geom_type_str + "', 2)";
                ret = sqlite3_exec(handle, sql_add_gem_column.c_str(), NULL, NULL, &err_msg);
            }
            else
            {
                std::string sql_add_gem_column =
                    "SELECT AddGeometryColumn('" + table_name_str + "', 'Geometry', 4326, '" + geom_type_str + "', 3)";
                ret = sqlite3_exec(handle, sql_add_gem_column.c_str(), NULL, NULL, &err_msg);
            }
        }
    }

    //! 判断SpatialIndex是否已经创建
    std::string sql_check_spatialindex =
        "select count(*) from sqlite_master where name = 'idx_" + table_name_str + "_Geometry'";
    ret = sqlite3_prepare_v2(handle, sql_check_spatialindex.c_str(), sql_check_spatialindex.size(), &stmt, NULL);
    if (ret != SQLITE_OK) printf("sql_check_spatialindex SQL error: %s\n", sqlite3_errmsg(handle));
    if (sqlite3_step(stmt) == SQLITE_ROW) //! 没有找到SpatialIndex表，添加
    {
        int val = sqlite3_column_int(stmt, 0);
        if (val == 0)
        {
            std::string sql_create_sindex =
                "SELECT CreateSpatialIndex('" + table_name_str + "', 'Geometry')";
            ret = sqlite3_exec(handle, sql_create_sindex.c_str(), NULL, NULL, &err_msg);
        }
    }
}

void DBHandler_Spatialite::prepare_chart_tables()
{
    if (m_chart_tables_initialized) return;
    char* err_msg = NULL;
    std::vector<std::string> level_char;
	//level_char.push_back("4"); 
	//level_char.push_back("5");
	//level_char.push_back("6");
	//level_char.push_back("7");
	//level_char.push_back("8");
	//level_char.push_back("9");
	level_char.push_back("10");
	//level_char.push_back("11");
	//level_char.push_back("12");
    for (int i=0; i< level_char.size(); i++)
    {
        printf("init chart level tables: %s\n", level_char[i].c_str());
        std::string level_str = level_char[i];
        {
            std::string table_name = "AREA_COMMON_A_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " OBJNAM TEXT," +
                " NOBJNM TEXT, " +
                " FillType INTEGER)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "POLYGON");
        }
        {
            std::string table_name = "AREA_COMMON_L_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " OBJNAM TEXT," +
                " NOBJNM TEXT," +
                " LineType INTEGER)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "LINESTRING");
        }
        {
            std::string table_name = "AREA_COMMON_M_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " OBJNAM TEXT," +
                " NOBJNM TEXT," +
                " MarkerName1 TEXT, " +
                " MarkerName2 TEXT, " +
                " ORIENT DOUBLE)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "POINT");
        }
        {
            std::string table_name = "LINE_COMMON_ML_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " OBJNAM TEXT," +
                " NOBJNM TEXT," +
                " CATEGORY INTEGER)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "MULTILINESTRING");
        }
        {
            std::string table_name = "POINT_COMMON_ICON_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " OBJNAM TEXT," +
                " NOBJNM TEXT," +
                " CATEGORY INTEGER," +
                " CATEGORY_S INTEGER," +
                " VAL1 DOUBLE," +
                " VAL2 DOUBLE)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "POINT");
        }
        {
            std::string table_name = "POINT_SOUNDG_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " OBJNAM TEXT," +
                " NOBJNM TEXT)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "POINTZ");
        }
        {
            std::string table_name = "AREA_DEPARE_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " DRVAL1 DOUBLE," +
                " DRVAL2 DOUBLE)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "POLYGON");
        }
    //    {
    //        std::string table_name = "AREA_DEPARE_TILED_" + level_str;
    //        //! 如果没有对应的表，就创建表，并以PK_UID作为主键
    //        std::string sql_create_table =
    //            "CREATE TABLE IF NOT EXISTS " + table_name +
    //            " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
    //            " CHARTID INTEGER,"
    //            " RCID INTEGER," +
    //            " OBJL INTEGER," +
    //            " DRVAL1 DOUBLE," +
    //            " DRVAL2 DOUBLE," +
				//" TILE_LEVEL INTEGER," +
				//" TILE_X INTEGER," +
				//" TILE_Y INTEGER)";
    //        int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
    //        check_table_geom_index_prepared(table_name.c_str(), "POLYGON");
    //    }
        {
            std::string table_name = "AREA_LNDARE_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " OBJNAM TEXT," +
                " NOBJNM TEXT)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "POLYGON");
        }
    //    {
    //        std::string table_name = "AREA_LNDARE_TILED_" + level_str;
    //        //! 如果没有对应的表，就创建表，并以PK_UID作为主键
    //        std::string sql_create_table =
    //            "CREATE TABLE IF NOT EXISTS " + table_name +
    //            " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
    //            " CHARTID INTEGER,"
    //            " RCID INTEGER," +
    //            " OBJL INTEGER," +
    //            " OBJNAM TEXT," +
				//" NOBJNM TEXT," +
				//" TILE_LEVEL INTEGER," +
				//" TILE_X INTEGER," +
				//" TILE_Y INTEGER)";
    //        int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
    //        check_table_geom_index_prepared(table_name.c_str(), "POLYGON");
    //    }
        {
            std::string table_name = "AREA_DRGARE_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " DRVAL1 DOUBLE," +
                " DRVAL2 DOUBLE)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "POLYGON");
        }
        {
            std::string table_name = "AREA_OBSTRN_" + level_str;
            //! 如果没有对应的表，就创建表，并以PK_UID作为主键
            std::string sql_create_table =
                "CREATE TABLE IF NOT EXISTS " + table_name +
                " (PK_UID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," +
                " CHARTID INTEGER,"
                " RCID INTEGER," +
                " OBJL INTEGER," +
                " OBJNAM TEXT," +
                " NOBJNM TEXT," +
                " FILLTYPE INTEGER)";
            int ret = sqlite3_exec(handle, sql_create_table.c_str(), NULL, NULL, &err_msg);
            check_table_geom_index_prepared(table_name.c_str(), "POLYGON");
        }
    }
    m_chart_tables_initialized = true;
}

//! 判断一个海图图幅中的Record是否已经在某个表了，根据RCID判断
bool DBHandler_Spatialite::check_record_exist(const char* table_name, int rcid)
{
    char rcid_str[50];
    std::string table_name_str = table_name;
    std::string sql_check_record_exist = "SELECT count(*) FROM " + table_name_str +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ";

    sprintf_s(rcid_str, "%d", rcid);
    std::string temp_check_str = sql_check_record_exist + rcid_str;
    int ret = sqlite3_prepare_v2(handle, temp_check_str.c_str(), temp_check_str.size(), &stmt, NULL);
    if (ret != SQLITE_OK) printf("temp_check_str SQL error: %s\n", sqlite3_errmsg(handle));
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int val = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (val >= 1) return true;
        else return false;
    }
    return false;
}

//! 处理海图面图层-可视化效果为面
void DBHandler_Spatialite::process_area_common_a()
{
    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "AREA_COMMON_A_" + table_name_level;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, OBJNAM, NOBJNM, FillType, Geometry) \
        SELECT ?, ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        std::map<int, std::vector<AreaFeature> >::iterator areaIter;
        std::map<int, std::vector<AreaFeature> >::iterator areaEnd = m_chart->area_features_map.end();

        // area_features_map --> Map<OBJL, Array of Feature>  
        // 遍历每一个 area feature 对象类
        for (areaIter = m_chart->area_features_map.begin(); areaIter != areaEnd; ++areaIter) 
        {   
            // Map<OBJL, Array of Feature>  
            int featureId = areaIter->first; // 其实就是 OBJL
            std::vector<AreaFeature>& temp_area_vec = areaIter->second;  // 这里就是 Array of Feature
            std::string shpName = m_chart->code_name_system[featureId];  // 这里获取了 OBJL 对应的 Acronym 缩写名

            // 这里过滤了一些特殊图层
			if (filter_area_fill(shpName.c_str()) == false)
            {
                continue;
            }
            // 下面的都是普通 area_fill 类的图层

            int blob_size;
            unsigned char* blob;
            // 遍历这个类别下的所有 feature
            for (int iA = 0; iA < temp_area_vec.size(); iA++)
            {
                // ruined == 2 canceled ， 直接跳过
                if (temp_area_vec[iA].frid.ruin == 2) continue;

                // 矢量处理，用到了一个和 Spatialite 配套的 gaia 库， 转为适配 Spatialite 的几何对象
                gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
                geo_pg->Srid = 4326;
                area_to_spatialite_polygon(temp_area_vec[iA], geo_pg);
                
                gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
                gaiaFreeGeomColl(geo_pg);

                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);
                
                // 这里只存了几个特别的属性，没有把全部属性给存下
                // CHARTID, RCID, OBJL, OBJNAM, NOBJNM, FillType, Geometry
                sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
                sqlite3_bind_int64(stmt, 2, temp_area_vec[iA].frid.rcid);
                sqlite3_bind_int64(stmt, 3, temp_area_vec[iA].frid.objl);


                std::string objnam = get_attfs_value(temp_area_vec[iA], "OBJNAM");
                sqlite3_bind_text(stmt, 4, objnam.c_str(), -1, SQLITE_TRANSIENT);

                std::string nobjnm = get_natfs_value(temp_area_vec[iA], "NOBJNM");
                sqlite3_bind_text(stmt, 5, nobjnm.c_str(), -1, SQLITE_TRANSIENT);
                
                // 基于面要素的类型和属性值 和 规则，返回fill_type 1, 2, 3
				int fill_type = get_fill_category(temp_area_vec[iA], shpName.c_str());
				sqlite3_bind_int64(stmt, 6, fill_type);

                sqlite3_bind_blob(stmt, 7, blob, blob_size, SQLITE_STATIC);
                sqlite3_bind_int64(stmt, 8, temp_area_vec[iA].frid.rcid);

                ret = sqlite3_step(stmt);
                if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
                {
                    printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
                }
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

//! 处理海图面图层-可视化效果为线
void DBHandler_Spatialite::process_area_common_l()
{
    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "AREA_COMMON_L_" + table_name_level;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, OBJNAM, NOBJNM, LineType, Geometry) \
        SELECT ?, ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name + 
        " WHERE CHARTID = " + m_chart_db_uid_str +" AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        std::map<int, std::vector<AreaFeature> >::iterator areaIter;
        std::map<int, std::vector<AreaFeature> >::iterator areaEnd = m_chart->area_features_map.end();
        for (areaIter = m_chart->area_features_map.begin(); areaIter != areaEnd; ++areaIter)
        {
            int featureId = areaIter->first; 
            std::vector<AreaFeature>& temp_area_vec = areaIter->second;
            std::string shpName = m_chart->code_name_system[featureId];
            
            // 这里先过滤一波
			if (filter_area_line(shpName.c_str()) == false)
            {
                continue;
            }

            int blob_size;
            unsigned char* blob;
            for (int iA = 0; iA < temp_area_vec.size(); iA++)
            {   
                // ruined 直接跳过
                if (temp_area_vec[iA].frid.ruin == 2) continue;


                // 存了那么几个属性
                // CHARTID, RCID, OBJL, OBJNAM, NOBJNM, LineType, Geometry
                gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
                geo_pg->Srid = 4326;
                area_to_spatialite_polyline(temp_area_vec[iA], geo_pg);

                gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
                gaiaFreeGeomColl(geo_pg);

                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);

                sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
                sqlite3_bind_int64(stmt, 2, temp_area_vec[iA].frid.rcid);
                sqlite3_bind_int64(stmt, 3, temp_area_vec[iA].frid.objl);

                std::string objnam = get_attfs_value(temp_area_vec[iA], "OBJNAM");
                sqlite3_bind_text(stmt, 4, objnam.c_str(), objnam.size(), SQLITE_STATIC);
                std::string nobjnm = get_natfs_value(temp_area_vec[iA], "NOBJNM");
                sqlite3_bind_text(stmt, 5, nobjnm.c_str(), nobjnm.size(), SQLITE_STATIC);

                // 基于面要素的类型和属性值，返回line_type 1 -- 7
                int line_type = get_line_category(temp_area_vec[iA], shpName.c_str());
                sqlite3_bind_int64(stmt, 6, line_type);

                sqlite3_bind_blob(stmt, 7, blob, blob_size, SQLITE_STATIC);
                sqlite3_bind_int64(stmt, 8, temp_area_vec[iA].frid.rcid);

                ret = sqlite3_step(stmt);
                if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
                {
                    printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
                }
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

//! 处理海图面图层-可视化效果为点
void DBHandler_Spatialite::process_area_common_m()
{
    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "AREA_COMMON_M_" + table_name_level;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, OBJNAM, NOBJNM, MarkerName1, MarkerName2, ORIENT, Geometry) \
        SELECT ?, ?, ?, ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        std::map<int, std::vector<AreaFeature> >::iterator areaIter;
        std::map<int, std::vector<AreaFeature> >::iterator areaEnd = m_chart->area_features_map.end();
        for (areaIter = m_chart->area_features_map.begin(); areaIter != areaEnd; ++areaIter)
        {
            int featureId = areaIter->first;
            std::vector<AreaFeature>& temp_area_vec = areaIter->second;
            std::string shpName = m_chart->code_name_system[featureId];

            // 同样的，先过滤一遍
            if (filter_area_mark(shpName.c_str()) == false)
            {
                continue;
            }

            int blob_size;
            unsigned char* blob;
            for (int iA = 0; iA < temp_area_vec.size(); iA++)
            {
                // 同样的，ruined 要素直接跳过
                if (temp_area_vec[iA].frid.ruin == 2) continue;

                gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
                geo_pg->Srid = 4326;
                area_to_spatialite_centroid(temp_area_vec[iA], geo_pg);

                gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
                gaiaFreeGeomColl(geo_pg);

                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);

                sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
                sqlite3_bind_int64(stmt, 2, temp_area_vec[iA].frid.rcid);
                sqlite3_bind_int64(stmt, 3, temp_area_vec[iA].frid.objl);

                std::string objnam = get_attfs_value(temp_area_vec[iA], "OBJNAM");

                // 针对BRIDGE要素，对名称进行了特别处理：添加了 clr cl， clr op 这样的前缀
                if (shpName == "BRIDGE")
                {
                    std::string cat_str = get_attfs_value(temp_area_vec[iA], "CATBRG");
                    if (cat_str == "2" || cat_str == "3" || cat_str == "4" || cat_str == "5" || cat_str == "7" || cat_str == "8")
                    {
                        std::string verccl_str = get_attfs_value(temp_area_vec[iA], "VERCCL");
                        std::string vercop_str = get_attfs_value(temp_area_vec[iA], "VERCOP");
                        if (verccl_str != "")
                        {
                            objnam = "clr cl " + verccl_str;
                        }
                        else if (vercop_str != "")
                        {
                            objnam = "clr op " + vercop_str;
                        }
                    }
                    else
                    {
                        if (objnam == "") objnam = "clr " + get_attfs_value(temp_area_vec[iA], "VERCLR");
                    }
                }
                sqlite3_bind_text(stmt, 4, objnam.c_str(), objnam.size(), SQLITE_STATIC);

                std::string nobjnm = get_natfs_value(temp_area_vec[iA], "NOBJNM");
                sqlite3_bind_text(stmt, 5, nobjnm.c_str(), nobjnm.size(), SQLITE_STATIC);


                //!===============================================================
                std::string markername = get_area_marker1(temp_area_vec[iA], shpName.c_str());
                sqlite3_bind_text(stmt, 6, markername.c_str(), markername.size(), SQLITE_STATIC);
                
                
                // 有的要素有一个基础符号，当属性特殊时，还有额外符号，所以这里用markername_2来存储
                // 比如 ACHARE
                //!===============================================================
                std::string markername_1 = get_area_marker2(temp_area_vec[iA], shpName.c_str());
                sqlite3_bind_text(stmt, 7, markername_1.c_str(), markername_1.size(), SQLITE_STATIC);

                //!===============================================================
                // 部分点符号具有ORIENT属性，在渲染时需进行旋转
                if (shpName == "FAIRWY" || shpName == "TSSLPT" || shpName == "DWRTPT" || shpName == "RECTRC" || shpName == "TWRTPT" || shpName == "TS_FEB")
                {
                    std::string orient = get_attfs_value(temp_area_vec[iA], "ORIENT");
                    if (orient == "") orient = "0";
                    sqlite3_bind_double(stmt, 8, std::stod(orient));
                }

                //!===============================================================
                sqlite3_bind_blob(stmt, 9, blob, blob_size, SQLITE_STATIC);
                sqlite3_bind_int64(stmt, 10, temp_area_vec[iA].frid.rcid);

                ret = sqlite3_step(stmt);
                if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
                {
                    printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
                }
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

void DBHandler_Spatialite::process_area_customize()
{
	this->process_area_DEPARE();
	this->process_area_LNDARE();
    /*this->process_area_DEPARE_TILED();
    this->process_area_LNDARE_TILED();*/
    this->process_area_DRGARE();
    this->process_area_OBSTRN();
}

void DBHandler_Spatialite::process_area_DEPARE()
{
    int layer_id = 42;
    std::vector<AreaFeature>& temp_area_vec = m_chart->area_features_map[layer_id];

    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "AREA_DEPARE_" + table_name_level;

    if (m_chart->area_features_map.count(layer_id) == 0) return;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, DRVAL1, DRVAL2, Geometry) \
        SELECT ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        int blob_size;
        unsigned char* blob;
        for (int iA = 0; iA < temp_area_vec.size(); iA++)
        {
            if (temp_area_vec[iA].frid.ruin == 2) continue;

            gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
            geo_pg->Srid = 4326;
            area_to_spatialite_polygon(temp_area_vec[iA], geo_pg);

            gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
            gaiaFreeGeomColl(geo_pg);

            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            // 入库时主要把 DRVAL1 DRVAL2 俩属性给入库了
            // 应该在渲染时做了一些特殊定制的

            sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
            sqlite3_bind_int64(stmt, 2, temp_area_vec[iA].frid.rcid);
            sqlite3_bind_int64(stmt, 3, temp_area_vec[iA].frid.objl);

            std::string drval1 = get_attfs_value(temp_area_vec[iA], "DRVAL1");
            if (drval1 == "") drval1 = "-10";
            sqlite3_bind_double(stmt, 4, std::stod(drval1));
            std::string drval2 = get_natfs_value(temp_area_vec[iA], "DRVAL2");
            if (drval2 == "") drval2 = "-10";
            sqlite3_bind_double(stmt, 5, std::stod(drval2));

            sqlite3_bind_blob(stmt, 6, blob, blob_size, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 7, temp_area_vec[iA].frid.rcid);

            ret = sqlite3_step(stmt);
            if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
            {
                printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

void DBHandler_Spatialite::process_area_LNDARE()
{
    int layer_id = 71;
    std::vector<AreaFeature>& temp_area_vec = m_chart->area_features_map[layer_id];

    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "AREA_LNDARE_" + table_name_level;

    if (m_chart->area_features_map.count(layer_id) == 0) return;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, OBJNAM, NOBJNM, Geometry) \
        SELECT ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        int blob_size;
        unsigned char* blob;
        for (int iA = 0; iA < temp_area_vec.size(); iA++)
        {
            if (temp_area_vec[iA].frid.ruin == 2) continue;

            gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
            geo_pg->Srid = 4326;
            area_to_spatialite_polygon(temp_area_vec[iA], geo_pg);

            gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
            gaiaFreeGeomColl(geo_pg);

            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
            sqlite3_bind_int64(stmt, 2, temp_area_vec[iA].frid.rcid);
            sqlite3_bind_int64(stmt, 3, temp_area_vec[iA].frid.objl);

            std::string objnam = get_attfs_value(temp_area_vec[iA], "OBJNAM");
            sqlite3_bind_text(stmt, 4, objnam.c_str(), objnam.size(), SQLITE_STATIC);
            std::string nobjnm = get_natfs_value(temp_area_vec[iA], "NOBJNM");
            sqlite3_bind_text(stmt, 5, nobjnm.c_str(), nobjnm.size(), SQLITE_STATIC);

            sqlite3_bind_blob(stmt, 6, blob, blob_size, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 7, temp_area_vec[iA].frid.rcid);

            ret = sqlite3_step(stmt);
            if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
            {
                printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

void DBHandler_Spatialite::process_area_DEPARE_TILED()
{
    int layer_id = 42;
    std::vector<AreaFeature>& temp_area_vec = m_chart->area_features_map[layer_id];

    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "AREA_DEPARE_TILED_" + table_name_level;

    if (m_chart->area_features_map.count(layer_id) == 0) return;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
		" (CHARTID, RCID, OBJL, DRVAL1, DRVAL2, TILE_LEVEL, TILE_X, TILE_Y, Geometry)  \
		SELECT ?, ?, ?, ?, ?, ?, ?, ?, ? \
		WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
		" WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ? \
												   AND TILE_LEVEL = ? \
												   AND TILE_X = ? \
												   AND TILE_Y = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        int blob_size;
        unsigned char* blob;
        for (int iA = 0; iA < temp_area_vec.size(); iA++)
        {
            if (temp_area_vec[iA].frid.ruin == 2) continue;

            std::vector<tiled_spatialite_geom> polygon_list = area_to_tiled_spatialite_polygon(temp_area_vec[iA]);
            for (int iPoly = 0; iPoly < polygon_list.size(); iPoly++)
            {
                gaiaGeomCollPtr geo_pg = polygon_list[iPoly].geom;

                gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
                gaiaFreeGeomColl(geo_pg);

                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);

                sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
                sqlite3_bind_int64(stmt, 2, temp_area_vec[iA].frid.rcid);
                sqlite3_bind_int64(stmt, 3, temp_area_vec[iA].frid.objl);

                std::string drval1 = get_attfs_value(temp_area_vec[iA], "DRVAL1");
                if (drval1 == "") drval1 = "-10";
                sqlite3_bind_double(stmt, 4, std::stod(drval1));
                std::string drval2 = get_natfs_value(temp_area_vec[iA], "DRVAL2");
                if (drval2 == "") drval2 = "-10";
                sqlite3_bind_double(stmt, 5, std::stod(drval2));

				sqlite3_bind_int64(stmt, 6, polygon_list[iPoly].tile_level);
				sqlite3_bind_int64(stmt, 7, polygon_list[iPoly].tile_x);
				sqlite3_bind_int64(stmt, 8, polygon_list[iPoly].tile_y);

                sqlite3_bind_blob(stmt, 9, blob, blob_size, SQLITE_STATIC);

				sqlite3_bind_int64(stmt, 10, temp_area_vec[iA].frid.rcid);
				sqlite3_bind_int64(stmt, 11, polygon_list[iPoly].tile_level);
				sqlite3_bind_int64(stmt, 12, polygon_list[iPoly].tile_x);
				sqlite3_bind_int64(stmt, 13, polygon_list[iPoly].tile_y);

                ret = sqlite3_step(stmt);
                if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
                {
                    printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
                }
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

void DBHandler_Spatialite::process_area_LNDARE_TILED()
{
    int layer_id = 71;
    std::vector<AreaFeature>& temp_area_vec = m_chart->area_features_map[layer_id];

    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "AREA_LNDARE_TILED_" + table_name_level;

    if (m_chart->area_features_map.count(layer_id) == 0) return;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
		" (CHARTID, RCID, OBJL, OBJNAM, NOBJNM, TILE_LEVEL, TILE_X, TILE_Y, Geometry)  \
		SELECT ?, ?, ?, ?, ?, ?, ?, ?, ? \
		WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
		" WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ? \
												   AND TILE_LEVEL = ? \
												   AND TILE_X = ? \
												   AND TILE_Y = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        int blob_size;
        unsigned char* blob;
        for (int iA = 0; iA < temp_area_vec.size(); iA++)
        {
            if (temp_area_vec[iA].frid.ruin == 2) continue;

            std::vector<tiled_spatialite_geom> polygon_list = area_to_tiled_spatialite_polygon(temp_area_vec[iA]);
            for (int iPoly = 0; iPoly < polygon_list.size(); iPoly++)
            {
                gaiaGeomCollPtr geo_pg = polygon_list[iPoly].geom;

                gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
                gaiaFreeGeomColl(geo_pg);

                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);

                sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
                sqlite3_bind_int64(stmt, 2, temp_area_vec[iA].frid.rcid);
                sqlite3_bind_int64(stmt, 3, temp_area_vec[iA].frid.objl);

                std::string objnam = get_attfs_value(temp_area_vec[iA], "OBJNAM");
                sqlite3_bind_text(stmt, 4, objnam.c_str(), objnam.size(), SQLITE_STATIC);
                std::string nobjnm = get_natfs_value(temp_area_vec[iA], "NOBJNM");
                sqlite3_bind_text(stmt, 5, nobjnm.c_str(), nobjnm.size(), SQLITE_STATIC);

				sqlite3_bind_int64(stmt, 6, polygon_list[iPoly].tile_level);
				sqlite3_bind_int64(stmt, 7, polygon_list[iPoly].tile_x);
				sqlite3_bind_int64(stmt, 8, polygon_list[iPoly].tile_y);

                sqlite3_bind_blob(stmt, 9, blob, blob_size, SQLITE_STATIC);

				sqlite3_bind_int64(stmt, 10, temp_area_vec[iA].frid.rcid);
				sqlite3_bind_int64(stmt, 11, polygon_list[iPoly].tile_level);
				sqlite3_bind_int64(stmt, 12, polygon_list[iPoly].tile_x);
				sqlite3_bind_int64(stmt, 13, polygon_list[iPoly].tile_y);

                ret = sqlite3_step(stmt);
                if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
                {
                    printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
                }
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

void DBHandler_Spatialite::process_area_DRGARE()
{
    int layer_id = 46;
    std::vector<AreaFeature>& temp_area_vec = m_chart->area_features_map[layer_id];

    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "AREA_DRGARE_" + table_name_level;

    if (m_chart->area_features_map.count(layer_id) == 0) return;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, DRVAL1, DRVAL2, Geometry) \
        SELECT ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        int blob_size;
        unsigned char* blob;
        for (int iA = 0; iA < temp_area_vec.size(); iA++)
        {
            if (temp_area_vec[iA].frid.ruin == 2) continue;

            gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
            geo_pg->Srid = 4326;
            area_to_spatialite_polygon(temp_area_vec[iA], geo_pg);

            gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
            gaiaFreeGeomColl(geo_pg);

            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
            sqlite3_bind_int64(stmt, 2, temp_area_vec[iA].frid.rcid);
            sqlite3_bind_int64(stmt, 3, temp_area_vec[iA].frid.objl);

            // 入库时主要把 DRVAL1 DRVAL2 俩属性给入库了
            // 应该在渲染时做了一些特殊定制的

            std::string drval1 = get_attfs_value(temp_area_vec[iA], "DRVAL1");
            if (drval1 == "") drval1 = "-10";
            sqlite3_bind_double(stmt, 4, std::stod(drval1));
            std::string drval2 = get_attfs_value(temp_area_vec[iA], "DRVAL2");
            if (drval2 == "") drval2 = "-10";
            sqlite3_bind_double(stmt, 5, std::stod(drval2));

            sqlite3_bind_blob(stmt, 6, blob, blob_size, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 7, temp_area_vec[iA].frid.rcid);

            ret = sqlite3_step(stmt);
            if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
            {
                printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

void DBHandler_Spatialite::process_area_OBSTRN()
{
    int layer_id = 86;
    std::vector<AreaFeature>& temp_area_vec = m_chart->area_features_map[layer_id];

    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "AREA_OBSTRN_" + table_name_level;

    if (m_chart->area_features_map.count(layer_id) == 0) return;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, OBJNAM, NOBJNM, FILLTYPE, Geometry) \
        SELECT ?, ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        int blob_size;
        unsigned char* blob;
        for (int iA = 0; iA < temp_area_vec.size(); iA++)
        {
            if (temp_area_vec[iA].frid.ruin == 2) continue;

            gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
            geo_pg->Srid = 4326;
            area_to_spatialite_polygon(temp_area_vec[iA], geo_pg);

            gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
            gaiaFreeGeomColl(geo_pg);

            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
            sqlite3_bind_int64(stmt, 2, temp_area_vec[iA].frid.rcid);
            sqlite3_bind_int64(stmt, 3, temp_area_vec[iA].frid.objl);

            std::string objnam = get_attfs_value(temp_area_vec[iA], "OBJNAM");
            sqlite3_bind_text(stmt, 4, objnam.c_str(), objnam.size(), SQLITE_STATIC);
            std::string nobjnm = get_natfs_value(temp_area_vec[iA], "NOBJNM");
            sqlite3_bind_text(stmt, 5, nobjnm.c_str(), nobjnm.size(), SQLITE_STATIC);

            int fill_type = 0;
            {
                std::string catobs = get_attfs_value(temp_area_vec[iA], "CATOBS");
                std::string watlev = get_attfs_value(temp_area_vec[iA], "WATLEV");
                std::string expsou = get_attfs_value(temp_area_vec[iA], "EXPSOU");
                std::string valsou = get_attfs_value(temp_area_vec[iA], "VALSOU");

                /*Other_Symbolized: CATOBS7&& VALSOU : SY(FOULGND1); LC(NAVARE51)
                Displaybase_Symbolized : CATOBS10: SY(FLTHAZ02); LS(DASH, 1, CSTLN)
                Other_Symbolized : CATOBS6: CS(OBSTRN04); AP(FOULAR01); LS(DOTT, 2, CHBLK)
                Other_Symbolized : CATOBS7: SY(FOULGND1); LC(NAVARE51)
                Displaybase_Symbolized : CATOBS8: SY(FLTHAZ02); LS(DASH, 1, CSTLN)
                Displaybase_Symbolized : CATOBS9: SY(ACHARE02); LS(DASH, 1, CHMGD)
                Displaybase_Symbolized : WATLEV7: SY(FLTHAZ02); LS(DASH, 1, CSTLN)
                Other_Symbolized : : CS(OBSTRN04)*/
                if (catobs == "7" && valsou!="") {}
                else if (catobs == "10") {}
                else if (catobs == "6") fill_type = 1; // Pattern: FOULAR01; DEPVS
                else if (catobs == "7") {}
                else if (catobs == "8") {}
                else if (catobs == "9") {}
                else if (watlev == "7") {}
                else
                {
                    if (watlev == "")  // default
                        fill_type = 2; // DEPVS
                    else {
                        int watlev_val = atoi(watlev.c_str());
                        switch (watlev_val) {
                        case 1:
                        case 2:
                            fill_type = 3; // CHBRN
                            break;
                        case 4:
                            fill_type = 4; // DEPIT
                            break;
                        case 5:
                        case 3: {
                            fill_type = 2; // DEPVS
                        } break;
                        default:
                            fill_type = 2; // DEPVS
                            break;
                        }
                    }
                }
            }
            sqlite3_bind_int64(stmt, 6, fill_type);

            sqlite3_bind_blob(stmt, 7, blob, blob_size, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 8, temp_area_vec[iA].frid.rcid);

            ret = sqlite3_step(stmt);
            if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
            {
                printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
            }

        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

void DBHandler_Spatialite::process_line_common_ml()
{
    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "LINE_COMMON_ML_" + table_name_level;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, OBJNAM, NOBJNM, CATEGORY, Geometry) \
        SELECT ?, ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        std::map<int, std::vector<LineFeature> >::iterator lineIter;
        std::map<int, std::vector<LineFeature> >::iterator lineEnd = m_chart->line_features_map.end();
        for (lineIter = m_chart->line_features_map.begin(); lineIter != lineEnd; ++lineIter)
        {
            int featureId = lineIter->first;
            std::vector<LineFeature>& temp_line_vec = lineIter->second;;
            std::string shpName = m_chart->code_name_system[featureId];

            int blob_size;
            unsigned char* blob;
            for (int iP = 0; iP < temp_line_vec.size(); iP++)
            {
                if (temp_line_vec[iP].frid.ruin == 2) continue;

                LineFeature line = temp_line_vec[iP];
                int lineSegCount = line.GetLineCount();

                gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
                geo_pg->Srid = 4326;

                for (int iSeg = 0; iSeg < lineSegCount; iSeg++)
                {
                    std::vector<sg2d_t> points = line.GetSG2DsVector(iSeg, m_chart->edge_vectors_map);
                    gaiaLinestringPtr temp_linestring = gaiaAddLinestringToGeomColl(geo_pg, points.size());
                    for (int iP = 0; iP < points.size(); iP++)
                    {
                        gaiaSetPoint(temp_linestring->Coords, iP, points[iP].long_lat[0], points[iP].long_lat[1]);
                    }
                }//! END for iSub

                if (lineSegCount == 1) //! 此处是为了兼容MultiLineString
                {
                    gaiaLinestringPtr temp_linestring = gaiaAddLinestringToGeomColl(geo_pg, 2);
                    gaiaSetPoint(temp_linestring->Coords, 0, 0, 0 );
                    gaiaSetPoint(temp_linestring->Coords, 1, 0, 0);
                }

                gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
                gaiaFreeGeomColl(geo_pg);

                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);

                sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
                sqlite3_bind_int64(stmt, 2, temp_line_vec[iP].frid.rcid);
                sqlite3_bind_int64(stmt, 3, temp_line_vec[iP].frid.objl);

                std::string objnam = get_attfs_value(temp_line_vec[iP], "OBJNAM");
                // 特殊名称处理
                if (shpName == "NAVLNE")
                {
                    objnam = get_attfs_value(temp_line_vec[iP], "ORIENT");
                }
                if (shpName == "PIPOHD")
                {
                    objnam = get_attfs_value(temp_line_vec[iP], "VERCLR");
                    objnam = "clr " + objnam;
                }
                sqlite3_bind_text(stmt, 4, objnam.c_str(), objnam.size(), SQLITE_STATIC);

                std::string nobjnm = get_natfs_value(temp_line_vec[iP], "NOBJNM");
                sqlite3_bind_text(stmt, 5, nobjnm.c_str(), nobjnm.size(), SQLITE_STATIC);

                // 基于ID和属性的，给了个category 1 - 5，应该和渲染时有匹配
                int category = get_line_category(line, shpName.c_str());
                sqlite3_bind_int64(stmt, 6, category);

                sqlite3_bind_blob(stmt, 7, blob, blob_size, SQLITE_STATIC);
                sqlite3_bind_int64(stmt, 8, temp_line_vec[iP].frid.rcid);

                ret = sqlite3_step(stmt);
                if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
                {
                    printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
                }

            }//! END for iP
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

void DBHandler_Spatialite::process_point_common_icon()
{
    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "POINT_COMMON_ICON_" + table_name_level;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, OBJNAM, NOBJNM, CATEGORY, VAL1, VAL2, Geometry) \
        SELECT ?, ?, ?, ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
        std::map<int, std::vector<PointFeature> >::iterator pfIter;
        std::map<int, std::vector<PointFeature> >::iterator end = m_chart->point_features_map.end();
        for (pfIter = m_chart->point_features_map.begin(); pfIter != end; ++pfIter)
        {
            int featureId = pfIter->first;
            std::vector<PointFeature>& temp_point_vec = pfIter->second;
            std::string shpName = m_chart->code_name_system[featureId];
            /*if (!(shpName == "ACHBRT" || shpName == "AIRARE" || shpName == "ACHARE" ||
                shpName == "BCNCAR" || shpName == "BCNISD" || shpName == "BCNLAT" ||
                shpName == "BCNSAW" || shpName == "BCNSPP" || shpName == "BERTHS" ||
                shpName == "BOYINB" ||
                shpName == "BOYISD" || shpName == "BOYLAT" || shpName == "BOYSPP" || shpName == "BOYSAW" ||
                shpName == "BUAARE" || shpName == "BUISGL" ||
                shpName == "CHKPNT" || shpName == "CGUSTA" || shpName == "CRANES" || shpName == "CTSARE" || shpName == "CTNARE" || 
                shpName == "DAMCON" || shpName == "DISMAR" || shpName == "DMPGRD" ||
                shpName == "FOGSIG" || shpName == "FORSTC" ||
                shpName == "GATCON" ||
                shpName == "HRBFAC" || shpName == "HULKES" ||
                shpName == "ICNARE" ||
                shpName == "LNDARE" || shpName == "LNDMRK" || shpName == "LNDRGN" || 
                shpName == "LIGHTS" || shpName == "LITFLT" || shpName == "LITVES" ||
                shpName == "MARCUL" || shpName == "MORFAC" || 
                shpName == "OFSPLF" ||
                shpName == "PILBOP" || shpName == "PILPNT" || shpName == "PIPARE" || shpName == "PYLONS" ||
                shpName == "PRCARE" || shpName == "PRDARE" ||
                shpName == "RADRFL" || shpName == "RDOSTA" || shpName == "RTPBCN" || shpName == "RETRFL" ||
                shpName == "SNDWAV" || shpName == "SILTNK" || shpName == "SLCONS" || shpName == "SPLARE" ||
                shpName == "SISTAT" ||
                shpName == "TOPMAR" ||
                shpName == "NEWOBJ"
                ))
            {
                continue;
            }*/

            if (shpName == "SOUNDG")
            {
                continue;
            }

            int blob_size;
            unsigned char* blob;
            for (int iP = 0; iP < temp_point_vec.size(); iP++)
            {
                if (temp_point_vec[iP].frid.ruin == 2) continue;

                gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
                geo_pg->Srid = 4326;
                point_to_spatialite_point(temp_point_vec[iP], geo_pg);

                gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
                gaiaFreeGeomColl(geo_pg);

                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);

                sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
                sqlite3_bind_int64(stmt, 2, temp_point_vec[iP].frid.rcid);
                sqlite3_bind_int64(stmt, 3, temp_point_vec[iP].frid.objl);

                // 对象名处理
                std::string objnam = get_attfs_value(temp_point_vec[iP], "OBJNAM");
                if (shpName == "ANNOTA") objnam = get_attfs_value(temp_point_vec[iP], "ANTXT1");
                if (shpName == "OBSTRN") objnam = get_attfs_value(temp_point_vec[iP], "VALSOU");
                if (shpName == "CURENT") objnam = get_attfs_value(temp_point_vec[iP], "CURVEL");
                if (shpName == "LNDELV") objnam = get_attfs_value(temp_point_vec[iP], "ELEVAT");
                if (shpName == "MAGVAR") objnam = get_attfs_value(temp_point_vec[iP], "VALMAG");
                if (shpName == "SBDARE")
                {
                    objnam = get_attfs_value(temp_point_vec[iP], "NATSUR");
                    
                    if ("1" == objnam) objnam = "mud";
                    if ("2" == objnam) objnam = "clay";
                    if ("3" == objnam) objnam = "silt";
                    if ("4" == objnam) objnam = "sand";
                    if ("5" == objnam) objnam = "stone";
                    if ("6" == objnam) objnam = "gravel";
                    if ("7" == objnam) objnam = "pebbles";
                    if ("8" == objnam) objnam = "cobbles";
                    if ("9" == objnam) objnam = "rock";
                    if ("11" == objnam) objnam = "lava";
                    if ("14" == objnam) objnam = "coral";
                    if ("17" == objnam) objnam = "shells";
                    if ("18" == objnam) objnam = "boulder";
                }
                if (shpName == "WRECKS") objnam = "Wk"; //具体的深度值在VAL1中存储
                if (shpName == "UWTROC") objnam = get_attfs_value(temp_point_vec[iP], "VALSOU");
                if (shpName == "TS_FEB")  objnam = get_attfs_value(temp_point_vec[iP], "CURVEL") + " kn";
                sqlite3_bind_text(stmt, 4, objnam.c_str(), objnam.size(), SQLITE_STATIC);

                std::string nobjnm = get_natfs_value(temp_point_vec[iP], "NOBJNM");
                sqlite3_bind_text(stmt, 5, nobjnm.c_str(), nobjnm.size(), SQLITE_STATIC);

				int marker_category = get_marker_category(temp_point_vec[iP], shpName.c_str());
				sqlite3_bind_int64(stmt, 6, marker_category);

				if (shpName == "LIGHTS")
				{
					int icon_type = -1;
					std::string catlit_str = get_attfs_value(temp_point_vec[iP], "CATLIT");
					std::string valnmr_str = get_attfs_value(temp_point_vec[iP], "VALNMR");
					std::string litvis_str = get_attfs_value(temp_point_vec[iP], "LITVIS");
					std::string colour_str = get_attfs_value(temp_point_vec[iP], "COLOUR");

					if (catlit_str != "")
					{
						char catlit[1024] = { '\0' };
						_parseList(catlit_str.c_str(), catlit, sizeof(catlit));
						if (STRPBRK(catlit, "\010\013")) icon_type = LIGHTS_ICON::LIGHTS_LIGHTS82;
						if (STRPBRK(catlit, "\011")) icon_type = LIGHTS_ICON::LIGHTS_LIGHTS81;
					}
					if (icon_type<0)
					{
						char colist[1024] = { '\0' };
						if (colour_str != "")
							_parseList(colour_str.c_str(), colist, sizeof(colist));
						else {
							colist[0] = '\014';  // magenta (12)
							colist[1] = '\000';
						}

						std::string sectr1_str = get_attfs_value(temp_point_vec[iP], "SECTR1");
						std::string sectr2_str = get_attfs_value(temp_point_vec[iP], "SECTR2");
						if (sectr1_str == "" || sectr2_str == "")
						{
							double valnmr = atof(valnmr_str.c_str());
							if (valnmr < 10.0)
								;
							else //! 整个圆
							{
								int radius = 3;
								if (valnmr > 0) 
								{
									if (valnmr < 7.0) radius = 3;
									else if (valnmr < 15.0) radius = 10;
									else if (valnmr < 30.0) radius = 15;
									else radius = 20;
								}

								// max 1 color
								if ('\0' == colist[1]) 
								{
									if (STRPBRK(colist, "\003"))
									{
										icon_type = 101; // LITRD
										radius += 1;
									}
									else if (STRPBRK(colist, "\004"))
									{
										icon_type = 102; // LITGN
									}
									else if (STRPBRK(colist, "\001\006\011"))
									{
										icon_type = 103; // LITYW
										radius += 2;
									}
									else if (STRPBRK(colist, "\014"))
									{
										icon_type = 104; // CHMGD
										radius += 3;
									}
									else
									{
										icon_type = 104; // CHMGD
										radius += 5;
									}
								}
								else if ('\0' == colist[2])  // or 2 color
								{
									if (STRPBRK(colist, "\001") && STRPBRK(colist, "\003"))
									{
										icon_type = 101; // LITRD
										radius += 1;
									}
									else if (STRPBRK(colist, "\001") && STRPBRK(colist, "\004"))
									{
										icon_type = 102; // LITGN
									}
									else
									{
										icon_type = 104; // CHMGD
										radius += 5;
									}
								}
								else
								{
									icon_type = 104; // CHMGD
									radius += 5;
								}
								sqlite3_bind_double(stmt, 7, radius);
							}
						}
						else // 半圆
						{
							icon_type = 200;

							double sectr1 = std::stod(sectr1_str);
							double sectr2 = std::stod(sectr2_str);

							sqlite3_bind_double(stmt, 7, sectr1);
							sqlite3_bind_double(stmt, 8, sectr2);
						}
					}
				}
				if (shpName == "RDOCAL")
				{
					std::string orient_str = get_attfs_value(temp_point_vec[iP], "ORIENT");
					sqlite3_bind_double(stmt, 7, std::atof(orient_str.c_str()));
				}
				if (shpName == "RCTLPT")
				{
					std::string orient_str = get_attfs_value(temp_point_vec[iP], "ORIENT");
					sqlite3_bind_double(stmt, 7, std::atof(orient_str.c_str()));
				}
				if (shpName == "WRECKS")
				{
					std::string valsou_str = get_attfs_value(temp_point_vec[iP], "VALSOU");
					if (valsou_str != "")
					{
						double valsou = atof(valsou_str.c_str());
						sqlite3_bind_double(stmt, 7, valsou);
					}
				}
				if (shpName == "TS_FEB")
				{
					std::string orient_str = get_attfs_value(temp_point_vec[iP], "ORIENT");
					sqlite3_bind_double(stmt, 7, atof(orient_str.c_str()));
				}

                sqlite3_bind_blob(stmt, 9, blob, blob_size, SQLITE_STATIC);
                sqlite3_bind_int64(stmt, 10, temp_point_vec[iP].frid.rcid);

                ret = sqlite3_step(stmt);
                if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
                {
                    printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
                }
            }
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

void DBHandler_Spatialite::process_point_SOUNDG()
{
	int layer_id = 129;
	std::vector<PointFeature>& temp_point_vec = m_chart->point_features_map[layer_id];

    char* err_msg = NULL;

    char level_char[10];
    sprintf_s(level_char, "%d", m_chart->m_cscl_level);
    std::string table_name_level = level_char;
    std::string table_name = "POINT_SOUNDG_" + table_name_level;

	if (m_chart->point_features_map.count(layer_id) == 0) return;

    //! 开始添加内容
    std::string sql_begin = "BEGIN";
    int ret = sqlite3_exec(handle, sql_begin.c_str(), NULL, NULL, &err_msg);

    std::string sql_insert_item = "INSERT INTO " + table_name +
        " (CHARTID, RCID, OBJL, OBJNAM, NOBJNM, Geometry) \
        SELECT ?, ?, ?, ?, ?, ? \
        WHERE NOT EXISTS (SELECT 1 FROM " + table_name +
        " WHERE CHARTID = " + m_chart_db_uid_str + " AND RCID = ?)";
    ret = sqlite3_prepare_v2(handle, sql_insert_item.c_str(), sql_insert_item.size(), &stmt, NULL);
    if (ret == SQLITE_OK)
    {
		int blob_size;
		unsigned char* blob;
		for (int iP = 0; iP < temp_point_vec.size(); iP++)
		{
			if (temp_point_vec[iP].frid.ruin == 2) continue;

			PointFeature point = temp_point_vec[iP];
			std::vector<sg3d_t> sg3dList = m_chart->isolated_node_vectors_map[point.ref_record_ids[0]].GetSG3DVector();
			int p_count = sg3dList.size();

			for (int i = 0; i < p_count; i++)
			{
				double lon = sg3dList[i].long_lat[0];
				double lat = sg3dList[i].long_lat[1];
				double depth = sg3dList[i].depth;

				gaiaGeomCollPtr geo_pg = gaiaAllocGeomCollXYZ();
				geo_pg->Srid = 4326;
				gaiaAddPointToGeomCollXYZ(geo_pg, lon, lat, depth);

				gaiaToSpatiaLiteBlobWkb(geo_pg, &blob, &blob_size);
				gaiaFreeGeomColl(geo_pg);

				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);

				int rcid = temp_point_vec[iP].frid.rcid * 10000 + i;

				sqlite3_bind_int64(stmt, 1, m_chart_db_uid);
				sqlite3_bind_int64(stmt, 2, rcid);
				sqlite3_bind_int64(stmt, 3, temp_point_vec[iP].frid.objl);

				std::string objnam = get_attfs_value(temp_point_vec[iP], "OBJNAM");
				sqlite3_bind_text(stmt, 4, objnam.c_str(), objnam.size(), SQLITE_STATIC);

				std::string nobjnm = get_natfs_value(temp_point_vec[iP], "NOBJNM");
				sqlite3_bind_text(stmt, 5, nobjnm.c_str(), nobjnm.size(), SQLITE_STATIC);

				sqlite3_bind_blob(stmt, 6, blob, blob_size, SQLITE_STATIC);
				sqlite3_bind_int64(stmt, 7, rcid);

				ret = sqlite3_step(stmt);
				if (!(ret == SQLITE_DONE || ret == SQLITE_ROW))
				{
					printf("sqlite3_step() error: %s\n", sqlite3_errmsg(handle));
				}
			}
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(handle));
    }

    std::string sql_commit = "COMMIT";
    ret = sqlite3_exec(handle, sql_commit.c_str(), NULL, NULL, &err_msg);
}

//! 关闭数据库
void DBHandler_Spatialite::close_database()
{
    sqlite3_close(handle);
    spatialite_cleanup_ex(cache);
}


std::vector<tiled_spatialite_geom> DBHandler_Spatialite::area_to_tiled_spatialite_polygon(AreaFeature& area)
{
    int edgeCount = area.fspts.size();
    int feature_rcid = area.frid.rcid;

    OGRPolygon polygon;
    OGRLinearRing temp_ring;
    for (int iE = 0; iE < edgeCount; iE++)
    {
        fspt_t temp_fspt = area.fspts[iE];
        int edgeId = temp_fspt.rcid;
        int orient = temp_fspt.ornt;
        int usage = temp_fspt.usag;
        int mask = temp_fspt.mask;

        if (m_chart->edge_vectors_map[edgeId].getIsClosedEdge())
        {
            OGRLinearRing colosed_ring;
            std::vector<sg2d_t> sg2ds = m_chart->edge_vectors_map[edgeId].GetSG2Ds();
            if (orient == 2)
            {
                for (int iP = sg2ds.size() - 1; iP >= 0; iP--)
                {
                    colosed_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            else
            {
                for (int iP = 0; iP < sg2ds.size(); iP++)
                {
                    colosed_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            polygon.addRing(&colosed_ring);
        }
        else
        {
            std::vector<sg2d_t> sg2ds = m_chart->edge_vectors_map[edgeId].GetSG2Ds();
            if (orient == 2)
            {
                for (int iP = sg2ds.size() - 1; iP >= 0; iP--)
                {
                    temp_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            else
            {
                for (int iP = 0; iP < sg2ds.size(); iP++)
                {
                    temp_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            if (temp_ring.get_IsClosed() == true)
            {
                polygon.addRing(&temp_ring);
                temp_ring.empty();
            }
        }
    }
    if (temp_ring.IsEmpty() == false && temp_ring.get_IsClosed() == false)
    {
        temp_ring.closeRings();
        polygon.addRing(&temp_ring);
        temp_ring.empty();
    }

    OGREnvelope bound_geom;
    polygon.getEnvelope(&bound_geom);
    double minx = bound_geom.MinX;
    double miny = bound_geom.MinY;
    double maxx = bound_geom.MaxX;
    double maxy = bound_geom.MaxY;

    double level = m_chart->m_cscl_level;

    Tile t1 = LonLat2Tile(minx, miny, level);
    Tile t2 = LonLat2Tile(maxx, maxy, level);

    std::vector<tiled_spatialite_geom> tiled_polygon_list;
    for (int ix = t1.x; ix <= t2.x; ix++)
    {
        for (int iy = t1.y; iy <= t2.y; iy++)
        {
            LonLatRect temp_rect = Tile2LonLat(ix, iy, level);
            OGRLinearRing temp_ring;
            temp_ring.addPoint(temp_rect.lon0, temp_rect.lat0);
            temp_ring.addPoint(temp_rect.lon1, temp_rect.lat0);
            temp_ring.addPoint(temp_rect.lon1, temp_rect.lat1);
            temp_ring.addPoint(temp_rect.lon0, temp_rect.lat1);
            temp_ring.closeRings();
            OGRPolygon temp_polygon;
            temp_polygon.addRing(&temp_ring);

            OGRGeometry* temp_result = polygon.Intersection(&temp_polygon);
			if (temp_result == NULL) 
				continue;

            OGRwkbGeometryType geom_type = temp_result->getGeometryType();

            if (geom_type == OGRwkbGeometryType::wkbPolygon)
            {
                OGRPolygon* temp_result_polygon = (OGRPolygon*)temp_result;
                if (temp_result_polygon->getExteriorRing() == NULL) 
                    continue;

                int vert_num = temp_result_polygon->getExteriorRing()->getNumPoints();
                int hole_num = temp_result_polygon->getNumInteriorRings();
                
                gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
                geo_pg->Srid = 4326;
                gaiaPolygonPtr g_polygon = gaiaAddPolygonToGeomColl(geo_pg, vert_num, hole_num);

                gaiaRingPtr g_ext_ring = g_polygon->Exterior;
                OGRLinearRing* ext_ring = temp_result_polygon->getExteriorRing();
                for (int iP = 0; iP < ext_ring->getNumPoints(); iP++)
                {
                    gaiaSetPoint(g_ext_ring->Coords, iP, ext_ring->getX(iP), ext_ring->getY(iP));
                }

                for (int iR = 0; iR < g_polygon->NumInteriors; iR++)
                {
                    OGRLinearRing* temp_ring = temp_result_polygon->getInteriorRing(iR);
                    int temp_v_count = temp_ring->getNumPoints();
                    gaiaRingPtr temp_g_in_ring = gaiaAddInteriorRing(g_polygon, iR, temp_v_count);

                    for (int iP = 0; iP < temp_ring->getNumPoints(); iP++)
                    {
                        gaiaSetPoint(temp_g_in_ring->Coords, iP, temp_ring->getX(iP), temp_ring->getY(iP));
                    }
                }

				tiled_spatialite_geom temp_tiled_geom;
				temp_tiled_geom.geom = geo_pg;
				temp_tiled_geom.tile_level = level;
				temp_tiled_geom.tile_x = ix;
				temp_tiled_geom.tile_y = iy;
                tiled_polygon_list.push_back(temp_tiled_geom);
            }
            else if (geom_type == OGRwkbGeometryType::wkbMultiPolygon)
            {
                OGRMultiPolygon* temp_result_polygon = (OGRMultiPolygon*)temp_result;
                int count = temp_result_polygon->getNumGeometries();
                for (int iM = 0; iM < count; iM++)
                {
                    OGRPolygon* temp = (OGRPolygon*)temp_result_polygon->getGeometryRef(iM);

                    int vert_num = temp->getExteriorRing()->getNumPoints();
                    int hole_num = temp->getNumInteriorRings();

                    gaiaGeomCollPtr geo_pg = gaiaAllocGeomColl();
                    geo_pg->Srid = 4326;
                    gaiaPolygonPtr g_polygon = gaiaAddPolygonToGeomColl(geo_pg, vert_num, hole_num);

                    gaiaRingPtr g_ext_ring = g_polygon->Exterior;
                    OGRLinearRing* ext_ring = temp->getExteriorRing();
                    for (int iP = 0; iP < ext_ring->getNumPoints(); iP++)
                    {
                        gaiaSetPoint(g_ext_ring->Coords, iP, ext_ring->getX(iP), ext_ring->getY(iP));
                    }

                    for (int iR = 0; iR < g_polygon->NumInteriors; iR++)
                    {
                        OGRLinearRing* temp_ring = temp->getInteriorRing(iR);
                        int temp_v_count = temp_ring->getNumPoints();
                        gaiaRingPtr temp_g_in_ring = gaiaAddInteriorRing(g_polygon, iR, temp_v_count);

                        for (int iP = 0; iP < temp_ring->getNumPoints(); iP++)
                        {
                            gaiaSetPoint(temp_g_in_ring->Coords, iP, temp_ring->getX(iP), temp_ring->getY(iP));
                        }
                    }

					tiled_spatialite_geom temp_tiled_geom;
					temp_tiled_geom.geom = geo_pg;
					temp_tiled_geom.tile_level = level;
					temp_tiled_geom.tile_x = ix;
					temp_tiled_geom.tile_y = iy;
					tiled_polygon_list.push_back(temp_tiled_geom);
                }
            }
        }
    }
    return tiled_polygon_list;
}

void DBHandler_Spatialite::area_to_spatialite_polygon(AreaFeature& area, gaiaGeomCollPtr geo)
{
    int edgeCount = area.fspts.size();
    int feature_rcid = area.frid.rcid;

    OGRPolygon polygon;
    OGRLinearRing temp_ring;
    for (int iE = 0; iE < edgeCount; iE++)
    {
        fspt_t temp_fspt = area.fspts[iE];
        int edgeId = temp_fspt.rcid;
        int orient = temp_fspt.ornt;
        int usage = temp_fspt.usag;
        int mask = temp_fspt.mask;

        if (m_chart->edge_vectors_map[edgeId].getIsClosedEdge())
        {
            OGRLinearRing colosed_ring;
            std::vector<sg2d_t> sg2ds = m_chart->edge_vectors_map[edgeId].GetSG2Ds();
            if (orient == 2)
            {
                for (int iP = sg2ds.size() - 1; iP >= 0; iP--)
                {
                    colosed_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            else
            {
                for (int iP = 0; iP < sg2ds.size(); iP++)
                {
                    colosed_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            polygon.addRing(&colosed_ring);
        }
        else
        {
            std::vector<sg2d_t> sg2ds = m_chart->edge_vectors_map[edgeId].GetSG2Ds();
            if (orient == 2)
            {
                for (int iP = sg2ds.size() - 1; iP >= 0; iP--)
                {
                    temp_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            else
            {
                for (int iP = 0; iP < sg2ds.size(); iP++)
                {
                    temp_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            if (temp_ring.get_IsClosed() == true)
            {
                polygon.addRing(&temp_ring);
                temp_ring.empty();
            }
        }
    }
    if (temp_ring.IsEmpty() == false && temp_ring.get_IsClosed() == false)
    {
        temp_ring.closeRings();
        polygon.addRing(&temp_ring);
        temp_ring.empty();
    }

    //! 初始化一个gaia的Polygon对象
    gaiaPolygonPtr g_polygon = gaiaAddPolygonToGeomColl(geo,
        polygon.getExteriorRing()->getNumPoints(), polygon.getNumInteriorRings());

    gaiaRingPtr g_ext_ring = g_polygon->Exterior;
    OGRLinearRing* ext_ring = polygon.getExteriorRing();
    for (int iP = 0; iP < ext_ring->getNumPoints(); iP++)
    {
        gaiaSetPoint(g_ext_ring->Coords, iP, ext_ring->getX(iP), ext_ring->getY(iP));
    }

    for (int iR = 0; iR < g_polygon->NumInteriors; iR++)
    {
        OGRLinearRing* temp_ring = polygon.getInteriorRing(iR);
        int temp_v_count = temp_ring->getNumPoints();
        gaiaRingPtr temp_g_in_ring = gaiaAddInteriorRing(g_polygon, iR, temp_v_count);

        for (int iP = 0; iP < temp_ring->getNumPoints(); iP++)
        {
            gaiaSetPoint(temp_g_in_ring->Coords, iP, temp_ring->getX(iP), temp_ring->getY(iP));
        }
    }
}

void DBHandler_Spatialite::area_to_spatialite_polyline(AreaFeature& area, gaiaGeomCollPtr geo)
{
    int edgeCount = area.fspts.size();
    int feature_rcid = area.frid.rcid;

    OGRPolygon polygon;
    OGRLinearRing temp_ring;
    for (int iE = 0; iE < edgeCount; iE++)
    {
        fspt_t temp_fspt = area.fspts[iE];
        int edgeId = temp_fspt.rcid;
        int orient = temp_fspt.ornt;
        int usage = temp_fspt.usag;
        int mask = temp_fspt.mask;

        if (m_chart->edge_vectors_map[edgeId].getIsClosedEdge())
        {
            OGRLinearRing colosed_ring;
            std::vector<sg2d_t> sg2ds = m_chart->edge_vectors_map[edgeId].GetSG2Ds();
            if (orient == 2)
            {
                for (int iP = sg2ds.size() - 1; iP >= 0; iP--)
                {
                    colosed_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            else
            {
                for (int iP = 0; iP < sg2ds.size(); iP++)
                {
                    colosed_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            polygon.addRing(&colosed_ring);
        }
        else
        {
            std::vector<sg2d_t> sg2ds = m_chart->edge_vectors_map[edgeId].GetSG2Ds();
            if (orient == 2)
            {
                for (int iP = sg2ds.size() - 1; iP >= 0; iP--)
                {
                    temp_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            else
            {
                for (int iP = 0; iP < sg2ds.size(); iP++)
                {
                    temp_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            if (temp_ring.get_IsClosed() == true)
            {
                polygon.addRing(&temp_ring);
                temp_ring.empty();
            }
        }
    }
    if (temp_ring.IsEmpty() == false && temp_ring.get_IsClosed() == false)
    {
        temp_ring.closeRings();
        polygon.addRing(&temp_ring);
        temp_ring.empty();
    }

    //! 初始化一个gaia的LineString对象
    gaiaLinestringPtr g_linestring = gaiaAddLinestringToGeomColl(geo, 
        polygon.getExteriorRing()->getNumPoints());

    OGRLinearRing* ext_ring = polygon.getExteriorRing();
    for (int iP = 0; iP < ext_ring->getNumPoints(); iP++)
    {
        gaiaSetPoint(g_linestring->Coords, iP, ext_ring->getX(iP), ext_ring->getY(iP));
    }

    /*for (int iR = 0; iR < polygon.getNumInteriorRings(); iR++)
    {
        OGRLinearRing* temp_ring = polygon.getInteriorRing(iR);
        gaiaLinestringPtr temp_linestring = gaiaAddLinestringToGeomColl(geo, temp_ring->getNumPoints());
        for (int iP = 0; iP < temp_ring->getNumPoints(); iP++)
        {
            gaiaSetPoint(temp_linestring->Coords, iP, temp_ring->getX(iP), temp_ring->getY(iP));
        }
    }*/
}

void DBHandler_Spatialite::area_to_spatialite_centroid(AreaFeature& area, gaiaGeomCollPtr geo)
{
    int edgeCount = area.fspts.size();
    int feature_rcid = area.frid.rcid;
    OGRPolygon polygon;
    OGRLinearRing temp_ring;
    for (int iE = 0; iE < edgeCount; iE++)
    {
        fspt_t temp_fspt = area.fspts[iE];
        int edgeId = temp_fspt.rcid;
        int orient = temp_fspt.ornt;
        int usage = temp_fspt.usag;
        int mask = temp_fspt.mask;

        if (m_chart->edge_vectors_map[edgeId].getIsClosedEdge())
        {
            OGRLinearRing colosed_ring;
            std::vector<sg2d_t> sg2ds = m_chart->edge_vectors_map[edgeId].GetSG2Ds();
            if (orient == 2)
            {
                for (int iP = sg2ds.size() - 1; iP >= 0; iP--)
                {
                    colosed_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            else
            {
                for (int iP = 0; iP < sg2ds.size(); iP++)
                {
                    colosed_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            polygon.addRing(&colosed_ring);
        }
        else
        {
            std::vector<sg2d_t> sg2ds = m_chart->edge_vectors_map[edgeId].GetSG2Ds();
            if (orient == 2)
            {
                for (int iP = sg2ds.size() - 1; iP >= 0; iP--)
                {
                    temp_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            else
            {
                for (int iP = 0; iP < sg2ds.size(); iP++)
                {
                    temp_ring.addPoint(sg2ds[iP].long_lat[0], sg2ds[iP].long_lat[1]);
                }
            }
            if (temp_ring.get_IsClosed() == true)
            {
                polygon.addRing(&temp_ring);
                temp_ring.empty();
            }
        }
    }

    OGRPoint p;
    polygon.Centroid(&p);
    gaiaAddPointToGeomColl(geo, p.getX(), p.getY());
}

void DBHandler_Spatialite::line_to_spatialite_line(LineFeature& line, gaiaGeomCollPtr geo)
{
    int lineSegCount = line.GetLineCount();
    for (int iSeg = 0; iSeg < lineSegCount; iSeg++)
    {
        std::vector<sg2d_t> points = line.GetSG2DsVector(iSeg, m_chart->edge_vectors_map);

        gaiaLinestringPtr g_linestring = gaiaAddLinestringToGeomColl(geo, points.size());
        for (int iP = 0; iP < points.size(); iP++)
        {
            gaiaSetPoint(g_linestring->Coords, iP, points[iP].long_lat[0], points[iP].long_lat[1]);
        }
    }
}

void DBHandler_Spatialite::point_to_spatialite_point(PointFeature& point, gaiaGeomCollPtr geo)
{
    int check_flag = point.SetLongLat(m_chart->isolated_node_vectors_map, m_chart->connected_node_vectors_map);
    gaiaAddPointToGeomColl(geo, point.long_lat[0], point.long_lat[1]);
}



void DBHandler_Spatialite::geometry_printout(gaiaGeomCollPtr geom)
{
    /* utility function printing a generic Geometry object */
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    int n_pts = 0;
    int n_lns = 0;
    int n_pgs = 0;
    int cnt;
    int iv;
    int ir;
    double x;
    double y;


    /* we'll now count how many POINTs are there */
    pt = geom->FirstPoint;
    while (pt)
    {
        n_pts++;
        pt = pt->Next;
    }
    /* we'll now count how many LINESTRINGs are there */
    ln = geom->FirstLinestring;
    while (ln)
    {
        n_lns++;
        ln = ln->Next;
    }
    /* we'll now count how many POLYGONs are there */
    pg = geom->FirstPolygon;
    while (pg)
    {
        n_pgs++;
        pg = pg->Next;
    }



    if (n_pts)
    {
        /* printing POINTs coords */
        cnt = 0;
        pt = geom->FirstPoint;
        while (pt)
        {
            /* we'll now scan the linked list of POINTs */
            printf("\t\t\tPOINT %d/%d x=%1.4lf y=%1.4lf\n",
                cnt, n_pts, pt->X, pt->Y);
            cnt++;
            pt = pt->Next;
        }
    }


    if (n_lns)
    {
        /* printing LINESTRINGs coords */
        cnt = 0;
        ln = geom->FirstLinestring;
        while (ln)
        {
            /* we'll now scan the linked list of LINESTRINGs */
            printf("\t\t\tLINESTRING %d/%d has %d vertices\n",
                cnt, n_lns, ln->Points);
            for (iv = 0; iv < ln->Points; iv++)
            {
                /* we'll now retrieve coords for each vertex */
                gaiaGetPoint(ln->Coords, iv, &x, &y);
                printf("\t\t\t\tvertex %d/%d x=%1.4lf y=%1.4lf\n",
                    iv, ln->Points, x, y);
            }
            cnt++;
            ln = ln->Next;
        }
    }


    if (n_pgs)
    {
        /* printing POLYGONs coords */
        cnt = 0;
        pg = geom->FirstPolygon;
        while (pg)
        {
            /* we'll now scan the linked list of POLYGONs */
            printf("\t\t\tPOLYGON %d/%d has %d hole%c\n",
                cnt, n_pgs, pg->NumInteriors,
                (pg->NumInteriors == 1) ? ' ' : 's');

            /*
            now we'll print out the Exterior ring
            [surely a POLYGON has an Exterior ring
            */
            rng = pg->Exterior;
            printf("\t\t\t\tExteriorRing has %d vertices\n", rng->Points);
            for (iv = 0; iv < rng->Points; iv++)
            {
                /* we'll now retrieve coords for each vertex */
                gaiaGetPoint(rng->Coords, iv, &x, &y);
                printf("\t\t\t\t\tvertex %d/%d x=%1.4lf y=%1.4lf\n",
                    iv, rng->Points, x, y);
            }

            for (ir = 0; ir < pg->NumInteriors; ir++)
            {
                /*
                a POLYGON can contain any arbitrary number of Interior rings
                [this including ZERO]
                */
                rng = pg->Interiors + ir;
                printf("\t\t\t\tInteriorRing %d/%d has %d vertices\n",
                    ir, pg->NumInteriors, rng->Points);
                for (iv = 0; iv < rng->Points; iv++)
                {
                    /* we'll now retrieve coords for each vertex */
                    gaiaGetPoint(rng->Coords, iv, &x, &y);
                    printf
                    ("\t\t\t\t\tvertex %d/%d x=%1.4lf y=%1.4lf\n",
                        iv, rng->Points, x, y);
                }
            }

            cnt++;
            pg = pg->Next;
        }
    }
}
