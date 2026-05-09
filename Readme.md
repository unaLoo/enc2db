# Enc2db
S-57 (.000) file parsing and Spatialite DB translation tool

# Usage

```bash
pip install enc2db
```

```python
from enc2db import process

process(
    db_path="enc.sqlite",
    chart_files=["./data/test.000"]
)
```