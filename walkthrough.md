# 项目工作流与技能配置文档 (Walkthrough)

## 项目概述
本项目已配置 Antigravity 全局 UI 设计技能 `frontend-design`，用于规范与提升前端界面生成的审美水准，消除模板化 AI 界面风格。

---

## 架构与工作流程

```mermaid
graph TD
    A[用户提出 UI/前端开发需求] --> B[Antigravity 识别任务]
    B --> C[自动读取 frontend-design SKILL]
    C --> D[执行审美定位与场景解构]
    D --> E[输出高质感、个性化前端代码]
```

---

## 已安装技能速查 (Skills Registry)

| 技能名称 | 存储路径 | 核心功能说明 |
| :--- | :--- | :--- |
| **`frontend-design`** | `C:\Users\Administrator\.gemini\config\skills\frontend-design\SKILL.md` | 提供定制化视觉设计规范，防止模板化设计，优化色彩、字体排版、动态交互与布局逻辑。 |

---

## 变更历史记录

### 2026-07-30
- **操作内容**：成功安装 Anthropic 官方 `frontend-design` 技能到 Antigravity 全局技能目录。
- **关联文件**：[SKILL.md](file:///C:/Users/Administrator/.gemini/config/skills/frontend-design/SKILL.md)

- **UI 优化重构**：基于 `@frontend-design` 技能规范，为 [`admin_client.py`](file:///d:/AI/MyRD_VIZIO_KEY_Genimi/admin_client.py) 注入 `MODERN_INDUSTRIAL_QSS` 工业级深色视觉控制台主题。
- **界面改进细节**：
  1. **色彩与质感**：背景升级为 Slate 深色系 (`#0f172a` / `#1e293b`)，搭配 `#38bdf8` 气泡蓝与 `#34d399` 翡翠绿发光指示。
  2. **组件重置**：优化 `QTabWidget` 胶囊卡片选项卡、`QGroupBox` 微发光边框、`QTableWidget` 暗黑隔行相间表格。
  3. **交互聚焦**：重构“🚀 开始全功能烧录”按钮，增加渐变高亮与悬浮态效果；为 SN 输入框增加深色高对比 Console 字体。

### 2026-08-25 (Novatek 方案支持与 Key 区分设计)

#### 1. 需求与设计概述
- 为烧录工具新增对 **Novatek (Novatech)** 芯片方案的支持，与现有的 **MTK (5586)** 方案进行隔离管理。
- 采用 **分层平台配置 + 动态 UI 联动** 架构（方案 A）。

#### 2. 系统架构与数据流

```mermaid
graph TD
    Config[config.json 分层配置 platforms] --> UI[UI 芯片方案选择器 Platform Combo]
    UI -->|切换 MTK / Novatek| DynamicKey[动态刷新 Key 复选框与库存列表]
    DynamicKey --> Worker[BurnWorker 烧录工作线程]
    Worker -->|串口通用协议| TVProtocol[TVSerialProtocol 协议引擎]
    Worker -->|存储/归档 sn_record| MinIO[MinIO Key 仓库与记录]
```

#### 3. 决策记录 (Decision Log)

| 决策项 | 选定方案 | 决策依据 |
| :--- | :--- | :--- |
| **多方案架构支持** | 方案 A：分层平台配置 + 动态 UI 联动 | 具备高度扩展性与防呆性，兼顾代码复用与向后兼容。 |
| **通信协议复用** | 复用 `TVSerialProtocol` 底层帧与校验引擎 | 经确认 Novatek 协议帧格式与 MTK 一致，保持单一职责。 |
| **存储路径规范** | `key/<Key Type>/available\|used/`（在 Key 类型中体现方案） | 保持现有 MinIO 路径解析逻辑完全兼容，无需迁移旧数据。 |
| **MTK 仓库保护** | 严格保持 MTK 现有路径与命名不变，默认平台为 MTK | 零迁移、零改动现有 MTK 资源桶，彻底消除产线退化风险。 |

#### 4. 实施变更与关键函数速查

- **[config.json](file:///d:/AI/MyRD_VIZIO_KEY_Genimi/config.json)**：
  - 引入 `platforms` 分层字典（`MTK` 与 `Novatek`），支持定义各自专属的 `key_types`；
  - 预设 `default_platform: "MTK"`。
- **[main.py](file:///d:/AI/MyRD_VIZIO_KEY_Genimi/main.py)**：
  - `load_config()`：升级支持多方案层级解析并向上兼容旧版单层 `key_types`。
  - `get_platforms(config)` / `get_platform_key_types(config, platform)` / `get_default_platform(config)` / `get_key_platform(config, key_type)`：提供方案与 Key 反查工具函数。
  - `BurnWorker`：增加 `platform` 入参，格式化输出方案标识日志，并在 `sn_record/{sn}.json` 中归档方案信息。
  - `_create_burn_tab()` / `_rebuild_key_checks()` / `_on_burn_platform_changed()`：实现生产烧录界面芯片方案下拉联动与 Key 复选框动态生成。
  - `_create_import_tab()` / `_on_import_platform_changed()`：实现资源导入页面按方案筛选 Key 类型。
  - `_create_view_tab()` / `_on_view_platform_changed()` / `_update_view_type_filter()` / `_refresh_inventory()` / `_add_row()`：库存查询页面支持按方案（全部方案 / MTK / Novatek）筛选类别与过滤资源，并在表格首列明确展示所属芯片方案（MTK / Novatek / 通用）。
- **[tv_protocol.py](file:///d:/AI/MyRD_VIZIO_KEY_Genimi/tv_protocol.py)**：
  - `build_command()`：帧长字段 `total_len = len(payload) + 7` 纯动态计算，自适应不同长度的 ULPK 数据。
  - `pack_ulpk_command()`：增加 UID 转换异常捕获容错，防止非数字文件名导致运行时异常。

#### 5. 验证结果 (Verification)
- **配置解析与兼容性验证**：新版 `platforms` 与旧版 `key_types` 均可正常解析，默认方案为 `MTK`。
- **UI 联动验证**：
  - 烧录页面切换 `Novatek` 时，Key 勾选列表动态更新；切回 `MTK` 时自动恢复为 MTK Key。
  - 导入页面切换方案时，Key 类型下拉框即时刷新对应列表。
- **库存查询多方案验证**：
  - 顶部增加“芯片方案”下拉筛选框（`全部方案` / `MTK` / `Novatek`），切换方案时类别下拉框自动同步联动；
  - 表格新增“芯片方案”列，MTK 资源显示 `MTK`，Novatek 资源显示 `Novatek`，MAC 地址显示 `通用`。
- **ULPK 动态长度与容错验证**：
  - ULPK 长度在 244 字节以内均可自动根据实际文件大小计算帧长与 CRC16，无硬编码限制。
- **MTK 仓库保护验证**：原有 MTK 存储路径 `key/HDCP1.4 5586 dev/...` 保持完全一致，无任何破坏性变动。








