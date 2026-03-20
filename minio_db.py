import os  # 新增
import io
import json
from minio import Minio
from minio.error import S3Error
from minio.commonconfig import CopySource

class MinioWarehouse:
    def __init__(self, endpoint, access_key, secret_key, bucket="warehouse"):
        self.client = Minio(endpoint, access_key=access_key, secret_key=secret_key, secure=False)
        self.bucket = bucket
        self._ensure_bucket()

    def _ensure_bucket(self):
        if not self.client.bucket_exists(self.bucket):
            self.client.make_bucket(self.bucket)

    def check_exists(self, res_type, value):
        """检查资源是否在仓库中（无论是否使用）"""
        # 检查 available 和 used 两个目录
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
        if self.check_exists(res_type, value):
            return False, f"{value} 已存在，不可重复导入"
        
        path = f"{res_type}/available/{value}.json"
        data = json.dumps(metadata or {"status": "unused"}).encode('utf-8')
        self.client.put_object(self.bucket, path, io.BytesIO(data), len(data))
        return True, "上传成功"

    def upload_binary_resource(self, res_type, res_id, data_bytes):
        """
        直接上传二进制字节流
        res_type: 比如 'key/HDCP2.2'
        res_id: 文件名
        data_bytes: 原始 bytes 数据
        """
        if self.check_exists(res_type, res_id):
            return False, f"{res_id} 已存在"

        object_name = f"{res_type}/available/{res_id}" # 建议不加 .json 后缀，保持原扩展名
        
        # 使用 io.BytesIO 包装字节流
        data_stream = io.BytesIO(data_bytes)
        self.client.put_object(
            self.bucket, 
            object_name, 
            data_stream, 
            len(data_bytes),
            content_type="application/octet-stream" # 标记为二进制流
        )
        return True, "上传成功"

    def fetch_and_lock(self, res_type):
        """从 available 移动到 used (5586 专用逻辑)"""
        res_type = res_type.lower()
        prefix = f"{res_type}/available/"
        
        #objects = self.client.list_objects(self.bucket, prefix=prefix, recursive=False)
        # recursive=True 可以增加容错性，防止文件被误放在子目录下
        objects = self.client.list_objects(self.bucket, prefix=prefix, recursive=True)

        found_any = False
        for obj in objects:
            found_any = True
            old_path = obj.object_name
            val = os.path.basename(old_path) # 获取文件名
            new_path = f"{res_type}/used/{val}"
            
            try:
                # 修正点：构造 CopySource 对象
                source = CopySource(self.bucket, old_path)
                
                # 执行原子拷贝
                self.client.copy_object(self.bucket, new_path, source)
                
                # 拷贝成功后删除原文件
                self.client.remove_object(self.bucket, old_path)
                return val # 返回文件名作为 res_id
            except Exception as e:
                print(f"锁定资源失败: {e}")
                continue 
        if not found_any:
            print(f"Debug: 在路径 {prefix} 下没找到任何文件")
        return None
    
    def fetch_and_lock_old1(self, res_type):
        """
        生产端获取一个可用资源并立即移动到 used 目录 (防止重领)
        """
        prefix = f"{res_type}/available/"
        objects = self.client.list_objects(self.bucket, prefix=prefix, recursive=False)
        
        for obj in objects:
            old_path = obj.object_name
            val = old_path.split('/')[-1].replace('.json', '')
            new_path = f"{res_type}/used/{val}.json"
            
            try:
                # 原子操作：先拷贝到 used，再删除 available
                self.client.copy_object(self.bucket, new_path, f"{self.bucket}/{old_path}")
                self.client.remove_object(self.bucket, old_path)
                return val # 成功锁定并获取
            except S3Error:
                continue # 如果被别人抢先移动了，尝试下一个
        return None