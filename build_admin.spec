# -*- mode: python ; coding: utf-8 -*-
# admin_client 打包配置

a = Analysis(
    ['admin_client.py'],
    pathex=[],
    binaries=[
        ('C:\\Users\\arlag\\AppData\\Local\\Programs\\Python\\Python313\\vcruntime140.dll', '.'),
        ('C:\\Users\\arlag\\AppData\\Local\\Programs\\Python\\Python313\\vcruntime140_1.dll', '.'),
    ],
    datas=[('config.json', '.')],
    hiddenimports=['minio', 'minio.api', 'minio.helpers', 'urllib3', 'certifi'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=['PySide6', 'PyQt5', 'matplotlib', 'numpy', 'pandas', 'scipy', 'sklearn',
              'Flask', 'SQLAlchemy', 'pyserial', 'pyodbc', 'tkinter', 'tkinterdnd2',
              'pyqtgraph', 'hypothesis', 'pytest'],
    noarchive=False,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name='AdminClient',
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
    name='AdminClient',
)
