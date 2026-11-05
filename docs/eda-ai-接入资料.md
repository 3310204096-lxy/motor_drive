# 嘉立创 EDA 专业版 · AI 接入资料索引

> 整理日期：2026-11-05　适用客户端：lceda-pro 4.1.97（国内版）

---

## 一、官方资料入口

| 资料 | 地址 | 说明 |
|---|---|---|
| 命令行客户端 CLI（中文） | `prodocs.lceda.cn/cn/api/guide/cli.html` | 主文档，11 章，含完整示例 |
| 命令行客户端 CLI（英文） | `prodocs.easyeda.com/en/api/guide/cli.html` | 同内容 |
| **For AI 版（纯文本）** | `prodocs.lceda.cn/storage/texts/api/guide/cli-for-ai.md` | 专为 Agent 编写，只有规则没有叙述 |
| 扩展 API 指南 | `prodocs.lceda.cn/cn/api/guide/index.html` | 教怎么写扩展（.eext） |
| 扩展 API 参考 | `prodocs.lceda.cn/cn/api/reference/pro-api.html` | 120+ 类、62 枚举、70 接口的完整索引 |
| GitHub 仓库 | `github.com/easyeda/easyeda-client-cli` | Apache-2.0，仅 3 个 md |
| MCP 协议 | `modelcontextprotocol.io` | CLI 可作为 MCP 服务器 |

GitHub 仓库里的三份文档：

| 文件 | 用途 |
|---|---|
| `easyeda-cli.md` | 双语概览：版本要求、能力、核心概念、运行时如何查帮助 |
| `cli-for-ai.md` | 面向 AI 的执行规范（发现式，不写死命令） |
| `easyeda-format.md` | `.eprj3` / `.epro2` 工程文件格式：日志文档、记录结构、单位体系 |

---

## 二、两份 API 文档的分工

| | 扩展 API **指南** | 扩展 API **参考** |
|---|---|---|
| 性质 | 教程 | 字典 |
| 回答的问题 | "怎么做" —— 建工程、写 manifest、调试、打包 | "有什么" —— 类、方法签名、参数、返回值 |
| 使用场景 | 要自己写一个插件（带 UI 面板的常驻扩展） | 查某个函数怎么调 |
| 是否 AI 接入必需 | **否** | **是** |

**关键认知**：让 AI 操作 EDA **不需要写扩展**。CLI 的 `invoke --ext-uuid eda` 直接借用编辑器内置的 `eda` 命名空间，不需要注册自己的扩展。只有想做功能完备的常驻插件，才需要看指南。

---

## 三、三条技术路线

| | 路线 A：CLI + invoke | 路线 B：CLI + MCP | 路线 C：直接写 .eprj3 |
|---|---|---|---|
| 前提 | 编辑器窗口开着 | 同 A | 不需编辑器 |
| 调用形式 | 命令行传 JS 代码 | AI 直接调工具 | 写文件 |
| 库文档 | 编辑器自带，可直接搜索 | 同 A | 需从导出中采集 |
| 校验能力 | 可实时跑 DRC / 网表 / BOM / 截图 | 同 A | 落盘后等重载再验 |
| 版本管理 | 二进制 / SQLite，不友好 | 同 A | 纯文本，便于 diff |
| 上手难度 | 低 | 低（封装了 A） | 中（需先学格式） |
| 适用场景 | 改现有工程、交互操作、自动化测试 | AI 日常操作 | 批量生成工程、代码评审、Git 管理 |

路线 B 本质是 A 的包装，不算独立技术路线。

### 官方对 A 与 C 的对比（原文要点）

| 维度 | 扩展 API 操作 | 直接编写 .eprj3 |
|---|---|---|
| 上手难度 | 低 | 中（需先了解格式规范） |
| 适用场景 | 交互式操作、校验、导出、自动化测试 | 批量生成工程、代码评审、Git 管理与 AI 直接产出工程 |
| 库文档 | 编辑器自带，可直接搜索 | 需从导出中采集 |
| 版本管理 | 二进制 / SQLite 格式不友好 | 纯文本，便于 diff |

**官方没有推荐二选一，而是推荐混用**：

```
新建工程(API) → 采集库文档(API + 读盘) → 生成业务记录(脚本/程序) → 落盘
      ↓                                                                  ↓
   校验(API: DRC / 网表 / BOM / 截图)  ←──────────────────────────  等待重载
```

---

## 四、实操要点（官方实践建议）

### A 路线：通过扩展 API 操作

1. `invoke` 时始终附加 `--ext-uuid eda`，确保拿到完整权限
2. 路径统一用正斜杠，写入前确认目标在 `APP_PROJECT_DIR` 内
3. 放置器件遵循：`create()` → 读属性快照 → `modify(designator + otherProperty)`，让位号和属性一次成型
4. 引脚坐标必须用 `getAllPinsByPrimitiveId()` 读取，**不要自己推算**
5. 器件多时分批，每批 20~25 个，给超时留余量
6. 改完等 5~10 秒再查 DRC / 网表 / 渲染
7. 每一步的关键返回值存成 JSON 留档
8. 收尾执行 `save()` 并在磁盘核对产物
9. 会话用完 `session close --destroy` 关闭，避免窗口堆积

### C 路线：直接生成工程文件

1. 工程骨架用 API 创建（`createProject()` + `openProject()`），别手写索引文件
2. 库文档从编辑器导出中采集，业务记录手写
3. 写文件前先用 `doc format` 逐字段读类型
4. 写盘三条规则：`partId` 指向 `PART` 行、`id` 用随机 16 位十六进制、`ticket` 文档内唯一
5. 行尾用 `|` + LF，外壳与载荷之间用 `||` 分隔
6. 落盘前先静态校验（引用完整性、正交性、id 与 ticket 唯一性）
7. 落盘后等 10~15 秒，复核对象数量、DRC 与截图，数量不符可重写一次
8. `.eprj3` 双向同步是即时的，可以直接读回磁盘文件当标准答案比对

### 命令行自检三件套

```bash
lceda-pro --help                 # 列全部命令，顺带打印版本
lceda-pro --search <关键词>       # 不知道命令名时模糊搜
lceda-pro <cmd> --help           # 每个命令都能自述
lceda-pro doc api --class-name X --method-name Y   # 查当前版本 API 签名（带示例）
```

---

## 五、本机环境记录

| 项 | 值 |
|---|---|
| 客户端路径 | `E:\produce\lceda-pro\lceda-pro.exe`（便携版） |
| 版本 | JLCEDA Pro 4.1.97.04b092ac（version 文件：v41.2.1-jlc-sa-win-x64.6） |
| CLI 要求 | V4.1.70+，当前版本满足 |
| 工程数据目录 | `%USERPROFILE%\Documents\LCEDA-Pro`，路径配置在 `config.json` 的 `APP_PROJECT_DIR` |

### 已确认的阻塞项

**WMIC 缺失导致 CLI 无法连编辑器**

```
$ lceda-pro activate
SPAWN_OWNER_FAILED: Failed to spawn the server copy:
spawn C:\WINDOWS\System32\wbem\WMIC.exe ENOENT
```

CLI 需要一个「服务器副本」进程，用 WMIC 去查和拉起它。WMIC 在 Windows 11 已被移除，所以 `doctor` 永远返回 `connected: false`。

修复：

```
DISM /Online /Enable-Feature /FeatureName:WMIC /All
```

或：设置 → 系统 → 可选功能 → 添加可选功能 → 搜索 `WMIC`。

---

## 六、踩坑记录

| 现象 | 原因 | 解法 |
|---|---|---|
| `curl` 访问本地端口失败，报 upstream connect failed | 系统设置了 `http_proxy`，请求被代理拦走 | 加 `--noproxy '*'`，或用 Python 的 `ProxyHandler({})` |
| shell 里嵌套引号导致 JSON 解析失败 | 多层转义 | 用 Python 脚本构造 JSON 再 POST |
| `lceda-pro.exe doctor` 报 `Cannot find module 'E:\doctor'` | 旧版本没有 CLI，exe 在命令行下就是纯 Node | 升级到 V4.1.70+ |
| `sch_PrimitiveComponent.modify()` 改网络名返回 500 | 网络名不在组件字段里 | 用 `sch_PrimitiveAttribute.getAllPrimitiveId(组件ID)` → 找 key 为 `Net` 的属性 → `modify(属性ID, {value:'新名'})` |
| 后台起的 Node 桥接进程几小时后消失 | 用 `&` 脱离任务管理会被回收 | 用受管后台任务启动 |
| `sch_Net.getAllNetsName()` 返回空 | 未打开原理图文档时无网络 | 先 `dmt_EditorControl.openDocument(pageUuid)` |
| `sch_Netlist.getNetlist()` 请求超时 | 大工程生成网表慢 | 改用引脚级 `noConnected` 标记 + 导线 `net` 字段判断连通性 |

### 检查连通性的可靠方法

```js
// 1. 引脚悬空标记（EDA 自己给的最可靠）
const pins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(compId);
pins.filter(p => p.noConnected).map(p => p.pinNumber + ':' + p.pinName);

// 2. 导线所属网络名
const wires = await eda.sch_PrimitiveWire.getAll();
wires.filter(w => w.net === 'VCC');

// 3. 网络标签统计（数量为 1 的要警惕单点悬空）
```

---

## 七、给 AI 的硬性约束（摘自 cli-for-ai.md）

1. 一切以已安装版本的 `--help` 为准，它和其他文档冲突时以它为准
2. 每条命令先看返回值 `ok` 字段，失败要读错误信息，不能当作成功继续
3. 承诺写文件的命令，事后要到磁盘核对产物
4. 需要用户决定的值（工程名、目标路径）先问，不要臆造
5. 路径一律用正斜杠
6. 只往工程目录里写
7. 长耗时操作先告知用户
8. 自己创建的会话用完要关闭
9. 需要点 UI 的步骤交给用户
