import sys
import json
import os
import struct
import time
import serial.tools.list_ports
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QLineEdit, QPushButton, QLabel, 
                             QPlainTextEdit, QMessageBox, QComboBox, QCheckBox, QGroupBox)
from PyQt6.QtCore import Qt, QThread, pyqtSignal
from PyQt6.QtGui import QFont

from minio_db import MinioWarehouse
from tv_protocol import TVSerialProtocol

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
            
            # 进入工厂模式 (与 MFC 一致: "07 51 01 07 01 CB D1")
            fac_cmd = self.protocol.pack_factory_mode()
            self.protocol.send_and_wait_ack(fac_cmd, monitor_signal=self.monitor_signal,
                                            max_retries=5, ack_delay=0.3)
            time.sleep(0.5)
            
            burn_history = {}
            success = False

            for task_path in self.task_list:
                cmd_type = task_path.split('/')[-1]

                # MAC 任务不需要从 MinIO 读二进制文件
                if cmd_type.upper() == "MAC":
                    res_id = self.db.fetch_and_lock(task_path)
                    if not res_id:
                        self.result_signal.emit(False, f"库存空: {task_path}")
                        return
                    self.log_signal.emit(f"[MAC] 资源: {res_id}", "#3498db")
                    cmd = self.protocol.pack_mac_command(res_id)
                    self.log_signal.emit(f"[MAC] 发送: {cmd.hex(' ').upper()}", "#9b59b6")
                    success, msg = self.protocol.send_raw(cmd, monitor_signal=self.monitor_signal)

                elif "HDCP" in cmd_type.upper():
                    res_id = self.db.fetch_and_lock(task_path)
                    if not res_id:
                        self.result_signal.emit(False, f"库存空: {task_path}")
                        return
                    # 读取原始二进制 Key 文件
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
                    # 5586 ULPK: cmd_id=0xD1, payload=[UID 4字节]+[160字节数据]
                    cmd = self.protocol.pack_ulpk_command(uid, raw_data)
                    self.log_signal.emit(f"[ULPK] UID={uid}, 发送 {len(raw_data)} 字节", "#3498db")
                    ok, ack, msg = self.protocol.send_and_wait_ack(
                        cmd, monitor_signal=self.monitor_signal, max_retries=5, ack_delay=1.0)
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

            # 烧录 SN (MFC: cmd_id=0x88, SN 按 ASCII 编码)
            self.log_signal.emit(f"[SN] 正在写入 SN: {self.sn}", "#3498db")
            sn_cmd = self.protocol.pack_sn_command(self.sn)
            self.log_signal.emit(f"[SN] 发送: {sn_cmd.hex(' ').upper()}", "#9b59b6")
            sn_ok, sn_msg = self.protocol.send_raw(sn_cmd, monitor_signal=self.monitor_signal)
            if not sn_ok:
                self.result_signal.emit(False, f"SN 写入失败: {sn_msg}")
                return
            burn_history["SN"] = self.sn
            self.log_signal.emit("SN 写入成功 ✅", "#2ecc71")

            # 归档烧录记录
            self.db.upload_new_resource("sn_record", self.sn,
                                        {"sn": self.sn, "burn_results": burn_history})
            self.result_signal.emit(True, "所有任务已完成")
        except Exception as e:
            self.result_signal.emit(False, f"致命错误: {str(e)}")

    def _read_minio_binary(self, task_path, res_id):
        """从 MinIO 读取二进制资源并打印内容"""
        response = self.db.client.get_object(self.db.bucket, f"{task_path}/used/{res_id}")
        raw_data = response.read()
        response.close()
        response.release_conn()
        self.log_signal.emit(f"[MinIO] 资源: {res_id}, 大小: {len(raw_data)} 字节", "#3498db")
        self.log_signal.emit(f"[MinIO] HEX: {raw_data.hex(' ').upper()}", "#9b59b6")
        return raw_data

    def process_hdcp_burn(self, name, type_code, data):
        """
        HDCP 分包烧录，逐包等待 ACK，与 MFC Timer 状态机逻辑一致。
        
        HDCP1.4: block_size=120, type_code=0xE3
          MFC 根据实际文件大小动态计算包数，512字节文件只取前304字节。
          新版 burnHDCP() 支持任意长度，按 HDCP14_SIZE=120 分包。
          
        HDCP2.2: block_size=128, type_code=0xE4
          MFC 固定读取 1044 字节，BLOCK_NO=9, BLOCK_SIZE=128。
        """
        if type_code == 0xE3:
            block_size = self.protocol.HDCP14_BLOCK_SIZE  # 120
            # MFC: 512字节的bin文件只取前304字节
            if len(data) == 512:
                data = data[:304]
            total_blocks = (len(data) + block_size - 1) // block_size
        else:
            block_size = self.protocol.HDCP22_BLOCK_SIZE  # 128
            # MFC: 固定取前 1044 字节，固定 9 包
            data = data[:self.protocol.HDCP22_TOTAL_SIZE]
            total_blocks = self.protocol.HDCP22_BLOCK_NO  # 9

        full_crc = self.protocol.calculate_crc(data)

        # 1. 发送 Header 包并等待 ACK
        header_cmd = self.protocol.pack_hdcp_header(type_code, total_blocks, block_size, full_crc)
        self.log_signal.emit(f"[{name}] Header: blocks={total_blocks}, size={block_size}, crc=0x{full_crc:04X}", "#3498db")
        self.log_signal.emit(f"[{name}] Header HEX: {header_cmd.hex(' ').upper()}", "#9b59b6")

        ok, ack, msg = self.protocol.send_and_wait_ack(
            header_cmd, monitor_signal=self.monitor_signal, max_retries=10, ack_delay=0.3)
        if not ok:
            self.result_signal.emit(False, f"{name} Header 失败: {msg}")
            return False

        # 2. 逐包发送数据，每包等待 ACK (与 MFC 的 m_step 状态机一致)
        for b_id in range(1, total_blocks + 1):
            start = (b_id - 1) * block_size
            # 最后一包可能不足 block_size
            if b_id == total_blocks:
                end = len(data)
            else:
                end = start + block_size
            chunk = data[start:end]

            chunk_cmd = self.protocol.pack_hdcp_chunk(type_code, b_id, chunk)
            self.log_signal.emit(f"[{name}] 分包 {b_id}/{total_blocks}, {len(chunk)} 字节", "#3498db")

            ok, ack, msg = self.protocol.send_and_wait_ack(
                chunk_cmd, monitor_signal=self.monitor_signal, max_retries=10, ack_delay=0.3)
            if not ok:
                self.result_signal.emit(False, f"{name} 分包 {b_id} 失败: {msg}")
                return False

        # 最后一包的 ACK 应为 F2 表示整体完成
        self.log_signal.emit(f"[{name}] 烧录完成, 最终 ACK: {ack}", "#2ecc71")
        return True
    
    def extract_uid(self, filename):
        """
        从文件名提取 UID，与 MFC 逻辑一致:
        找到 .dat 前最后一个 '-' 后面的数字部分。
        例如: ulpk-potk-00199D-0359-12345678.dat -> 12345678
        """
        try:
            name_part = filename.replace(".json", "").replace(".dat", "").replace(".bin", "")
            if "-" in name_part:
                return name_part.split("-")[-1]
            return name_part
        except:
            return "0"

class OfflineProdWindow(QMainWindow):
    # 定义一个新的信号，用于 worker 线程向串口监控窗口发送数据
    monitor_data_signal = pyqtSignal(bytes)

    def __init__(self):
        super().__init__()
        self.setWindowTitle("电视 Key/MAC 手动烧录工具 (5586 专用版)")
        self.resize(1000, 800) # 调大窗口以容纳监控区
        
        self.config = self.load_local_config()

        # 2. 初始化 MinIO 仓库和串口协议
        try:
            self.db = MinioWarehouse(**self.config['minio'])
            self.protocol = TVSerialProtocol()
        except Exception as e:
            QMessageBox.critical(self, "初始化失败", f"无法连接 MinIO 或初始化协议: {e}")
            sys.exit(1)
        
        # 绑定信号
        self.monitor_data_signal.connect(self.update_serial_monitor)
        
        self.init_ui()

    def init_ui(self):
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        main_h_layout = QHBoxLayout(main_widget) # 使用水平布局拆分操作区和监控区

        # --- 左侧：操作面板 ---
        left_panel = QWidget()
        left_layout = QVBoxLayout(left_panel)
        
        # 1. 硬件配置
        hw_group = QGroupBox("1. 硬件连接配置")
        hw_layout = QHBoxLayout()
        self.port_combo = QComboBox()
        self.btn_refresh_port = QPushButton("刷新")
        self.btn_refresh_port.clicked.connect(self.refresh_serial_ports)
        hw_layout.addWidget(QLabel("端口:"))
        hw_layout.addWidget(self.port_combo, 1)
        hw_layout.addWidget(self.btn_refresh_port)
        hw_group.setLayout(hw_layout)
        left_layout.addWidget(hw_group)

        # 2. 任务勾选
        task_group = QGroupBox("2. 烧录任务")
        task_layout = QVBoxLayout()
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

        # 3. SN 录入与按钮
        self.sn_input = QLineEdit()
        self.sn_input.setPlaceholderText("在此录入 SN...")
        self.sn_input.setFont(QFont("Consolas", 14))
        self.btn_start = QPushButton("开始烧录")
        self.btn_start.setFixedHeight(50)
        self.btn_start.setStyleSheet("background-color: #27ae60; color: white; font-weight: bold;")
        self.btn_start.clicked.connect(self.run_process)
        left_layout.addWidget(QLabel("SN 输入:"))
        left_layout.addWidget(self.sn_input)
        left_layout.addWidget(self.btn_start)

        # 4. 流程日志
        self.log_view = QPlainTextEdit()
        self.log_view.setReadOnly(True)
        left_layout.addWidget(QLabel("流程状态:"))
        left_layout.addWidget(self.log_view)
        
        main_h_layout.addWidget(left_panel, 2)

        # --- 右侧：串口实时监控区 ---
        right_panel = QGroupBox("串口数据监控 (Serial Monitor)")
        right_layout = QVBoxLayout(right_panel)
        
        # 监控控制工具栏
        monitor_tools = QHBoxLayout()
        self.check_hex_show = QCheckBox("16进制显示 (Hex)")
        self.check_hex_show.setChecked(True)
        btn_clear_monitor = QPushButton("清除窗口")
        btn_clear_monitor.clicked.connect(lambda: self.serial_monitor.clear())
        monitor_tools.addWidget(self.check_hex_show)
        monitor_tools.addStretch()
        monitor_tools.addWidget(btn_clear_monitor)
        right_layout.addLayout(monitor_tools)

        self.serial_monitor = QPlainTextEdit()
        self.serial_monitor.setReadOnly(True)
        self.serial_monitor.setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: 'Consolas';")
        right_layout.addWidget(self.serial_monitor)
        
        main_h_layout.addWidget(right_panel, 3) # 监控区占比较大

        self.refresh_serial_ports()

    def update_serial_monitor(self, data_bytes):
        """更新右侧监控窗口的内容"""
        if self.check_hex_show.isChecked():
            text = " ".join([f"{b:02X}" for b in data_bytes])
        else:
            text = "".join([chr(b) if 32 <= b <= 126 or b in [10, 13] else "." for b in data_bytes])
        
        self.serial_monitor.appendPlainText(f"[RECV] {text}")
        
        # 强制界面刷新
        QApplication.processEvents() 
        
        # 确保滚动到底部
        self.serial_monitor.verticalScrollBar().setValue(self.serial_monitor.verticalScrollBar().maximum())

    def load_local_config(self):
        """从本地 config.json 加载配置"""
        try:
            if not os.path.exists("config.json"):
                # 提供默认模板
                default_cfg = {
                    "minio": {"endpoint": "127.0.0.1:9000", "access_key": "minioadmin", "secret_key": "minioadmin", "bucket": "warehouse"},
                    "key_types": ["HDCP1.4", "HDCP2.2", "ULPK"]
                }
                with open("config.json", "w") as f:
                    json.dump(default_cfg, f, indent=4)
                return default_cfg
            
            with open("config.json", "r") as f:
                return json.load(f)
        except Exception as e:
            QMessageBox.critical(self, "配置错误", f"解析 config.json 失败: {e}")
            sys.exit(1)

    def refresh_serial_ports(self):
        """动态扫描可用 COM 口"""
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        
        if not ports:
            self.port_combo.addItem("未发现可用串口", None)
            self.btn_start.setEnabled(False)
            return

        for p in ports:
            display_name = f"{p.device} ({p.description})"
            self.port_combo.addItem(display_name, p.device)
        self.btn_start.setEnabled(True)

    def on_port_selection_changed(self, index):
        """下拉框切换时同步更新协议对象"""
        port_device = self.port_combo.itemData(index)
        if port_device:
            self.protocol.port = port_device

    def log(self, message, color_hex="#ffffff"):
        """彩色日志输出并自动滚动到底部"""
        fmt = f'<span style="color:{color_hex};">{message}</span>'
        self.log_view.appendHtml(fmt)
        # 自动滚动
        self.log_view.verticalScrollBar().setValue(self.log_view.verticalScrollBar().maximum())

    def run_process(self):
        """执行烧录流水线"""
        # 1. 串口检查
        current_port = self.port_combo.itemData(self.port_combo.currentIndex())
        if not current_port:
            QMessageBox.warning(self, "硬件错误", "请先连接串口并点击刷新！")
            return

        # 2. SN 检查
        sn = self.sn_input.text().strip()
        if not sn:
            QMessageBox.warning(self, "提醒", "请输入或扫描机器 SN 序列号！")
            self.sn_input.setFocus()
            return

        # 3. 任务列表构建
        tasks = []
        if self.check_mac.isChecked(): 
            tasks.append("mac")
        for kt, cb in self.key_checks.items():
            if cb.isChecked(): 
                tasks.append(f"key/{kt}")

        if not tasks:
            QMessageBox.warning(self, "任务错误", "请至少勾选一项要烧录的内容！")
            return

        # 4. 锁定 UI 并启动工作线程
        self.btn_start.setEnabled(False)
        self.sn_input.setEnabled(False)
        self.log_view.clear()
        
        self.protocol.port = current_port # 确保最新

        # 实例化 BurnWorker 并传入 config
        self.worker = BurnWorker(self.db, self.protocol, sn, tasks, self.config)

        
        # 关键：我们需要在 BurnWorker 的 burn 逻辑中调用这个信号
        # 您可以修改 BurnWorker 让它能访问到这个信号
        self.worker.monitor_signal = self.monitor_data_signal 

        self.worker.log_signal.connect(self.log)
        self.worker.result_signal.connect(self.on_finished)
        self.worker.start()

    def on_finished(self, success, msg):
        """任务结束处理"""
        self.btn_start.setEnabled(True)
        self.sn_input.setEnabled(True)
        self.sn_input.clear()
        self.sn_input.setFocus()

        if success:
            self.log(f"\n[PASS] {msg}", "#2ecc71")
            QMessageBox.information(self, "烧录成功", f"SN: {self.worker.sn} 烧录完成！")
        else:
            self.log(f"\n[FAIL] {msg}", "#e74c3c")
            QMessageBox.critical(self, "烧录失败", msg)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = OfflineProdWindow()
    window.show()
    sys.exit(app.exec())