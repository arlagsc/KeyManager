def generate_mac_range(start_mac, count):
    """根据起始MAC和数量生成列表"""
    import struct
    # 去掉冒号转为整数
    mac_int = int(start_mac.replace(':', '').replace('-', ''), 16)
    mac_list = []
    for i in range(count):
        new_mac = hex(mac_int + i)[2:].zfill(12).upper()
        # 格式化为 00:11:22...
        formatted_mac = ":".join(new_mac[j:j+2] for j in range(0, 12, 2))
        mac_list.append(formatted_mac)
    return mac_list