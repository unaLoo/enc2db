#include "compat.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <exception>

#include "MyChartLoader2.h"
#include "DBHandler_Spatialite.h"

// 引入 pybind11
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // 实现 std::vector <-> Python list 的自动转换

namespace py = pybind11;

bool folderExists(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR) != 0; // 判断是否是目录
}
bool fileExists(const std::string &path)
{
    FILE *f = fopen(path.c_str(), "rb");
    if (f)
    {
        fclose(f);
        return true;
    }
    return false;
}

// 注意：这里增加了一个 resourceFolder 参数，不再使用硬编码
void processCharts(const std::string &dbPath, const std::vector<std::string> &chartFiles, const std::string &resourceFolder)
{
    try
    {
        DBHandler_Spatialite db_h;

        printf("[DEBUG] Preparing spatial database: %s\n", dbPath.c_str());
        db_h.prepare_spatial_database(dbPath.c_str());

        // 检查资源文件夹
        if (!folderExists(resourceFolder))
        {
            printf("[ERROR] resourceFolder does not exist: %s\n", resourceFolder.c_str());
            return;
        }

        for (size_t i = 0; i < chartFiles.size(); ++i)
        {
            const std::string &openFileName = chartFiles[i];

            if (!fileExists(openFileName))
            {
                printf("[ERROR] Chart file does not exist: %s\n", openFileName.c_str());
                continue;
            }

            printf("[DEBUG] Loading chart file (%zu/%zu): %s\n", i + 1, chartFiles.size(), openFileName.c_str());

            MyChartLoader myChartLoader1;

            try
            {
                printf("[DEBUG] LoadCodeNameSystem...\n");
                myChartLoader1.LoadCodeNameSystem(resourceFolder);

                printf("[DEBUG] S57Load...\n");
                myChartLoader1.S57Load(openFileName.c_str());

                printf("[DEBUG] SetupFeatures...\n");
                myChartLoader1.SetupFeatures();

                int level = myChartLoader1.GetSuitableLevel();
                myChartLoader1.m_cscl_level = 10;
                printf("[DEBUG] Chart level\n");

                printf("[DEBUG] Binding chart loader to DB...\n");
                db_h.bind_chart_loader(&myChartLoader1);

                printf("[DEBUG] Preparing chart tables...\n");
                db_h.prepare_chart_tables();

                printf("[DEBUG] Processing area common a...\n");
                db_h.process_area_common_a();
                printf("[DEBUG] Processing area common l...\n");
                db_h.process_area_common_l();
                printf("[DEBUG] Processing area common m...\n");
                db_h.process_area_common_m();
                printf("[DEBUG] Processing area customize...\n");
                db_h.process_area_customize();

                printf("[DEBUG] Processing line common ML...\n");
                db_h.process_line_common_ml();
                printf("[DEBUG] Processing point common icon...\n");
                db_h.process_point_common_icon();
                printf("[DEBUG] Processing point SOUNDG...\n");
                db_h.process_point_SOUNDG();

                printf("[DEBUG] Closing database...\n");
                db_h.close_database();
            }
            catch (const std::exception &e)
            {
                printf("[EXCEPTION] std::exception caught: %s\n", e.what());
            }
            catch (...)
            {
                printf("[EXCEPTION] Unknown exception caught while processing file: %s\n", openFileName.c_str());
            }
        }
    }
    catch (const std::exception &e)
    {
        printf("[EXCEPTION] std::exception caught in processCharts: %s\n", e.what());
    }
    catch (...)
    {
        printf("[EXCEPTION] Unknown exception caught in processCharts\n");
    }
}

// ---------------------------------------------------------
// Pybind11 模块定义
// 模块名 _enc2db_core 必须与 CMakeLists.txt 中的 pybind11_add_module 保持一致
// ---------------------------------------------------------
PYBIND11_MODULE(_enc2db_core, m)
{
    m.doc() = "C++ Core for S-57 ENC to Vector DB translation";

    // 绑定 processCharts 函数
    m.def("process_charts", &processCharts,
          "Parse S-57 files and output to a SpatiaLite database",
          py::arg("db_path"),
          py::arg("chart_files"),
          py::arg("resource_folder") = ""); // 提供一个默认值以防万一，但建议在 Python 侧显式传入
}