# enc2db/__init__.py
import os
from ._enc2db_core import process_charts as _process_charts

def get_resource_folder() -> str:
    """获取打包进 wheel 的 resource 目录路径"""
    return os.path.join(os.path.dirname(__file__), "resource") + os.sep

def process(db_path: str, chart_files: list):

    resource_folder = get_resource_folder()
    print("[DEBUG] ", resource_folder)
    _process_charts(db_path, chart_files, resource_folder)

__all__ = ["process"]