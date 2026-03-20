# -*- mode: python ; coding: utf-8 -*-
# prod_client 打包配置

a = Analysis(
    ['prod_client.py'],
    pathex=[],
    binaries=[],
    datas=[('config.json', '.')],
    hiddenimports=['minio', 'minio.api', 'minio.helpers', 'urllib3', 'certifi',
                   'serial', 'serial.tools', 'serial.tools.list_ports'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=['PySide6', 'PyQt5', 'matplotlib', 'numpy', 'pandas', 'scipy', 'sklearn',
              'Flask', 'SQLAlchemy', 'pyodbc', 'tkinter', 'tkinterdnd2',
              'pyqtgraph', 'hypothesis', 'pytest'],
    noarchive=False,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name='ProdClient',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    console=False,
    icon=None,
)
coll = COLLECT(
    exe,
    a.binaries,
    a.datas,
    strip=False,
    upx=True,
    upx_exclude=[],
    name='ProdClient',
)
