import os
import io
import json
import socket
from minio import Minio
from minio.error import S3Error
from minio.commonconfig import CopySource

class MinioWarehouse:
    def __init__(self, endpoint, access_key, secret_key, bucket="warehouse"):
        self.client = Minio(endpoint, access_key=access_key, secret_key=secret_key, secure=False)
        self.bucket = bucket
        self.endpoint = endpoint
        self.bucket_ready = False

    def check_network_connection(self):
        """检查网络连接是否可用"""
        try:
            # 解析endpoint
            if ":" in self.endpoint:
                host, port_str = self.endpoint.split(":", 1)
                port = int(port_str)
            else:
                host = self.endpoint
                port = 80  # 默认端口

            # 尝试连接
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(2)  # 缩短超时时间到2秒
            result = sock.connect_ex((host, port))
            sock.close()
            return result == 0
        except Exception:
            return False

    def _ensure_bucket(self):
        if not self.bucket_ready:
            if not self.client.bucket_exists(self.bucket):
                self.client.make_bucket(self.bucket)
            self.bucket_ready = True

    def check_exists(self, res_type, value):
        """检查资源是否在仓库中（无论是否使用）"""
        self._ensure_bucket()
        for status in ["available", "used"]:
            object_name = f"{res_type}/{status}/{value}.json"
            try:
                self.client.stat_object(self.bucket, object_name)
                return True
            except:
                continue
        return False

    def upload_new_resource(self, res_type, value, metadata=None):
        """管理员上传新资源"""
        self._ensure_bucket()
        if self.check_exists(res_type, value):
            return False, f"{value} 已存在，不可重复导入"
        path = f"{res_type}/available/{value}.json"
        data = json.dumps(metadata or {"status": "unused"}).encode('utf-8')
        self.client.put_object(self.bucket, path, io.BytesIO(data), len(data))
        return True, "上传成功"

    def upload_binary_resource(self, res_type, res_id, data_bytes):
        """直接上传二进制字节流"""
        self._ensure_bucket()
        if self.check_exists(res_type, res_id):
            return False, f"{res_id} 已存在"
        object_name = f"{res_type}/available/{res_id}"
        data_stream = io.BytesIO(data_bytes)
        self.client.put_object(
            self.bucket, object_name, data_stream, len(data_bytes),
            content_type="application/octet-stream"
        )
        return True, "上传成功"

    def lease_resource(self, res_type):
        """锁定领取一个 available 资源：将其移入 used 并返回文件名。

        先锁定后烧录，可将并发双领窗口缩小到 copy+remove 之间；
        以 remove 成功作为锁定判定：remove 失败视为未锁定，回滚本工位刚写入的
        used 副本并尝试下一个候选。多个烧录工位共享仓库时，仍建议后续引入
        分布式锁或 MinIO 条件拷贝(If-None-Match)进一步保证互斥。
        """
        self._ensure_bucket()
        res_type = res_type.lower()
        prefix = f"{res_type}/available/"
        try:
            objects = self.client.list_objects(self.bucket, prefix=prefix, recursive=True)
        except Exception as e:
            print(f"列举资源失败: {e}")
            return None
        for obj in objects:
            filename = os.path.basename(obj.object_name)
            src = f"{res_type}/available/{filename}"
            dst = f"{res_type}/used/{filename}"
            try:
                source = CopySource(self.bucket, src)
                self.client.copy_object(self.bucket, dst, source)
                try:
                    self.client.remove_object(self.bucket, src)
                    return filename
                except Exception as e:
                    # 源未移除成功：网络抖动或被并发工位抢先移除，回滚本工位刚写入的副本
                    print(f"锁定 {filename} 的源移除失败: {e}")
                    try:
                        self.client.remove_object(self.bucket, dst)
                    except Exception:
                        pass
            except Exception as e:
                # copy 失败(通常源已被其他工位移走)，继续尝试下一个候选
                print(f"锁定资源 {filename} 失败: {e}")
        return None

    def peek_available(self, res_type):
        """只获取第一个 available 资源的文件名，不移动"""
        self._ensure_bucket()
        res_type = res_type.lower()
        prefix = f"{res_type}/available/"
        objects = self.client.list_objects(self.bucket, prefix=prefix, recursive=True)
        for obj in objects:
            return os.path.basename(obj.object_name)
        return None

    def move_to_used(self, res_type, filename):
        """将指定资源从 available 移到 used"""
        self._ensure_bucket()
        res_type = res_type.lower()
        src = f"{res_type}/available/{filename}"
        dst = f"{res_type}/used/{filename}"
        try:
            source = CopySource(self.bucket, src)
            self.client.copy_object(self.bucket, dst, source)
            self.client.remove_object(self.bucket, src)
            return True
        except Exception as e:
            print(f"移动资源失败: {e}")
            return False

    def restore_available(self, res_type, filename):
        """烧录失败补偿：将已锁定(used)的资源回移到 available"""
        self._ensure_bucket()
        res_type = res_type.lower()
        src = f"{res_type}/used/{filename}"
        dst = f"{res_type}/available/{filename}"
        try:
            source = CopySource(self.bucket, src)
            self.client.copy_object(self.bucket, dst, source)
            self.client.remove_object(self.bucket, src)
            return True
        except Exception as e:
            print(f"回移资源失败: {e}")
            return False
