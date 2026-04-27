#include "compat.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <exception>

#include "MyChartLoader2.h"
#include "DBHandler_Spatialite.h"


bool fileExists(const std::string& path)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

void processCharts(const std::string& dbPath, const std::vector<std::string>& chartFiles)
{
    try {
        DBHandler_Spatialite db_h;

        printf("[DEBUG] Preparing spatial database: %s\n", dbPath.c_str());
        db_h.prepare_spatial_database(dbPath.c_str());

        std::string resourceFolder = "/home/dev/data/";
        if (!fileExists(resourceFolder)) {
            printf("[ERROR] resourceFolder does not exist: %s\n", resourceFolder.c_str());
            return;
        }

        for (size_t i = 0; i < chartFiles.size(); ++i)
        {
            const std::string& openFileName = chartFiles[i];

            // 检查文件是否存在
            if (!fileExists(openFileName)) {
                printf("[ERROR] Chart file does not exist: %s\n", openFileName.c_str());
                continue;
            }

            printf("[DEBUG] Loading chart file (%zu/%zu): %s\n", i + 1, chartFiles.size(), openFileName.c_str());

            MyChartLoader myChartLoader1;

            try {
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
            catch (const std::exception& e) {
                printf("[EXCEPTION] std::exception caught: %s\n", e.what());
            }
            catch (...) {
                printf("[EXCEPTION] Unknown exception caught while processing file: %s\n", openFileName.c_str());
            }
        }
    }
    catch (const std::exception& e) {
        printf("[EXCEPTION] std::exception caught in processCharts: %s\n", e.what());
    }
    catch (...) {
        printf("[EXCEPTION] Unknown exception caught in processCharts\n");
    }
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("Usage: %s <output_db_path> <chart_file1.000> [chart_file2.000 ...]\n", argv[0]);
        return 1;
    }

    std::string dbPath = argv[1];
    std::vector<std::string> chartFiles;
    for (int i = 2; i < argc; ++i) {
        chartFiles.push_back(argv[i]);
    }

    printf("===========Start============\n");
    processCharts(dbPath, chartFiles);
    printf("==========FINISH============\n");

    return 0;
}