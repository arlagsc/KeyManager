import serial
import time
import struct

class TVSerialProtocol:
    CRC_TABLE = [
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF
    ]

    # 与 MFC 一致的常量
    HDCP14_BLOCK_SIZE = 120   # MFC: HDCP14_SIZE = 120
    HDCP22_BLOCK_SIZE = 128   # MFC: BLOCK_SIZE = 128
    HDCP22_BLOCK_NO = 9       # MFC: BLOCK_NO = 9
    HDCP22_TOTAL_SIZE = 1044  # MFC: HDCP22_TOTAL_SIZE = 1044

    def __init__(self, port="COM3", baudrate=115200):
        self.port = port
        self.baud = baudrate
        self.ser = None

    def calculate_crc(self, data: bytes):
        """CRC16 校验，与 MFC calculateCRC_ROKU 一致"""
        checksum = 0xFFFF
        for byte in data:
            uc_temp = (checksum >> 12) & 0x0F
            checksum = ((checksum << 4) & 0xFFFF) ^ self.CRC_TABLE[uc_temp ^ (byte >> 4)]
            uc_temp = (checksum >> 12) & 0x0F
            checksum = ((checksum << 4) & 0xFFFF) ^ self.CRC_TABLE[uc_temp ^ (byte & 0x0F)]
        return checksum & 0xFFFF

    def send_raw(self, data, monitor_signal=None):
        """执行物理发送，将收到的原始数据回传给 UI 监控窗"""
        try:
            if not self.ser or not self.ser.is_open:
                self.ser = serial.Serial(self.port, self.baud, timeout=2)
            self.ser.reset_input_buffer()
            self.ser.write(data)
            time.sleep(0.6)
            res = self.ser.read_all()
            if monitor_signal and len(res) > 0:
                monitor_signal.emit(res)
            success = (b"\x03\x0C\xF1" in res or b"\x03\x0C\xF2" in res)
            return success, res.hex().upper()
        except Exception as e:
            return False, str(e)

    def send_and_wait_ack(self, data, monitor_signal=None, max_retries=10, ack_delay=0.3):
        """
        发送并等待 ACK，与 MFC Timer 逐包确认逻辑一致。
        返回 (success, ack_type, raw_hex)
        ack_type: 'F1' / 'F2' / 'ERROR' / None(超时)
        """
        try:
            if not self.ser or not self.ser.is_open:
                self.ser = serial.Serial(self.port, self.baud, timeout=2)
            self.ser.reset_input_buffer()
            self.ser.write(data)
            for _ in range(max_retries):
                time.sleep(ack_delay)
                res = self.ser.read_all()
                if monitor_signal and len(res) > 0:
                    monitor_signal.emit(res)
                if b"\x03\x0C\xE0" in res or b"\x03\x0C\xE1" in res or b"\x03\x0C\xE2" in res:
                    return False, "ERROR", res.hex().upper()
                if b"\x03\x0C\xF1" in res:
                    return True, "F1", res.hex().upper()
                if b"\x03\x0C\xF2" in res:
                    return True, "F2", res.hex().upper()
            return False, None, "TIMEOUT"
        except Exception as e:
            return False, None, str(e)

    # ========== 帧构建方法 ==========

    def build_command(self, cmd_id, payload: bytes):
        """
        构建 5586 通用指令帧 (SN/MAC/ULPK 等)。
        帧格式: 07 51 01 [帧总长度] [cmd_id] [payload] [CRC16]
        
        MFC 参考 (MAC): "07 51 01 0D B3 [6字节MAC] 00 00" → clrCRC
        帧总长度 = 3 + 1 + 1 + len(payload) + 2 = len(payload) + 7
        """
        total_len = len(payload) + 7
        frame_no_crc = bytes([0x07, 0x51, 0x01, total_len, cmd_id]) + payload
        crc = self.calculate_crc(frame_no_crc)
        return frame_no_crc + struct.pack(">H", crc)

    def build_hdcp_frame(self, payload: bytes):
        """
        构建 HDCP 专用帧 (cmd=0xFE)。
        帧格式: 07 51 01 FE 00 [第6字节] [payload] [CRC16]
        
        MFC 参考 (Header):
          "07 51 01 FE 00 0F E3 00 03 00 78 [H] [L] 00 00" → clrCRC
          clrCRC 替换最后2字节为真实CRC，最终帧 15 字节，第6字节=0x0F=15
        MFC 参考 (数据包):
          "07 51 01 FE 00 [SIZE+10] E3 01 [120字节] 00 00" → clrCRC
          SIZE+10 = 120+10 = 130, 帧总长度 = 6+1+1+120+2 = 130
        
        结论: 第6字节 = 帧总长度 = 6(头) + len(payload) + 2(CRC)
        payload 不含尾部 00 00 占位（那是 MFC clrCRC 的占位，这里直接追加真实 CRC）
        """
        total_len = 6 + len(payload) + 2
        frame_no_crc = bytes([0x07, 0x51, 0x01, 0xFE, 0x00, total_len]) + payload
        crc = self.calculate_crc(frame_no_crc)
        return frame_no_crc + struct.pack(">H", crc)

    # ========== 高层指令打包 ==========

    def pack_sn_command(self, sn_string):
        """
        SN 烧录指令，cmd_id=0x88，SN 按 ASCII 逐字节编码。
        MFC: m_strSn = "07 51 01 16 88 " + [SN每字符→%02X] + "00 00" → clrCRC
        """
        sn_bytes = sn_string.encode('ascii')
        return self.build_command(0x88, sn_bytes)

    def pack_mac_command(self, mac_str):
        """
        MAC 烧录指令，cmd_id=0xB3，MAC 为 6 字节二进制。
        MFC: m_strMac = "07 51 01 0D B3 " + [MAC去掉分隔符] + "00 00" → clrCRC
        """
        clean = mac_str.replace("-", "").replace(":", "").replace(".json", "").strip()
        mac_bytes = bytes.fromhex(clean)
        return self.build_command(0xB3, mac_bytes)

    def pack_ulpk_command(self, uid_str, ulpk_data):
        """
        5586 ULPK 烧录指令，cmd_id=0xD1。
        MFC: strUlpkPotk = "07 51 01 AB D1 " + [UID 4字节] + [160字节data] + "00 00" → clrCRC
        注意: 0xAB 是帧总长度，0xD1 是 cmd_id。
        """
        uid_int = int(uid_str)
        uid_bytes = struct.pack(">I", uid_int)
        payload = uid_bytes + ulpk_data
        return self.build_command(0xD1, payload)

    def pack_hdcp_header(self, hdcp_type, total_blocks, block_size, data_crc):
        """
        HDCP Header 包。
        MFC: "07 51 01 FE 00 0F [type] 00 [blocks] [sizeH] [sizeL] [crcH] [crcL] 00 00" → clrCRC
        末尾 00 00 是 MFC clrCRC 的占位符，会被替换为真实 CRC。
        这里 payload 不含占位符，build_hdcp_frame 会追加真实 CRC。
        """
        payload = bytes([hdcp_type, 0x00, total_blocks]) + \
                  struct.pack(">H", block_size) + \
                  struct.pack(">H", data_crc)
        return self.build_hdcp_frame(payload)

    def pack_hdcp_chunk(self, hdcp_type, block_id, chunk_data):
        """
        HDCP 数据分包。
        MFC: "07 51 01 FE 00 [size+10] [type] [block_id] [data] 00 00" → clrCRC
        """
        payload = bytes([hdcp_type, block_id]) + chunk_data
        return self.build_hdcp_frame(payload)

    def pack_factory_mode(self):
        """进入工厂模式指令，固定帧"""
        return bytes.fromhex("07510107 01 CBD1".replace(" ", ""))
