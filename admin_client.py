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

from minio_db import MinioWarehouse
from tv_protocol import TVSerialProtocol


def get_app_dir():
    """获取应用所在目录（兼容 pyinstaller 打包后的路径）"""
    if getattr(sys, 'frozen', False):
        return os.path.dirname(sys.executable)
    return os.path.dirname(os.path.abspath(__file__))

def load_config():
    """加载本地配置文件"""
    default_config = {
        "key_types": ["HDCP1.4 5586 dev", "HDCP2.2 5586 dev",
                       "HDCP1.4 5586 prod", "HDCP2.2 5586 prod",
                       "ULPK 5586F dev", "ULPK 5586F prod",
                       "ULPK 5586L dev", "ULPK 5586L prod"],
        "mac_clients": ["Vizio", "Onn"],
        "minio": {"endpoint": "172.16.9.28:9000", "access_key": "admin",
                  "secret_key": "12345678", "bucket": "warehouse"}
    }
    config_path = os.path.join(get_app_dir(), "config.json")
    if os.path.exists(config_path):
        try:
            with open(config_path, "r", encoding="utf-8") as f:
                return json.load(f)
        except:
            return default_config
    return default_config


# ============================================================
# 烧录工作线程（从 prod_client.py 合并）
# ============================================================
class BurnWorker(QThread):
    log_signal = pyqtSignal(str, str)
    result_signal = pyqtSignal(bool, str)

    def __init__(self, db, protocol, sn, task_list, config):
        super().__init__()
        self.db = db
        self.protocol = protocol
        self.sn = sn
        self.task_list = task_list
        self.config = config
        self.monitor_signal = None

    def run(self):
        try:
            self.log_signal.emit(f"--- 5586 全功能烧录启动: SN {self.sn} ---", "#ffffff")
            # 进入工厂模式
            fac_cmd = self.protocol.pack_factory_mode()
            self.protocol.send_and_wait_ack(fac_cmd, monitor_signal=self.monitor_signal,
                                            log_signal=self.log_signal,
                                            max_retries=5, ack_delay=0.3)
            time.sleep(0.5)
            burn_history = {}
            success = False

            for task_path in self.task_list:
                cmd_type = task_path.split('/')[-1]

                if task_path.startswith("mac/"):
                    res_id = self.db.fetch_and_lock(task_path)
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
                    res_id = self.db.fetch_and_lock(task_path)
                    if not res_id:
                        self.result_signal.emit(False, f"库存空: {task_path}")
                        return
                    raw_data = self._read_minio_binary(task_path, res_id)
                    hdcp_type = 0xE3 if "1.4" in cmd_type else 0xE4
                    success = self.process_hdcp_burn(cmd_type, hdcp_type, raw_data)
                    if not success:
                        return

                elif "ULPK" in cmd_type.upper():
                    res_id = self.db.fetch_and_lock(task_path)
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

            # 归档烧录记录
            record_data = json.dumps({"sn": self.sn, "burn_results": burn_history}).encode('utf-8')
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
        response = self.db.client.get_object(self.db.bucket, f"{task_path}/used/{res_id}")
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
        self.setWindowTitle("Key/MAC 烧录管理系统 - v3.0")
        self.resize(1100, 800)

        # 初始化 MinIO
        m_cfg = self.config.get("minio", {})
        self.db = MinioWarehouse(
            endpoint=m_cfg.get("endpoint", "127.0.0.1:9000"),
            access_key=m_cfg.get("access_key", "minioadmin"),
            secret_key=m_cfg.get("secret_key", "minioadmin"),
            bucket=m_cfg.get("bucket", "warehouse")
        )

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
        self.tabs.addTab(self._create_import_tab(), "资源导入")
        self.tabs.addTab(self._create_view_tab(), "库存查询")
        self.tabs.addTab(self._create_trace_tab(), "SN 追溯")
        self.tabs.addTab(self._create_burn_tab(), "烧录工具")
        layout.addWidget(self.tabs)

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
        type_select_layout = QHBoxLayout()
        type_select_layout.addWidget(QLabel("Key 类型:"))
        self.key_type_select = QComboBox()
        self.key_type_select.addItems(self.config.get("key_types", []))
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
        return widget

    # ==================== Tab 2: 库存查询 ====================
    def _create_view_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)

        filter_bar = QHBoxLayout()
        self.type_filter = QComboBox()
        mac_clients = self.config.get("mac_clients", ["Vizio", "Onn"])
        self.type_filter.addItems(["全部", "MAC (全部)"] + [f"MAC ({c})" for c in mac_clients])
        self.type_filter.addItems(self.config.get("key_types", []))
        self.status_filter = QComboBox()
        self.status_filter.addItems(["全部状态", "待使用 (available)", "已使用 (used)"])
        btn_refresh = QPushButton("刷新列表")
        btn_refresh.clicked.connect(self._refresh_inventory)
        filter_bar.addWidget(QLabel("类别:"))
        filter_bar.addWidget(self.type_filter)
        filter_bar.addWidget(QLabel("状态:"))
        filter_bar.addWidget(self.status_filter)
        filter_bar.addStretch()
        filter_bar.addWidget(btn_refresh)
        layout.addLayout(filter_bar)

        self.table = QTableWidget(0, 5)
        self.table.setHorizontalHeaderLabels(["分类", "子类型", "客户", "唯一标识 (ID)", "状态"])
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        layout.addWidget(self.table)
        return widget

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
        self.key_checks = {}
        for kt in self.config.get("key_types", []):
            cb = QCheckBox(f"烧录 {kt}")
            self.key_checks[kt] = cb
            task_layout.addWidget(cb)
        task_group.setLayout(task_layout)
        left_layout.addWidget(task_group)

        # SN 录入
        self.sn_input = QLineEdit()
        self.sn_input.setPlaceholderText("在此录入 SN...")
        self.sn_input.setFont(QFont("Consolas", 14))
        self.btn_start = QPushButton("开始烧录")
        self.btn_start.setFixedHeight(50)
        self.btn_start.setStyleSheet("background-color: #27ae60; color: white; font-weight: bold;")
        self.btn_start.clicked.connect(self._run_burn)
        left_layout.addWidget(QLabel("SN 输入:"))
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


    # ==================== 资源导入逻辑 ====================
    def _normalize_mac(self, mac_str):
        clean = mac_str.replace(":", "").replace("-", "").strip().upper()
        if len(clean) != 12:
            raise ValueError("MAC 地址必须为 12 位十六进制字符")
        return "-".join(clean[i:i+2] for i in range(0, 12, 2))

    def _handle_batch_mac_upload(self):
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
        key_type = self.key_type_select.currentText().strip()
        if not key_type: return
        if "ULPK" in key_type.upper():
            client_name = self.key_client_select.currentText().strip().lower()
            upload_path = f"key/{key_type}/{client_name}"
        else:
            upload_path = f"key/{key_type}"

        success_count = 0
        if is_batch:
            dir_path = QFileDialog.getExistingDirectory(self, "选择 Key 文件夹")
            if not dir_path: return
            files = [f for f in os.listdir(dir_path) if os.path.isfile(os.path.join(dir_path, f))]
            for fname in files:
                fpath = os.path.join(dir_path, fname)
                try:
                    with open(fpath, 'rb') as f:
                        content = f.read()
                    success, _ = self.db.upload_binary_resource(upload_path, fname, content)
                    if success: success_count += 1
                except Exception as e:
                    print(f"文件 {fname} 上传失败: {e}")
            QMessageBox.information(self, "结果", f"批量导入：{success_count}/{len(files)}")
        else:
            fpath, _ = QFileDialog.getOpenFileName(self, "选择 Key 文件")
            if not fpath: return
            try:
                with open(fpath, 'rb') as f:
                    content = f.read()
                fname = os.path.basename(fpath)
                success, msg = self.db.upload_binary_resource(upload_path, fname, content)
                if success:
                    QMessageBox.information(self, "成功", f"Key {fname} 已上传")
                else:
                    QMessageBox.warning(self, "失败", msg)
            except Exception as e:
                QMessageBox.critical(self, "错误", str(e))

    # ==================== 库存查询逻辑 ====================
    def _refresh_inventory(self):
        self.table.setRowCount(0)
        sel_type = self.type_filter.currentText()
        sel_stat = self.status_filter.currentText()

        prefixes = []
        if sel_type == "全部": prefixes = ["mac/", "key/"]
        elif sel_type == "MAC (全部)": prefixes = ["mac/"]
        elif sel_type.startswith("MAC ("):
            client_name = sel_type.replace("MAC (", "").replace(")", "").strip().lower()
            prefixes = [f"mac/{client_name}/"]
        else: prefixes = [f"key/{sel_type}/"]

        stat_keywords = []
        if "待使用" in sel_stat: stat_keywords = ["available"]
        elif "已使用" in sel_stat: stat_keywords = ["used"]
        else: stat_keywords = ["available", "used"]

        for p in prefixes:
            try:
                objs = self.db.client.list_objects(self.db.bucket, prefix=p, recursive=True)
                for obj in objs:
                    if any(f"/{kw}/" in obj.object_name for kw in stat_keywords):
                        self._add_row(obj.object_name)
            except: continue

    def _add_row(self, path):
        parts = path.split('/')
        if len(parts) < 3: return
        row = self.table.rowCount()
        self.table.insertRow(row)
        main_type = parts[0].upper()
        status = "已使用" if "used" in path else "待使用"
        res_id = os.path.basename(path).split('.')[0]
        mac_clients = [c.lower() for c in self.config.get("mac_clients", ["Vizio", "Onn"])]
        client = ""
        if parts[0] == "mac":
            sub_type = "网络地址"
            if len(parts) >= 4 and parts[1].lower() in mac_clients:
                client = parts[1].capitalize()
        else:
            sub_type = parts[1]
            for p in parts[2:]:
                if p.lower() in mac_clients:
                    client = p.capitalize()
                    break
        self.table.setItem(row, 0, QTableWidgetItem(main_type))
        self.table.setItem(row, 1, QTableWidgetItem(sub_type))
        self.table.setItem(row, 2, QTableWidgetItem(client))
        self.table.setItem(row, 3, QTableWidgetItem(res_id))
        stat_item = QTableWidgetItem(status)
        if status == "待使用": stat_item.setForeground(Qt.GlobalColor.darkGreen)
        self.table.setItem(row, 4, stat_item)

    # ==================== SN 追溯逻辑 ====================
    def _query_sn_history(self):
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
            self.btn_toggle_port.setStyleSheet("background-color: #2980b9; color: white;")
            self.port_combo.setEnabled(True)
            self.btn_refresh_port.setEnabled(True)
            self._burn_log("串口已关闭", "#e67e22")
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
                self.btn_toggle_port.setStyleSheet("background-color: #e74c3c; color: white;")
                self.port_combo.setEnabled(False)
                self.btn_refresh_port.setEnabled(False)
                self._burn_log(f"串口 {current_port} 已打开", "#2ecc71")
            except Exception as e:
                QMessageBox.critical(self, "串口错误", f"无法打开串口: {e}")

    def _burn_log(self, message, color_hex="#ffffff"):
        fmt = f'<span style="color:{color_hex};">{message}</span>'
        self.burn_log.appendHtml(fmt)
        self.burn_log.verticalScrollBar().setValue(
            self.burn_log.verticalScrollBar().maximum())

    def _run_burn(self):
        current_port = self.port_combo.itemData(self.port_combo.currentIndex())
        if not current_port:
            QMessageBox.warning(self, "硬件错误", "请先连接串口并点击刷新！")
            return
        sn = self.sn_input.text().strip()
        if not sn:
            QMessageBox.warning(self, "提醒", "请输入或扫描机器 SN 序列号！")
            self.sn_input.setFocus()
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
        self.worker = BurnWorker(self.db, self.protocol, sn, tasks, self.config)
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


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
