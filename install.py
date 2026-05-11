import os
import shutil
from pathlib import Path

SOURCE_DIR = Path("source")
INSTALL_DIR = Path("/usr/include/Arkas")

# Create installation root safely
INSTALL_DIR.mkdir(parents=True, exist_ok=True)

for src_path in SOURCE_DIR.rglob("*.h"):
    # Compute relative path inside source/
    relative_path = src_path.relative_to(SOURCE_DIR)

    # Destination path
    dest_path = INSTALL_DIR / relative_path

    # Ensure parent directories exist
    dest_path.parent.mkdir(parents=True, exist_ok=True)

    # Copy file with metadata
    shutil.copy2(src_path, dest_path)

    print(f"Installed: {dest_path}")
