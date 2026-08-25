import sys
import os
import json
import io
import time
import serial
import serial.tools.list_ports
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QLineEdit, QPushButton, QTableWidget,
                             QTableWidgetItem, QHeaderView, QSpinBox, QLabel,
                             QMessageBox, QTabWidget, QComboBox, QFileDialog,
                             QPlainTextEdit, QCheckBox, QGroupBox)
from PyQt6.QtCore import Qt, QThread, pyqtSignal
from PyQt6.QtGui import QFont

# ============================================================
# Frontend-Design 规范 - 现代工业深色控制台 UI 样式表 (QSS)
# ============================================================
MODERN_INDUSTRIAL_QSS = """
/* 全局窗口基础设置 */
QMainWindow, QDialog {
    background-color: #0f172a;
    color: #f8fafc;
    font-family: 'Segoe UI', 'PingFang SC', 'Microsoft YaHei', sans-serif;
    font-size: 13px;
}

QWidget {
    color: #cbd5e1;
    font-family: 'Segoe UI', 'PingFang SC', 'Microsoft YaHei', sans-serif;
}

/* Tab 选项卡组 */
QTabWidget::pane {
    border: 1px solid #334155;
    border-radius: 8px;
    background-color: #1e293b;
    top: -1px;
}

QTabBar::tab {
    background-color: #0f172a;
    color: #94a3b8;
    border: 1px solid #334155;
    border-bottom: none;
    border-top-left-radius: 6px;
    border-top-right-radius: 6px;
    padding: 8px 18px;
    margin-right: 4px;
    font-weight: 600;
}

QTabBar::tab:hover {
    background-color: #1e293b;
    color: #38bdf8;
}

QTabBar::tab:selected {
    background-color: #1e293b;
    color: #f8fafc;
    border-top: 3px solid #38bdf8;
    border-left: 1px solid #334155;
    border-right: 1px solid #334155;
}

/* 分组框 QGroupBox */
QGroupBox {
    background-color: #1e293b;
    border: 1px solid #334155;
    border-radius: 8px;
    margin-top: 1.2em;
    padding-top: 14px;
    font-weight: bold;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 12px;
    padding: 2px 8px;
    background-color: #0f172a;
    color: #38bdf8;
    border: 1px solid #334155;
    border-radius: 4px;
}

/* 输入框 & 下拉菜单 */
QLineEdit, QSpinBox, QComboBox {
    background-color: #0f172a;
    border: 1px solid #334155;
    border-radius: 6px;
    padding: 6px 12px;
    color: #f8fafc;
    selection-background-color: #0284c7;
    selection-color: #ffffff;
}

QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
    border: 1px solid #38bdf8;
    background-color: #111c33;
}

QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: top right;
    width: 20px;
    border-left-width: 0px;
}

QComboBox QAbstractItemView {
    background-color: #1e293b;
    border: 1px solid #38bdf8;
    selection-background-color: #0284c7;
    color: #f8fafc;
    padding: 4px;
}

/* 按钮通用样式 */
QPushButton {
    background-color: #2563eb;
    color: #ffffff;
    border: none;
    border-radius: 6px;
    padding: 7px 16px;
    font-weight: 600;
}

QPushButton:hover {
    background-color: #3b82f6;
}

QPushButton:pressed {
    background-color: #1d4ed8;
}

QPushButton:disabled {
    background-color: #334155;
    color: #64748b;
}

/* 表格控件 QTableWidget */
QTableWidget {
    background-color: #0f172a;
    border: 1px solid #334155;
    gridline-color: #1e293b;
    border-radius: 6px;
    color: #f8fafc;
}

QTableWidget::item {
    padding: 6px;
}

QTableWidget::item:alternate {
    background-color: #162032;
}

QTableWidget::item:selected {
    background-color: #1e3a8a;
    color: #38bdf8;
}

QHeaderView::section {
    background-color: #1e293b;
    color: #38bdf8;
    padding: 8px;
    border: none;
    border-right: 1px solid #334155;
    border-bottom: 2px solid #0284c7;
    font-weight: bold;
}

/* 文本编辑器 / 日志监控框 */
QPlainTextEdit {
    background-color: #090d16;
    border: 1px solid #334155;
    border-radius: 6px;
    color: #f8fafc;
    padding: 8px;
    font-family: 'Consolas', 'Courier New', monospace;
}

/* 复选框 QCheckBox */
QCheckBox {
    spacing: 8px;
    color: #f8fafc;
}

QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border-radius: 4px;
    border: 1px solid #475569;
    background-color: #0f172a;
}

QCheckBox::indicator:checked {
    background-color: #0284c7;
    border: 1px solid #38bdf8;
}

/* 滚动条 */
QScrollBar:vertical {
    border: none;
    background-color: #0f172a;
    width: 10px;
    border-radius: 5px;
}

QScrollBar::handle:vertical {
    background-color: #334155;
    border-radius: 5px;
    min-height: 20px;
}

QScrollBar::handle:vertical:hover {
    background-color: #475569;
}
"""

from minio_db import MinioWarehouse
from tv_protocol import TVSerialProtocol


def get_app_dir():
    """获取应用所在目录（兼容 pyinstaller 打包后的路径）"""
    if getattr(sys, 'frozen', False):
        return os.path.dirname(sys.executable)
    return os.path.dirname(os.path.abspath(__file__))

def load_config():
    """加载本地配置文件，支持 platforms 分层结构并自动兼容旧版单层 key_types"""
    default_config = {
        "platforms": {
            "MTK": {
                "key_types": [
                    "HDCP1.4 5586 dev", "HDCP2.2 5586 dev",
                    "HDCP1.4 5586 prod", "HDCP2.2 5586 prod",
                    "ULPK 5586F dev", "ULPK 5586F prod",
                    "ULPK 5586L dev", "ULPK 5586L prod"
                ]
            },
            "Novatek": {
                "key_types": [
                    "HDCP1.4 NTK dev", "HDCP2.2 NTK dev",
                    "HDCP1.4 NTK prod", "HDCP2.2 NTK prod",
                    "ULPK NTK dev", "ULPK NTK prod"
                ]
            }
        },
        "default_platform": "MTK",
        "mac_clients": ["Vizio", "Onn"],
        "minio": {"endpoint": "172.16.9.28:9000", "access_key": "admin",
                  "secret_key": "12345678", "bucket": "warehouse"}
    }
    config_path = os.path.join(get_app_dir(), "config.json")
    if os.path.exists(config_path):
        try:
            with open(config_path, "r", encoding="utf-8") as f:
                loaded = json.load(f)
                # 兼容旧版本：若存在单层 key_types 且无 platforms，自动封装到 MTK 平台下
                if "platforms" not in loaded and "key_types" in loaded:
                    loaded["platforms"] = {
                        "MTK": {"key_types": loaded.pop("key_types")}
                    }
                    if "default_platform" not in loaded:
                        loaded["default_platform"] = "MTK"
                return loaded
        except Exception as e:
            print(f"[WARN] 加载 config.json 失败: {e}，将使用默认配置")
            return default_config
    return default_config


def get_platforms(config):
    """获取所有已配置的芯片方案名称列表"""
    if "platforms" in config and isinstance(config["platforms"], dict):
        return list(config["platforms"].keys())
    return ["MTK"]


def get_platform_key_types(config, platform=None):
    """获取指定方案的 Key 类型列表；若未指定则返回所有方案的 Key 类型合集"""
    if "platforms" in config and isinstance(config["platforms"], dict):
        if platform and platform in config["platforms"]:
            return config["platforms"][platform].get("key_types", [])
        # 未指定方案时，返回全部方案的去重 Key 列表
        all_keys = []
        for p_data in config["platforms"].values():
            for kt in p_data.get("key_types", []):
                if kt not in all_keys:
                    all_keys.append(kt)
        return all_keys
    # 兼容旧版单一 key_types 列表
    return config.get("key_types", [])


def get_default_platform(config):
    """获取默认芯片方案"""
    default_p = config.get("default_platform")
    platforms = get_platforms(config)
    if default_p in platforms:
        return default_p
    return platforms[0] if platforms else "MTK"


def get_key_platform(config, key_type):
    """根据 Key 类型反查所属的芯片方案名称（如 MTK / Novatek / 通用）"""
    if "platforms" in config and isinstance(config["platforms"], dict):
        for p_name, p_data in config["platforms"].items():
            if key_type in p_data.get("key_types", []):
                return p_name
    # 启发式后备判断
    kt_upper = key_type.upper()
    if "5586" in kt_upper or "MTK" in kt_upper:
        return "MTK"
    elif "NTK" in kt_upper or "NOVATEK" in kt_upper or "NOVATECH" in kt_upper:
        return "Novatek"
    return "-"



# ============================================================
# 烧录工作线程（支持多平台方案识别）
# ============================================================
class BurnWorker(QThread):
    log_signal = pyqtSignal(str, str)
    result_signal = pyqtSignal(bool, str)

    def __init__(self, db, protocol, sn, task_list, config, platform="MTK"):
        super().__init__()
        self.db = db
        self.protocol = protocol
        self.sn = sn
        self.task_list = task_list
        self.config = config
        self.platform = platform
        self.monitor_signal = None

    def run(self):
        try:
            # 检查网络连接
            if not self.db.check_network_connection():
                self.result_signal.emit(False, "网络连接失败：无法连接到MinIO服务器，请检查网络连接。")
                return

            self.log_signal.emit(f"--- [{self.platform}] 全功能烧录启动: SN {self.sn} ---", "#ffffff")
            # 进入工厂模式
            fac_cmd = self.protocol.pack_factory_mode()
            self.protocol.send_and_wait_ack(fac_cmd, monitor_signal=self.monitor_signal,
                                            log_signal=self.log_signal,
                                            max_retries=5, ack_delay=0.3)
            time.sleep(0.5)
            burn_history = {}
            # 记录待移动的资源: [(task_path, res_id), ...]
            pending_moves = []
            success = False

            for task_path in self.task_list:
                # 对于 key/ULPK xxx/client 路径，类型名是中间部分而非最后一段
                parts = task_path.split('/')
                if parts[0] == "mac":
                    cmd_type = parts[-1]  # 客户名，如 onn
                elif len(parts) >= 3 and "ULPK" in parts[1].upper():
                    cmd_type = parts[1]   # 如 "ULPK 5586L prod" 或 "ULPK NTK prod"
                else:
                    cmd_type = parts[1] if len(parts) >= 2 else parts[-1]

                if task_path.startswith("mac/"):
                    res_id = self.db.peek_available(task_path)
                    if not res_id:
                        self.result_signal.emit(False, f"库存空: {task_path}")
                        return
                    mac_label = f"MAC({cmd_type})"
                    self.log_signal.emit(f"[{mac_label}] 资源: {res_id}", "#3498db")
                    cmd = self.protocol.pack_mac_command(res_id)
                    self.log_signal.emit(f"[{mac_label}] 发送: {cmd.hex(' ').upper()}", "#9b59b6")
                    ok, ack, msg = self.protocol.send_and_wait_ack(
                        cmd, monitor_signal=self.monitor_signal, log_signal=self.log_signal,
                        max_retries=10, ack_delay=0.5)
                    if not ok:
                        self.log_signal.emit(f"[{mac_label}] 失败详情: ack={ack}, raw={msg}", "#e74c3c")
                    success = ok

                elif "HDCP" in cmd_type.upper():
                    res_id = self.db.peek_available(task_path)
                    if not res_id:
                        self.result_signal.emit(False, f"库存空: {task_path}")
                        return
                    raw_data = self._read_minio_binary(task_path, res_id)
                    hdcp_type = 0xE3 if "1.4" in cmd_type else 0xE4
                    success = self.process_hdcp_burn(cmd_type, hdcp_type, raw_data)
                    if not success:
                        return

                elif "ULPK" in cmd_type.upper():
                    res_id = self.db.peek_available(task_path)
                    if not res_id:
                        self.result_signal.emit(False, f"库存空: {task_path}")
                        return
                    raw_data = self._read_minio_binary(task_path, res_id)
                    uid = self.extract_uid(res_id)
                    cmd = self.protocol.pack_ulpk_command(uid, raw_data)
                    self.log_signal.emit(f"[ULPK] UID={uid}, 帧长={len(cmd)} 字节", "#3498db")
                    self.log_signal.emit(f"[ULPK] HEX: {cmd.hex(' ').upper()}", "#9b59b6")
                    ok, ack, msg = self.protocol.send_and_wait_ack(
                        cmd, monitor_signal=self.monitor_signal, log_signal=self.log_signal,
                        max_retries=10, ack_delay=1.0)
                    if not ok:
                        self.result_signal.emit(False, f"{cmd_type} ULPK 烧录失败: {msg}")
                        return
                    success = True
                else:
                    self.log_signal.emit(f"未知任务类型: {cmd_type}", "#e74c3c")
                    continue

                if not success:
                    self.result_signal.emit(False, f"{cmd_type} 烧录失败")
                    return
                burn_history[cmd_type] = res_id
                pending_moves.append((task_path, res_id))
                self.log_signal.emit(f"{cmd_type} 烧录成功 ✅", "#2ecc71")

            # 烧录 SN
            self.log_signal.emit(f"[SN] 正在写入 SN: {self.sn}", "#3498db")
            sn_cmd = self.protocol.pack_sn_command(self.sn)
            self.log_signal.emit(f"[SN] 发送: {sn_cmd.hex(' ').upper()}", "#9b59b6")
            sn_ok, sn_ack, sn_msg = self.protocol.send_and_wait_ack(
                sn_cmd, monitor_signal=self.monitor_signal, log_signal=self.log_signal,
                max_retries=5, ack_delay=0.5)
            if not sn_ok:
                self.result_signal.emit(False, f"SN 写入失败: {sn_msg}")
                return
            burn_history["SN"] = self.sn
            self.log_signal.emit("SN 写入成功 ✅", "#2ecc71")

            # 全部烧录成功，统一将资源从 available 移到 used
            self.log_signal.emit("正在标记资源为已使用...", "#f39c12")
            for task_path, res_id in pending_moves:
                self.db.move_to_used(task_path, res_id)

            # 归档烧录记录（包含所属方案信息）
            record_data = json.dumps({
                "sn": self.sn,
                "platform": self.platform,
                "burn_results": burn_history
            }, ensure_ascii=False).encode('utf-8')
            record_path = f"sn_record/{self.sn}.json"
            self.db.client.put_object(
                self.db.bucket, record_path,
                io.BytesIO(record_data), len(record_data),
                content_type="application/json"
            )
            self.result_signal.emit(True, "所有任务已完成")
        except Exception as e:
            self.result_signal.emit(False, f"致命错误: {str(e)}")

    def _read_minio_binary(self, task_path, res_id):
        response = self.db.client.get_object(self.db.bucket, f"{task_path}/available/{res_id}")
        raw_data = response.read()
        response.close()
        response.release_conn()
        self.log_signal.emit(f"[MinIO] 资源: {res_id}, 大小: {len(raw_data)} 字节", "#3498db")
        self.log_signal.emit(f"[MinIO] HEX: {raw_data.hex(' ').upper()}", "#9b59b6")
        return raw_data

    def process_hdcp_burn(self, name, type_code, data):
        if type_code == 0xE3:
            block_size = self.protocol.HDCP14_BLOCK_SIZE
            if len(data) == 512:
                data = data[:304]
            total_blocks = (len(data) + block_size - 1) // block_size
        else:
            block_size = self.protocol.HDCP22_BLOCK_SIZE
            # key 不足 1044 字节时补零，超过则截取
            if len(data) < self.protocol.HDCP22_TOTAL_SIZE:
                data = data + b'\x00' * (self.protocol.HDCP22_TOTAL_SIZE - len(data))
            else:
                data = data[:self.protocol.HDCP22_TOTAL_SIZE]
            total_blocks = self.protocol.HDCP22_BLOCK_NO

        full_crc = self.protocol.calculate_crc(data)
        header_cmd = self.protocol.pack_hdcp_header(type_code, total_blocks, block_size, full_crc)
        self.log_signal.emit(f"[{name}] Header: blocks={total_blocks}, size={block_size}, crc=0x{full_crc:04X}", "#3498db")
        self.log_signal.emit(f"[{name}] Header HEX: {header_cmd.hex(' ').upper()}", "#9b59b6")

        ok, ack, msg = self.protocol.send_and_wait_ack(
            header_cmd, monitor_signal=self.monitor_signal, log_signal=self.log_signal,
            max_retries=10, ack_delay=0.3)
        if not ok:
            self.result_signal.emit(False, f"{name} Header 失败: {msg}")
            return False

        for b_id in range(1, total_blocks + 1):
            start = (b_id - 1) * block_size
            end = len(data) if b_id == total_blocks else start + block_size
            chunk = data[start:end]
            chunk_cmd = self.protocol.pack_hdcp_chunk(type_code, b_id, chunk)
            self.log_signal.emit(f"[{name}] 分包 {b_id}/{total_blocks}, {len(chunk)} 字节", "#3498db")
            self.log_signal.emit(f"[{name}] HEX: {chunk_cmd.hex(' ').upper()}", "#9b59b6")
            # 包间延迟，与 MFC Timer 300ms 轮询节奏一致
            time.sleep(0.1)
            ok, ack, msg = self.protocol.send_and_wait_ack(
                chunk_cmd, monitor_signal=self.monitor_signal, log_signal=self.log_signal,
                max_retries=10, ack_delay=0.3)
            if not ok:
                self.result_signal.emit(False, f"{name} 分包 {b_id} 失败: {msg}")
                return False

        self.log_signal.emit(f"[{name}] 烧录完成, 最终 ACK: {ack}", "#2ecc71")
        return True

    def extract_uid(self, filename):
        try:
            name_part = filename.replace(".json", "").replace(".dat", "").replace(".bin", "")
            if "-" in name_part:
                return name_part.split("-")[-1]
            return name_part
        except:
            return "0"


class SerialReaderThread(QThread):
    """后台串口持续读取线程"""
    data_received = pyqtSignal(bytes)

    def __init__(self, serial_obj):
        super().__init__()
        self.serial_obj = serial_obj
        self._running = True

    def run(self):
        while self._running:
            try:
                if self.serial_obj and self.serial_obj.is_open:
                    if self.serial_obj.in_waiting > 0:
                        data = self.serial_obj.read(self.serial_obj.in_waiting)
                        if data:
                            self.data_received.emit(data)
                time.sleep(0.05)
            except Exception:
                break

    def stop(self):
        self._running = False
        self.wait(1000)


# ============================================================
# 合并后的主窗口
# ============================================================
class MainWindow(QMainWindow):
    # 串口监控信号
    monitor_data_signal = pyqtSignal(bytes)

    def __init__(self):
        super().__init__()
        self.config = load_config()
        self.setWindowTitle("Key/MAC 烧录管理系统 - v4.4")
        self.resize(1100, 800)

        # 初始化 MinIO
        m_cfg = self.config.get("minio", {})
        self.db = MinioWarehouse(
            endpoint=m_cfg.get("endpoint", "127.0.0.1:9000"),
            access_key=m_cfg.get("access_key", "minioadmin"),
            secret_key=m_cfg.get("secret_key", "minioadmin"),
            bucket=m_cfg.get("bucket", "warehouse")
        )

        # 初始化网络状态为未知，稍后异步检查
        self.network_status = None

        # 初始化串口协议
        self.protocol = TVSerialProtocol()
        self.serial_reader = None
        self.monitor_data_signal.connect(self._update_serial_monitor)

        self._init_ui()

    def _init_ui(self):
        central = QWidget()
        self.setCentralWidget(central)
        layout = QVBoxLayout(central)

        self.tabs = QTabWidget()
        self.tabs.addTab(self._create_burn_tab(), "烧录工具")
        self.tabs.addTab(self._create_manual_record_tab(), "手动录入")
        self.tabs.addTab(self._create_import_tab(), "资源导入")
        self.tabs.addTab(self._create_view_tab(), "库存查询")
        self.tabs.addTab(self._create_trace_tab(), "SN 追溯")
        layout.addWidget(self.tabs)

        # 添加状态栏
        self.status_bar = QLabel()
        self._update_network_status()
        layout.addWidget(self.status_bar)

    # ==================== Tab 1: 资源导入 ====================
    def _create_import_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)

        layout.addWidget(QLabel("<b>[1] MAC 地址段生成</b>"))
        mac_client_layout = QHBoxLayout()
        mac_client_layout.addWidget(QLabel("客户:"))
        self.mac_client_select = QComboBox()
        self.mac_client_select.addItems(self.config.get("mac_clients", ["Vizio", "Onn"]))
        mac_client_layout.addWidget(self.mac_client_select, 1)
        layout.addLayout(mac_client_layout)

        mac_layout = QHBoxLayout()
        self.start_mac_input = QLineEdit()
        self.start_mac_input.setPlaceholderText("起始 MAC (支持 : 或 -)")
        self.mac_count_input = QSpinBox()
        self.mac_count_input.setRange(1, 5000)
        self.mac_count_input.setValue(10)
        btn_mac = QPushButton("检查并上传 MAC")
        btn_mac.clicked.connect(self._handle_batch_mac_upload)
        mac_layout.addWidget(self.start_mac_input)
        mac_layout.addWidget(QLabel("数量:"))
        mac_layout.addWidget(self.mac_count_input)
        mac_layout.addWidget(btn_mac)
        layout.addLayout(mac_layout)

        layout.addSpacing(30)
        layout.setContentsMargins(20, 20, 20, 20)

        layout.addWidget(QLabel("<b>[2] Key 资源上传</b>"))
        import_platform_layout = QHBoxLayout()
        import_platform_layout.addWidget(QLabel("芯片方案:"))
        self.import_platform_select = QComboBox()
        self.import_platform_select.addItems(get_platforms(self.config))
        self.import_platform_select.setCurrentText(get_default_platform(self.config))
        self.import_platform_select.currentTextChanged.connect(self._on_import_platform_changed)
        import_platform_layout.addWidget(self.import_platform_select, 1)
        layout.addLayout(import_platform_layout)

        type_select_layout = QHBoxLayout()
        type_select_layout.addWidget(QLabel("Key 类型:"))
        self.key_type_select = QComboBox()
        self.key_type_select.addItems(get_platform_key_types(self.config, get_default_platform(self.config)))
        self.key_type_select.setEditable(True)
        type_select_layout.addWidget(self.key_type_select, 1)
        layout.addLayout(type_select_layout)

        key_client_layout = QHBoxLayout()
        key_client_layout.addWidget(QLabel("客户 (ULPK):"))
        self.key_client_select = QComboBox()
        self.key_client_select.addItems(self.config.get("mac_clients", ["Vizio", "Onn"]))
        key_client_layout.addWidget(self.key_client_select, 1)
        layout.addLayout(key_client_layout)

        key_btn_layout = QHBoxLayout()
        btn_single = QPushButton("选择单个文件上传")
        btn_single.clicked.connect(lambda: self._handle_key_upload(is_batch=False))
        btn_batch = QPushButton("选择文件夹批量上传")
        btn_batch.clicked.connect(lambda: self._handle_key_upload(is_batch=True))
        key_btn_layout.addWidget(btn_single)
        key_btn_layout.addWidget(btn_batch)
        layout.addLayout(key_btn_layout)

        layout.addStretch()

        # 网络状态检查
        network_layout = QHBoxLayout()
        network_layout.addWidget(QLabel("网络状态:"))
        self.network_check_btn = QPushButton("检查连接")
        self.network_check_btn.clicked.connect(self._check_network_status)
        network_layout.addWidget(self.network_check_btn)
        network_layout.addStretch()
        layout.addLayout(network_layout)

        return widget
    def _create_manual_record_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(20, 20, 20, 20)

        layout.addWidget(QLabel("<b>手动录入已使用的 SN / MAC / Key 记录</b>"))
        layout.addWidget(QLabel("适用于线下已烧录但系统中未记录的情况，提交后会将对应资源标记为已使用并生成归档记录。"))
        layout.addSpacing(10)

        # SN 输入
        sn_row = QHBoxLayout()
        sn_row.addWidget(QLabel("SN 序列号:"))
        self.manual_sn = QLineEdit()
        self.manual_sn.setPlaceholderText("必填，机器 SN")
        sn_row.addWidget(self.manual_sn, 1)
        layout.addLayout(sn_row)

        # 客户选择
        client_row = QHBoxLayout()
        client_row.addWidget(QLabel("客户:"))
        self.manual_client = QComboBox()
        self.manual_client.addItems(self.config.get("mac_clients", ["Vizio", "Onn"]))
        client_row.addWidget(self.manual_client, 1)
        client_row.addStretch(2)
        layout.addLayout(client_row)

        # MAC 输入
        mac_row = QHBoxLayout()
        mac_row.addWidget(QLabel("MAC 地址:"))
        self.manual_mac = QLineEdit()
        self.manual_mac.setPlaceholderText("选填，如 AA-BB-CC-DD-EE-FF")
        mac_row.addWidget(self.manual_mac, 1)
        layout.addLayout(mac_row)

        # Key 条目列表
        layout.addSpacing(10)
        layout.addWidget(QLabel("<b>Key 资源 (选填，可添加多条):</b>"))

        self.manual_key_rows = []
        key_list_widget = QWidget()
        self.manual_key_list_layout = QVBoxLayout(key_list_widget)
        self.manual_key_list_layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(key_list_widget)

        btn_add_key = QPushButton("+ 添加一条 Key 记录")
        btn_add_key.clicked.connect(self._add_manual_key_row)
        layout.addWidget(btn_add_key)

        # 提交按钮
        layout.addSpacing(20)
        btn_submit = QPushButton("提交录入")
        btn_submit.setFixedHeight(45)
        btn_submit.setStyleSheet("background-color: #e67e22; color: white; font-weight: bold; font-size: 14px;")
        btn_submit.clicked.connect(self._submit_manual_record)
        layout.addWidget(btn_submit)

        # 结果提示
        self.manual_result_label = QLabel("")
        layout.addWidget(self.manual_result_label)

        layout.addStretch()
        return widget

    def _add_manual_key_row(self):
        """添加一行 Key 录入"""
        row_widget = QWidget()
        row_layout = QHBoxLayout(row_widget)
        row_layout.setContentsMargins(0, 2, 0, 2)

        type_combo = QComboBox()
        type_combo.addItems(get_platform_key_types(self.config))
        type_combo.setEditable(True)
        res_id_input = QLineEdit()
        res_id_input.setPlaceholderText("资源文件名 / ID")
        btn_remove = QPushButton("删除")
        btn_remove.setFixedWidth(60)

        row_layout.addWidget(QLabel("类型:"))
        row_layout.addWidget(type_combo, 1)
        row_layout.addWidget(QLabel("资源ID:"))
        row_layout.addWidget(res_id_input, 1)
        row_layout.addWidget(btn_remove)

        entry = {"widget": row_widget, "type": type_combo, "id": res_id_input}
        self.manual_key_rows.append(entry)
        self.manual_key_list_layout.addWidget(row_widget)

        btn_remove.clicked.connect(lambda: self._remove_manual_key_row(entry))

    def _remove_manual_key_row(self, entry):
        if entry in self.manual_key_rows:
            self.manual_key_rows.remove(entry)
            entry["widget"].setParent(None)
            entry["widget"].deleteLater()

    def _submit_manual_record(self):
        """提交手动录入记录"""
        # 检查网络连接
        if not self.db.check_network_connection():
            QMessageBox.critical(self, "网络错误", "无法连接到MinIO服务器！\n请检查网络连接后重试。")
            return

        sn = self.manual_sn.text().strip()
        if not sn:
            QMessageBox.warning(self, "提示", "请输入 SN 序列号！")
            return

        # 检查 SN 是否已使用
        if self._check_sn_exists(sn):
            ret = QMessageBox.warning(self, "SN 重复",
                f"SN [{sn}] 已存在烧录记录！\n继续将覆盖原有记录，是否继续？",
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.No)
            if ret != QMessageBox.StandardButton.Yes:
                return

        client_name = self.manual_client.currentText().strip().lower()
        mac_str = self.manual_mac.text().strip()
        burn_results = {}
        errors = []

        # 处理 MAC
        if mac_str:
            try:
                clean = mac_str.replace(":", "").replace("-", "").replace(" ", "").upper()
                if len(clean) != 12:
                    raise ValueError("MAC 必须为 12 位十六进制")
                std_mac = "-".join(clean[i:i+2] for i in range(0, 12, 2))
                mac_path = f"mac/{client_name}"
                # 尝试从 available 移到 used
                moved = self._try_move_to_used(mac_path, std_mac + ".json")
                if moved:
                    burn_results[client_name] = std_mac
                else:
                    # 资源不在 available 中，仍然记录但提示
                    burn_results[client_name] = std_mac
                    errors.append(f"MAC {std_mac} 未在库存 available 中找到，已直接记录")
            except Exception as e:
                errors.append(f"MAC 格式错误: {e}")

        # 处理 Key 列表
        for entry in self.manual_key_rows:
            key_type = entry["type"].currentText().strip()
            res_id = entry["id"].text().strip()
            if not key_type or not res_id:
                continue
            if "ULPK" in key_type.upper():
                key_path = f"key/{key_type}/{client_name}"
            else:
                key_path = f"key/{key_type}"
            moved = self._try_move_to_used(key_path, res_id)
            if not moved:
                errors.append(f"Key [{key_type}] {res_id} 未在 available 中找到，已直接记录")
            burn_results[key_type] = res_id

        if not burn_results:
            # SN 是唯一必填项，MAC/Key 都是选填
            pass

        # 写入 sn_record
        burn_results["SN"] = sn
        record_data = json.dumps({"sn": sn, "burn_results": burn_results}).encode('utf-8')
        record_path = f"sn_record/{sn}.json"
        try:
            self.db.client.put_object(
                self.db.bucket, record_path,
                io.BytesIO(record_data), len(record_data),
                content_type="application/json"
            )
            msg = f"SN {sn} 的记录已成功写入！"
            if errors:
                msg += "\n\n注意:\n" + "\n".join(errors)
            QMessageBox.information(self, "录入成功", msg)
            self.manual_result_label.setText(f"✅ 最近录入: SN={sn}, 共 {len(burn_results)-1} 项资源")
            self.manual_result_label.setStyleSheet("color: green;")
            # 清空输入
            self.manual_sn.clear()
            self.manual_mac.clear()
            for e in list(self.manual_key_rows):
                self._remove_manual_key_row(e)
        except Exception as e:
            QMessageBox.critical(self, "写入失败", f"无法写入 MinIO: {e}")

    def _try_move_to_used(self, res_path, filename):
        """尝试将资源从 available 移到 used，支持模糊匹配（不带后缀也能找到）"""
        from minio.commonconfig import CopySource
        # 先精确匹配
        src = f"{res_path}/available/{filename}"
        try:
            self.db.client.stat_object(self.db.bucket, src)
        except:
            # 精确匹配失败，按前缀模糊搜索
            src = None
            prefix = f"{res_path}/available/{filename}"
            try:
                for obj in self.db.client.list_objects(self.db.bucket, prefix=prefix, recursive=True):
                    src = obj.object_name
                    break
            except:
                pass
            if not src:
                return False
        dst = src.replace("/available/", "/used/", 1)
        try:
            source = CopySource(self.db.bucket, src)
            self.db.client.copy_object(self.db.bucket, dst, source)
            self.db.client.remove_object(self.db.bucket, src)
            return True
        except:
            return False

    # ==================== Tab 2: 库存查询 ====================
    def _create_view_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)

        filter_bar = QHBoxLayout()

        # 芯片方案过滤
        filter_bar.addWidget(QLabel("芯片方案:"))
        self.view_platform_filter = QComboBox()
        self.view_platform_filter.addItems(["全部方案"] + get_platforms(self.config))
        self.view_platform_filter.currentTextChanged.connect(self._on_view_platform_changed)
        filter_bar.addWidget(self.view_platform_filter)

        filter_bar.addWidget(QLabel("类别:"))
        self.type_filter = QComboBox()
        self._update_view_type_filter("全部方案")
        filter_bar.addWidget(self.type_filter)

        filter_bar.addWidget(QLabel("状态:"))
        self.status_filter = QComboBox()
        self.status_filter.addItems(["全部状态", "待使用 (available)", "已使用 (used)"])
        filter_bar.addWidget(self.status_filter)

        filter_bar.addStretch()
        btn_refresh = QPushButton("刷新列表")
        btn_refresh.clicked.connect(self._refresh_inventory)
        filter_bar.addWidget(btn_refresh)
        layout.addLayout(filter_bar)

        self.table = QTableWidget(0, 6)
        self.table.setHorizontalHeaderLabels(["芯片方案", "分类", "子类型", "客户", "唯一标识 (ID)", "状态"])
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        layout.addWidget(self.table)
        return widget

    def _on_view_platform_changed(self, platform_name):
        """库存查询页面切换方案时，更新类别下拉框选项"""
        self._update_view_type_filter(platform_name)

    def _update_view_type_filter(self, platform_name):
        """根据选中的方案更新类别下拉列表"""
        self.type_filter.clear()
        mac_clients = self.config.get("mac_clients", ["Vizio", "Onn"])
        mac_items = ["MAC (全部)"] + [f"MAC ({c})" for c in mac_clients]

        if platform_name == "全部方案":
            self.type_filter.addItems(["全部"] + mac_items + get_platform_key_types(self.config))
        else:
            self.type_filter.addItems([f"全部 ({platform_name})"] + mac_items + get_platform_key_types(self.config, platform_name))

    # ==================== Tab 3: SN 追溯 ====================
    def _create_trace_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)

        search_layout = QHBoxLayout()
        self.trace_sn_input = QLineEdit()
        self.trace_sn_input.setPlaceholderText("请输入要查询的机器 SN 序列号")
        self.trace_sn_input.returnPressed.connect(self._query_sn_history)
        btn_search = QPushButton("查询绑定关系")
        btn_search.clicked.connect(self._query_sn_history)
        btn_search.setStyleSheet("background-color: #27ae60; color: white; padding: 5px 15px;")
        search_layout.addWidget(self.trace_sn_input)
        search_layout.addWidget(btn_search)
        layout.addLayout(search_layout)

        layout.addWidget(QLabel("<b>绑定明细：</b>"))
        self.trace_result_table = QTableWidget(0, 2)
        self.trace_result_table.setHorizontalHeaderLabels(["资源类型", "绑定的资源 ID / 值"])
        self.trace_result_table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        layout.addWidget(self.trace_result_table)

        self.raw_json_view = QPlainTextEdit()
        self.raw_json_view.setReadOnly(True)
        self.raw_json_view.setMaximumHeight(150)
        self.raw_json_view.setStyleSheet("background: #f4f4f4; font-family: Consolas;")
        layout.addWidget(QLabel("原始归档数据:"))
        layout.addWidget(self.raw_json_view)
        return widget

    # ==================== Tab 4: 烧录工具 ====================
    def _create_burn_tab(self):
        widget = QWidget()
        main_h_layout = QHBoxLayout(widget)

        # --- 左侧：操作面板 ---
        left_panel = QWidget()
        left_layout = QVBoxLayout(left_panel)

        # 硬件配置
        hw_group = QGroupBox("1. 硬件连接配置")
        hw_layout = QHBoxLayout()
        self.port_combo = QComboBox()
        self.btn_refresh_port = QPushButton("刷新")
        self.btn_refresh_port.clicked.connect(self._refresh_serial_ports)
        self.btn_toggle_port = QPushButton("打开串口")
        self.btn_toggle_port.setStyleSheet("background-color: #2980b9; color: white;")
        self.btn_toggle_port.clicked.connect(self._toggle_serial_port)
        hw_layout.addWidget(QLabel("端口:"))
        hw_layout.addWidget(self.port_combo, 1)
        hw_layout.addWidget(self.btn_refresh_port)
        hw_layout.addWidget(self.btn_toggle_port)
        hw_group.setLayout(hw_layout)
        left_layout.addWidget(hw_group)

        # 任务勾选
        task_group = QGroupBox("2. 烧录任务")
        task_layout = QVBoxLayout()

        # 芯片方案选择
        platform_row = QHBoxLayout()
        platform_row.addWidget(QLabel("芯片方案:"))
        self.burn_platform_combo = QComboBox()
        self.burn_platform_combo.addItems(get_platforms(self.config))
        self.burn_platform_combo.setCurrentText(get_default_platform(self.config))
        self.burn_platform_combo.currentTextChanged.connect(self._on_burn_platform_changed)
        platform_row.addWidget(self.burn_platform_combo, 1)
        platform_row.addStretch()
        task_layout.addLayout(platform_row)

        client_row = QHBoxLayout()
        client_row.addWidget(QLabel("客户:"))
        self.client_combo = QComboBox()
        self.client_combo.addItems(self.config.get("mac_clients", ["Vizio", "Onn"]))
        client_row.addWidget(self.client_combo, 1)
        client_row.addStretch()
        task_layout.addLayout(client_row)

        self.check_mac = QCheckBox("烧录 MAC")
        self.check_mac.setChecked(True)
        task_layout.addWidget(self.check_mac)

        # 动态 Key 勾选容器
        self.key_checks_container = QWidget()
        self.key_checks_layout = QVBoxLayout(self.key_checks_container)
        self.key_checks_layout.setContentsMargins(0, 0, 0, 0)
        self.key_checks = {}
        self._rebuild_key_checks(get_default_platform(self.config))
        task_layout.addWidget(self.key_checks_container)

        task_group.setLayout(task_layout)
        left_layout.addWidget(task_group)

        # SN 录入
        self.sn_input = QLineEdit()
        self.sn_input.setPlaceholderText("在此输入或扫描 SN 序列号...")
        self.sn_input.setFont(QFont("Consolas", 14, QFont.Weight.Bold))
        self.sn_input.setStyleSheet("QLineEdit { font-size: 16px; padding: 8px 12px; background-color: #090d16; border: 1px solid #38bdf8; border-radius: 8px; color: #38bdf8; } QLineEdit:focus { border: 2px solid #00f0ff; background-color: #0e172a; }")

        self.btn_start = QPushButton("🚀 开始全功能烧录")
        self.btn_start.setFixedHeight(52)
        self.btn_start.setCursor(Qt.CursorShape.PointingHandCursor)
        self.btn_start.setStyleSheet("""
            QPushButton {
                background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #10b981, stop:1 #047857);
                color: #ffffff;
                font-size: 16px;
                font-weight: bold;
                border-radius: 8px;
                border: 1px solid #34d399;
            }
            QPushButton:hover {
                background-color: #10b981;
                border: 1px solid #6ee7b7;
            }
            QPushButton:pressed {
                background-color: #047857;
            }
            QPushButton:disabled {
                background-color: #1e293b;
                border: 1px solid #334155;
                color: #64748b;
            }
        """)
        self.btn_start.clicked.connect(self._run_burn)
        left_layout.addWidget(QLabel("<b>SN 序列号输入:</b>"))
        left_layout.addWidget(self.sn_input)
        left_layout.addWidget(self.btn_start)

        # 流程日志
        self.burn_log = QPlainTextEdit()
        self.burn_log.setReadOnly(True)
        left_layout.addWidget(QLabel("流程状态:"))
        left_layout.addWidget(self.burn_log)
        main_h_layout.addWidget(left_panel, 2)

        # --- 右侧：串口监控 ---
        right_panel = QGroupBox("串口数据监控 (Serial Monitor)")
        right_layout = QVBoxLayout(right_panel)
        monitor_tools = QHBoxLayout()
        self.check_hex_show = QCheckBox("16进制显示 (Hex)")
        self.check_hex_show.setChecked(True)
        btn_clear = QPushButton("清除窗口")
        btn_clear.clicked.connect(lambda: self.serial_monitor.clear())
        monitor_tools.addWidget(self.check_hex_show)
        monitor_tools.addStretch()
        monitor_tools.addWidget(btn_clear)
        right_layout.addLayout(monitor_tools)

        self.serial_monitor = QPlainTextEdit()
        self.serial_monitor.setReadOnly(True)
        self.serial_monitor.setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: 'Consolas';")
        right_layout.addWidget(self.serial_monitor)
        main_h_layout.addWidget(right_panel, 3)

        self._refresh_serial_ports()
        return widget

    def _on_import_platform_changed(self, platform_name):
        """导入页面切换芯片方案时，刷新 Key 类型下拉列表"""
        self.key_type_select.clear()
        self.key_type_select.addItems(get_platform_key_types(self.config, platform_name))

    def _on_burn_platform_changed(self, platform_name):
        """烧录工具页面切换芯片方案时，重新构建 Key 复选框"""
        self._rebuild_key_checks(platform_name)

    def _rebuild_key_checks(self, platform_name):
        """根据指定的芯片方案动态生成对应的 Key 复选框"""
        for cb in self.key_checks.values():
            cb.setParent(None)
            cb.deleteLater()
        self.key_checks.clear()

        key_list = get_platform_key_types(self.config, platform_name)
        for kt in key_list:
            cb = QCheckBox(f"烧录 {kt}")
            self.key_checks[kt] = cb
            self.key_checks_layout.addWidget(cb)

    # ==================== 资源导入逻辑 ====================
    def _normalize_mac(self, mac_str):
        clean = mac_str.replace(":", "").replace("-", "").strip().upper()
        if len(clean) != 12:
            raise ValueError("MAC 地址必须为 12 位十六进制字符")
        return "-".join(clean[i:i+2] for i in range(0, 12, 2))

    def _handle_batch_mac_upload(self):
        # 检查网络连接
        if not self.db.check_network_connection():
            QMessageBox.critical(self, "网络错误", "无法连接到MinIO服务器！\n请检查网络连接后重试。")
            return

        raw_mac = self.start_mac_input.text().strip()
        count = self.mac_count_input.value()
        client_name = self.mac_client_select.currentText().strip().lower()
        try:
            std_mac = self._normalize_mac(raw_mac)
            mac_base = int(std_mac.replace("-", ""), 16)
            success_count = 0
            for i in range(count):
                new_hex = hex(mac_base + i)[2:].zfill(12).upper()
                final_mac = "-".join(new_hex[j:j+2] for j in range(0, 12, 2))
                success, _ = self.db.upload_new_resource(f"mac/{client_name}", final_mac)
                if success: success_count += 1
            QMessageBox.information(self, "完成", f"成功导入 {success_count} 条 MAC ({client_name})")
        except Exception as e:
            QMessageBox.critical(self, "错误", f"MAC 处理失败: {str(e)}")

    def _handle_key_upload(self, is_batch=False):
        # 检查网络连接
        if not self.db.check_network_connection():
            QMessageBox.critical(self, "网络错误", "无法连接到MinIO服务器！\n请检查网络连接后重试。")
            return

        key_type = self.key_type_select.currentText().strip()
        if not key_type: return
        if "ULPK" in key_type.upper():
            client_name = self.key_client_select.currentText().strip().lower()
            upload_path = f"key/{key_type}/{client_name}"
        else:
            upload_path = f"key/{key_type}"

        success_count = 0
        skipped_count = 0
        if is_batch:
            dir_path = QFileDialog.getExistingDirectory(self, "选择 Key 文件夹")
            if not dir_path: return
            files = [f for f in os.listdir(dir_path) if os.path.isfile(os.path.join(dir_path, f))]

            # 检查文件格式要求
            expected_ext = None
            if "ULPK" in key_type.upper():
                expected_ext = ".dat"
            elif "HDCP" in key_type.upper():
                expected_ext = ".bin"

            for fname in files:
                fpath = os.path.join(dir_path, fname)

                # 检查文件扩展名
                if expected_ext and not fname.lower().endswith(expected_ext):
                    print(f"跳过文件 {fname}: 期望格式 {expected_ext}，实际格式 .{fname.split('.')[-1] if '.' in fname else '无扩展名'}")
                    skipped_count += 1
                    continue

                try:
                    with open(fpath, 'rb') as f:
                        content = f.read()
                    success, _ = self.db.upload_binary_resource(upload_path, fname, content)
                    if success: success_count += 1
                except Exception as e:
                    print(f"文件 {fname} 上传失败: {e}")

            # 显示结果
            total_files = len(files)
            if skipped_count > 0:
                QMessageBox.information(self, "结果",
                    f"批量导入完成：\n"
                    f"✅ 成功上传: {success_count} 个文件\n"
                    f"⚠️  跳过格式不符: {skipped_count} 个文件\n"
                    f"📁 总文件数: {total_files} 个")
            else:
                QMessageBox.information(self, "结果", f"批量导入：{success_count}/{total_files}")
        else:
            fpath, _ = QFileDialog.getOpenFileName(self, "选择 Key 文件")
            if not fpath: return

            fname = os.path.basename(fpath)

            # 检查文件格式要求
            expected_ext = None
            if "ULPK" in key_type.upper():
                expected_ext = ".dat"
            elif "HDCP" in key_type.upper():
                expected_ext = ".bin"

            if expected_ext and not fname.lower().endswith(expected_ext):
                QMessageBox.warning(self, "文件格式错误",
                    f"文件格式不正确！\n\n"
                    f"Key 类型: {key_type}\n"
                    f"期望格式: *{expected_ext}\n"
                    f"选择文件: {fname}\n\n"
                    f"请重新选择正确格式的文件。")
                return

            try:
                with open(fpath, 'rb') as f:
                    content = f.read()
                success, msg = self.db.upload_binary_resource(upload_path, fname, content)
                if success:
                    QMessageBox.information(self, "成功", f"Key {fname} 已上传")
                else:
                    QMessageBox.warning(self, "失败", msg)
            except Exception as e:
                QMessageBox.critical(self, "错误", str(e))

    # ==================== 库存查询逻辑 ====================
    def _refresh_inventory(self):
        # 检查网络连接
        if not self.db.check_network_connection():
            QMessageBox.critical(self, "网络错误", "无法连接到MinIO服务器！\n请检查网络连接后重试。")
            return

        self.table.setRowCount(0)
        sel_platform = self.view_platform_filter.currentText()
        sel_type = self.type_filter.currentText()
        sel_stat = self.status_filter.currentText()

        prefixes = []
        if sel_type == "全部" or sel_type.startswith("全部 ("):
            if sel_platform == "全部方案":
                prefixes = ["mac/", "key/"]
            else:
                plat_keys = get_platform_key_types(self.config, sel_platform)
                prefixes = [f"key/{kt}/" for kt in plat_keys] + ["mac/"]
        elif sel_type == "MAC (全部)":
            prefixes = ["mac/"]
        elif sel_type.startswith("MAC ("):
            client_name = sel_type.replace("MAC (", "").replace(")", "").strip().lower()
            prefixes = [f"mac/{client_name}/"]
        else:
            prefixes = [f"key/{sel_type}/"]

        stat_keywords = []
        if "待使用" in sel_stat: stat_keywords = ["available"]
        elif "已使用" in sel_stat: stat_keywords = ["used"]
        else: stat_keywords = ["available", "used"]

        for p in prefixes:
            try:
                objs = self.db.client.list_objects(self.db.bucket, prefix=p, recursive=True)
                for obj in objs:
                    if any(f"/{kw}/" in obj.object_name for kw in stat_keywords):
                        self._add_row(obj.object_name, sel_platform)
            except: continue

    def _add_row(self, path, sel_platform="全部方案"):
        parts = path.split('/')
        if len(parts) < 3: return

        main_type = parts[0].upper()
        status = "已使用" if "used" in path else "待使用"
        res_id = os.path.basename(path).split('.')[0]
        mac_clients = [c.lower() for c in self.config.get("mac_clients", ["Vizio", "Onn"])]
        client = ""

        if parts[0] == "mac":
            platform = "通用"
            sub_type = "网络地址"
            if len(parts) >= 4 and parts[1].lower() in mac_clients:
                client = parts[1].capitalize()
        else:
            sub_type = parts[1]
            platform = get_key_platform(self.config, sub_type)
            for p in parts[2:]:
                if p.lower() in mac_clients:
                    client = p.capitalize()
                    break

        # 若选择了特定方案且当前项为 Key，过滤掉不属于该方案的项
        if sel_platform != "全部方案" and platform != "通用" and platform != sel_platform:
            return

        row = self.table.rowCount()
        self.table.insertRow(row)

        self.table.setItem(row, 0, QTableWidgetItem(platform))
        self.table.setItem(row, 1, QTableWidgetItem(main_type))
        self.table.setItem(row, 2, QTableWidgetItem(sub_type))
        self.table.setItem(row, 3, QTableWidgetItem(client))
        self.table.setItem(row, 4, QTableWidgetItem(res_id))
        stat_item = QTableWidgetItem(status)
        if status == "待使用": stat_item.setForeground(Qt.GlobalColor.darkGreen)
        self.table.setItem(row, 5, stat_item)

    # ==================== SN 追溯逻辑 ====================
    def _query_sn_history(self):
        # 检查网络连接
        if not self.db.check_network_connection():
            QMessageBox.critical(self, "网络错误", "无法连接到MinIO服务器！\n请检查网络连接后重试。")
            return

        sn = self.trace_sn_input.text().strip()
        if not sn: return
        self.trace_result_table.setRowCount(0)
        self.raw_json_view.clear()
        candidate_paths = [
            f"sn_record/{sn}.json",
            f"sn_record/available/{sn}.json",
            f"sn_record/used/{sn}.json",
        ]
        data = None
        try:
            for object_path in candidate_paths:
                try:
                    response = self.db.client.get_object(self.db.bucket, object_path)
                    data = json.load(response)
                    response.close()
                    response.release_conn()
                    break
                except: continue
            if data is None:
                raise FileNotFoundError("未找到")
            self.raw_json_view.setPlainText(json.dumps(data, indent=4, ensure_ascii=False))
            results = data.get("burn_results", {})
            for key_type, res_id in results.items():
                row = self.trace_result_table.rowCount()
                self.trace_result_table.insertRow(row)
                self.trace_result_table.setItem(row, 0, QTableWidgetItem(key_type))
                self.trace_result_table.setItem(row, 1, QTableWidgetItem(str(res_id)))
        except Exception:
            QMessageBox.warning(self, "未找到记录",
                                f"在仓库中未发现 SN 为 [{sn}] 的烧录记录。\n请检查输入是否正确。")

    # ==================== 烧录工具逻辑 ====================
    def _update_serial_monitor(self, data_bytes):
        if self.check_hex_show.isChecked():
            text = " ".join([f"{b:02X}" for b in data_bytes])
        else:
            text = "".join([chr(b) if 32 <= b <= 126 or b in [10, 13] else "." for b in data_bytes])
        self.serial_monitor.appendPlainText(f"[RECV] {text}")
        QApplication.processEvents()
        self.serial_monitor.verticalScrollBar().setValue(
            self.serial_monitor.verticalScrollBar().maximum())

    def _refresh_serial_ports(self):
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        if not ports:
            self.port_combo.addItem("未发现可用串口", None)
            self.btn_start.setEnabled(False)
            return
        for p in ports:
            self.port_combo.addItem(f"{p.device} ({p.description})", p.device)
        self.btn_start.setEnabled(True)

    def _toggle_serial_port(self):
        if self.protocol.ser and self.protocol.ser.is_open:
            if self.serial_reader:
                self.serial_reader.stop()
                self.serial_reader = None
            self.protocol.ser.close()
            self.protocol.ser = None
            self.btn_toggle_port.setText("打开串口")
            self.btn_toggle_port.setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0284c7, stop:1 #0369a1); color: white; border-radius: 6px; font-weight: bold;")
            self.port_combo.setEnabled(True)
            self.btn_refresh_port.setEnabled(True)
            self._burn_log("串口已关闭", "#f59e0b")
        else:
            current_port = self.port_combo.itemData(self.port_combo.currentIndex())
            if not current_port:
                QMessageBox.warning(self, "错误", "请先选择一个有效的串口！")
                return
            try:
                self.protocol.port = current_port
                self.protocol.ser = serial.Serial(current_port, self.protocol.baud, timeout=2)
                self.serial_reader = SerialReaderThread(self.protocol.ser)
                self.serial_reader.data_received.connect(self._update_serial_monitor)
                self.serial_reader.start()
                self.btn_toggle_port.setText("关闭串口")
                self.btn_toggle_port.setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ef4444, stop:1 #b91c1c); color: white; border-radius: 6px; font-weight: bold;")
                self.port_combo.setEnabled(False)
                self.btn_refresh_port.setEnabled(False)
                self._burn_log(f"串口 {current_port} 已成功打开", "#34d399")
            except Exception as e:
                QMessageBox.critical(self, "串口错误", f"无法打开串口: {e}")

    def _burn_log(self, message, color_hex="#ffffff"):
        fmt = f'<span style="color:{color_hex};">{message}</span>'
        self.burn_log.appendHtml(fmt)
        self.burn_log.verticalScrollBar().setValue(
            self.burn_log.verticalScrollBar().maximum())

    def _check_sn_exists(self, sn):
        """检查 SN 是否已有烧录记录"""
        for path in [f"sn_record/{sn}.json", f"sn_record/available/{sn}.json", f"sn_record/used/{sn}.json"]:
            try:
                self.db.client.stat_object(self.db.bucket, path)
                return True
            except:
                continue
        return False

    def _run_burn(self):
        # 检查网络连接
        if not self.db.check_network_connection():
            QMessageBox.critical(self, "网络错误", "无法连接到MinIO服务器！\n请检查网络连接后重试。")
            return

        current_port = self.port_combo.itemData(self.port_combo.currentIndex())
        if not current_port:
            QMessageBox.warning(self, "硬件错误", "请先连接串口并点击刷新！")
            return
        sn = self.sn_input.text().strip()
        if not sn:
            QMessageBox.warning(self, "提醒", "请输入或扫描机器 SN 序列号！")
            self.sn_input.setFocus()
            return

        # 检查 SN 是否已使用
        if self._check_sn_exists(sn):
            ret = QMessageBox.warning(self, "SN 重复",
                f"SN [{sn}] 已存在烧录记录！\n继续将覆盖原有记录，是否继续？",
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.No)
            if ret != QMessageBox.StandardButton.Yes:
                return

        client_name = self.client_combo.currentText().strip().lower()
        tasks = []
        if self.check_mac.isChecked():
            tasks.append(f"mac/{client_name}")
        for kt, cb in self.key_checks.items():
            if cb.isChecked():
                if "ULPK" in kt.upper():
                    tasks.append(f"key/{kt}/{client_name}")
                else:
                    tasks.append(f"key/{kt}")
        if not tasks:
            QMessageBox.warning(self, "任务错误", "请至少勾选一项要烧录的内容！")
            return

        self.btn_start.setEnabled(False)
        self.sn_input.setEnabled(False)
        self.burn_log.clear()

        if self.serial_reader:
            self.serial_reader.stop()
            self.serial_reader = None

        self.protocol.port = current_port
        platform = self.burn_platform_combo.currentText().strip()
        self.worker = BurnWorker(self.db, self.protocol, sn, tasks, self.config, platform=platform)
        self.worker.monitor_signal = self.monitor_data_signal
        self.worker.log_signal.connect(self._burn_log)
        self.worker.result_signal.connect(self._on_burn_finished)
        self.worker.start()

    def _on_burn_finished(self, success, msg):
        self.btn_start.setEnabled(True)
        self.sn_input.setEnabled(True)
        self.sn_input.clear()
        self.sn_input.setFocus()

        if self.protocol.ser and self.protocol.ser.is_open and not self.serial_reader:
            self.serial_reader = SerialReaderThread(self.protocol.ser)
            self.serial_reader.data_received.connect(self._update_serial_monitor)
            self.serial_reader.start()

        if success:
            self._burn_log(f"\n[PASS] {msg}", "#2ecc71")
            QMessageBox.information(self, "烧录成功", f"SN: {self.worker.sn} 烧录完成！")
        else:
            self._burn_log(f"\n[FAIL] {msg}", "#e74c3c")
            QMessageBox.critical(self, "烧录失败", msg)

    def _update_network_status(self):
        """更新网络状态显示"""
        self.status_bar.setContentsMargins(10, 4, 10, 4)
        if self.network_status is None:
            self.status_bar.setText(" 网络状态: 🔄 正在检查连接...")
            self.status_bar.setStyleSheet("color: #38bdf8; font-weight: bold; background-color: #1e293b; border-radius: 4px; padding: 4px;")
        elif self.network_status:
            self.status_bar.setText(" 网络状态: ✅ 已成功连接到 MinIO 仓库")
            self.status_bar.setStyleSheet("color: #34d399; font-weight: bold; background-color: #1e293b; border-radius: 4px; padding: 4px;")
        else:
            self.status_bar.setText(" 网络状态: ❌ 无法连接到 MinIO 仓库")
            self.status_bar.setStyleSheet("color: #f87171; font-weight: bold; background-color: #1e293b; border-radius: 4px; padding: 4px;")

    def _async_check_network(self):
        """异步检查网络连接"""
        try:
            self.network_status = self.db.check_network_connection()
            self._update_network_status()

            if not self.network_status:
                QMessageBox.warning(self, "网络连接警告",
                    "无法连接到MinIO服务器！\n\n请检查：\n1. 网络连接是否正常\n2. MinIO服务器是否运行\n3. 服务器地址和端口是否正确\n\n程序将继续运行，但网络相关功能将不可用。")
        except Exception as e:
            self.network_status = False
            self._update_network_status()
            QMessageBox.warning(self, "网络检查错误", f"网络检查过程中发生错误：{e}")

    def _check_network_status(self):
        """手动检查网络状态"""
        self.network_status = self.db.check_network_connection()
        self._update_network_status()
        if self.network_status:
            QMessageBox.information(self, "网络检查", "✅ 网络连接正常！")
        else:
            QMessageBox.warning(self, "网络检查", "❌ 无法连接到MinIO服务器！\n请检查网络连接。")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyleSheet(MODERN_INDUSTRIAL_QSS)
    window = MainWindow()
    window.show()
    # UI显示后异步检查网络连接
    from PyQt6.QtCore import QTimer
    QTimer.singleShot(100, window._async_check_network)
    sys.exit(app.exec())
