import sys
import os
import json
import io
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QLineEdit, QPushButton, QTableWidget, 
                             QTableWidgetItem, QHeaderView, QSpinBox, QLabel, 
                             QMessageBox, QTabWidget, QComboBox, QFileDialog,
                             QPlainTextEdit) # <--- 确保加上了这一项
from PyQt6.QtCore import Qt

# 导入 Task 1 的 MinIO 封装类
try:
    from minio_db import MinioWarehouse
except ImportError:
    # 如果还没创建 minio_db.py，请参考之前的 Task 1 代码
    class MinioWarehouse: pass 

def load_config():
    """加载本地配置文件"""
    default_config = {
        "key_types": ["HDCP", "Widevine"],
        "minio": {"endpoint": "127.0.0.1:9000", "access_key": "minioadmin", "secret_key": "minioadmin", "bucket": "warehouse"}
    }
    if os.path.exists("config.json"):
        try:
            with open("config.json", "r", encoding="utf-8") as f:
                return json.load(f)
        except:
            return default_config
    return default_config

class AdminWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.config = load_config()
        self.setWindowTitle("远程仓库管理端 (MinIO Based) - v2.0")
        self.resize(1000, 700)
        
        # 初始化 MinIO
        m_cfg = self.config.get("minio", {})
        self.db = MinioWarehouse(
            endpoint=m_cfg.get("endpoint", "127.0.0.1:9000"),
            access_key=m_cfg.get("access_key", "minioadmin"),
            secret_key=m_cfg.get("secret_key", "minioadmin"),
            bucket=m_cfg.get("bucket", "warehouse")
        )
        
        self.init_ui()

    def init_ui(self):
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)

        self.tabs = QTabWidget()
        self.tabs.addTab(self.create_import_tab(), "资源导入")
        self.tabs.addTab(self.create_view_tab(), "库存查询")
        self.tabs.addTab(self.create_trace_tab(), "SN 追溯") # 新增 Tab
        main_layout.addWidget(self.tabs)

    def create_import_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # --- MAC 批量导入区 ---
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
        btn_mac.clicked.connect(self.handle_batch_mac_upload)
        
        mac_layout.addWidget(self.start_mac_input)
        mac_layout.addWidget(QLabel("数量:"))
        mac_layout.addWidget(self.mac_count_input)
        mac_layout.addWidget(btn_mac)
        layout.addLayout(mac_layout)

        layout.addSpacing(30)
        layout.setContentsMargins(20, 20, 20, 20)

        # --- Key 文件导入区 ---
        layout.addWidget(QLabel("<b>[2] Key 资源上传</b>"))
        
        type_select_layout = QHBoxLayout()
        type_select_layout.addWidget(QLabel("Key 类型:"))
        self.key_type_select = QComboBox()
        self.key_type_select.addItems(self.config.get("key_types", []))
        self.key_type_select.setEditable(True) 
        type_select_layout.addWidget(self.key_type_select, 1)
        layout.addLayout(type_select_layout)

        # ULPK 客户选择（ULPK 按客户分目录存储）
        key_client_layout = QHBoxLayout()
        key_client_layout.addWidget(QLabel("客户 (ULPK):"))
        self.key_client_select = QComboBox()
        self.key_client_select.addItems(self.config.get("mac_clients", ["Vizio", "Onn"]))
        key_client_layout.addWidget(self.key_client_select, 1)
        layout.addLayout(key_client_layout)

        key_btn_layout = QHBoxLayout()
        btn_single = QPushButton("选择单个文件上传")
        btn_single.clicked.connect(lambda: self.handle_key_upload(is_batch=False))
        btn_batch = QPushButton("选择文件夹批量上传")
        btn_batch.clicked.connect(lambda: self.handle_key_upload(is_batch=True))
        key_btn_layout.addWidget(btn_single)
        key_btn_layout.addWidget(btn_batch)
        layout.addLayout(key_btn_layout)

        layout.addStretch()
        return widget

    def create_view_tab(self):
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
        btn_refresh.clicked.connect(self.refresh_inventory)
        
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

    def create_trace_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # 搜索栏
        search_layout = QHBoxLayout()
        self.trace_sn_input = QLineEdit()
        self.trace_sn_input.setPlaceholderText("请输入要查询的机器 SN 序列号")
        self.trace_sn_input.returnPressed.connect(self.query_sn_history) # 支持回车查询
        
        btn_search = QPushButton("查询绑定关系")
        btn_search.clicked.connect(self.query_sn_history)
        btn_search.setStyleSheet("background-color: #27ae60; color: white; padding: 5px 15px;")

        search_layout.addWidget(self.trace_sn_input)
        search_layout.addWidget(btn_search)
        layout.addLayout(search_layout)

        # 结果显示区
        layout.addWidget(QLabel("<b>绑定明细：</b>"))
        self.trace_result_table = QTableWidget(0, 2)
        self.trace_result_table.setHorizontalHeaderLabels(["资源类型", "绑定的资源 ID / 值"])
        self.trace_result_table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        layout.addWidget(self.trace_result_table)

        # 原始 JSON 查看（可选，用于高级排错）
        self.raw_json_view = QPlainTextEdit()
        self.raw_json_view.setReadOnly(True)
        self.raw_json_view.setMaximumHeight(150)
        self.raw_json_view.setStyleSheet("background: #f4f4f4; font-family: Consolas;")
        layout.addWidget(QLabel("原始归档数据:"))
        layout.addWidget(self.raw_json_view)

        return widget
    
    # --- 逻辑处理 ---

    def normalize_mac(self, mac_str):
        """归一化 MAC 格式为 XX-XX-XX-XX-XX-XX"""
        clean = mac_str.replace(":", "").replace("-", "").strip().upper()
        if len(clean) != 12:
            raise ValueError("MAC 地址必须为 12 位十六进制字符")
        return "-".join(clean[i:i+2] for i in range(0, 12, 2))

    def handle_batch_mac_upload(self):
        raw_mac = self.start_mac_input.text().strip()
        count = self.mac_count_input.value()
        client_name = self.mac_client_select.currentText().strip().lower()
        try:
            std_mac = self.normalize_mac(raw_mac)
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

    def handle_key_upload(self, is_batch=False):
        key_type = self.key_type_select.currentText().strip()
        if not key_type: return
        
        # ULPK 按客户分目录
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
                # ... 提示信息 ...
                if success: 
                    success_count = 1
                    QMessageBox.information(self, "成功", f"Key {fname} 已上传")
                else: QMessageBox.warning(self, "失败", msg)
            except Exception as e: QMessageBox.critical(self, "错误", str(e))

    def refresh_inventory(self):
        self.table.setRowCount(0)
        sel_type = self.type_filter.currentText()
        sel_stat = self.status_filter.currentText()
        
        # 确定前缀
        prefixes = []
        if sel_type == "全部": prefixes = ["mac/", "key/"]
        elif sel_type == "MAC (全部)": prefixes = ["mac/"]
        elif sel_type.startswith("MAC ("):
            client_name = sel_type.replace("MAC (", "").replace(")", "").strip().lower()
            prefixes = [f"mac/{client_name}/"]
        else: prefixes = [f"key/{sel_type}/"]

        # 状态关键词过滤
        stat_keywords = []
        if "待使用" in sel_stat: stat_keywords = ["available"]
        elif "已使用" in sel_stat: stat_keywords = ["used"]
        else: stat_keywords = ["available", "used"]

        for p in prefixes:
            try:
                # 递归列出前缀下所有对象（含客户子目录）
                objs = self.db.client.list_objects(self.db.bucket, prefix=p, recursive=True)
                for obj in objs:
                    # 按状态过滤：路径中必须包含 available 或 used
                    if any(f"/{kw}/" in obj.object_name for kw in stat_keywords):
                        self._add_row(obj.object_name)
            except: continue

    def _add_row(self, path):
        # 路径格式:
        #   mac/vizio/available/xx.json
        #   key/ULPK 5586F prod/vizio/available/xx.dat
        #   key/HDCP1.4 5586 prod/available/xx.bin
        parts = path.split('/')
        if len(parts) < 3: return
        
        row = self.table.rowCount()
        self.table.insertRow(row)
        
        main_type = parts[0].upper()
        status = "已使用" if "used" in path else "待使用"
        res_id = os.path.basename(path).split('.')[0]
        
        # 提取子类型和客户
        mac_clients = [c.lower() for c in self.config.get("mac_clients", ["Vizio", "Onn"])]
        client = ""
        if parts[0] == "mac":
            sub_type = "网络地址"
            if len(parts) >= 4 and parts[1].lower() in mac_clients:
                client = parts[1].capitalize()
        else:
            sub_type = parts[1]
            # 检查路径中是否有客户名（ULPK 等）
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


    
    def query_sn_history(self):
        sn = self.trace_sn_input.text().strip()
        if not sn:
            return

        self.trace_result_table.setRowCount(0)
        self.raw_json_view.clear()

        # 兼容两种路径：新版直接写 sn_record/{sn}.json，旧版走 available 目录
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
                except:
                    continue
            
            if data is None:
                raise FileNotFoundError("未找到")

            # 显示原始 JSON
            self.raw_json_view.setPlainText(json.dumps(data, indent=4, ensure_ascii=False))

            # 解析 burn_results 字典并填入表格
            results = data.get("burn_results", {})
            for key_type, res_id in results.items():
                row = self.trace_result_table.rowCount()
                self.trace_result_table.insertRow(row)
                self.trace_result_table.setItem(row, 0, QTableWidgetItem(key_type))
                self.trace_result_table.setItem(row, 1, QTableWidgetItem(str(res_id)))

        except Exception as e:
            # 如果文件不存在，stat_object 或 get_object 会抛错
            QMessageBox.warning(self, "未找到记录", f"在仓库中未发现 SN 为 [{sn}] 的烧录记录。\n请检查输入是否正确。")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = AdminWindow()
    window.show()
    sys.exit(app.exec())