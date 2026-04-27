# import _enc2db_core

# _enc2db_core.process_charts(
#     db_path="test_output.db",
#     chart_files=["D:\\data\\US5VA27M.000"]
# )

from enc2db import process

process(
    db_path="test_output.db",
    chart_files=["D:\\data\\US5VA27M.000"]
)