---
name: confluence-doc
description: Confluence 文档编写技能——通过 API 在 Confluence 页面上编写文档、绘制图表、管理页面内容。支持 PlantUML/Mermaid/draw.io 三种绘图方法和完整的页面 CRUD 操作。触发：当用户要求"Confluence文档编写"、"输出文档到Confluence"、"在Confluence绘制图/写文档"时。
---

# Confluence 文档编写技能

## 能力概览

| 能力 | 说明 |
|------|------|
| **页面创建/更新** | 通过 REST API 创建新页面或更新已有页面（标题/正文/版本管理） |
| **绘制图表** | PlantUML / Mermaid / draw.io 三种方法（自动选型） |
| **附件管理** | 上传/删除/更新页面附件（图片、文件） |
| **页面查询** | 按空间/标题/ID 查询页面内容与元信息 |

后续将持续扩充（表格/模板/批量操作等）。

---

## 基础配置

```python
import json, ssl, urllib.request, urllib.error

TOKEN = "YOUR_PAT"          # 头像→设置→个人访问令牌
BASE = "https://confluence.example.com"
CTX = ssl._create_unverified_context()

def req(path, data=None, method="GET"):
    h = {"Authorization": "Bearer " + TOKEN}
    if data is not None: h["Content-Type"] = "application/json"
    d = json.dumps(data).encode() if data is not None else None
    r = urllib.request.Request(BASE+path, data=d, headers=h, method=method)
    try: x = urllib.request.urlopen(r, timeout=60, context=CTX); return x.status, x.read().decode()
    except urllib.error.HTTPError as e: return e.code, e.read().decode()
```

**前提**：Confluence 容器已装 Graphviz（`docker exec -u root 容器名 apt-get install -y graphviz`）

---

## 能力一：绘制图表

### 自动选型

| 用户场景 | 方法 | 原因 |
|---------|------|------|
| 时序图/交互图/用例图/类图/组件图 | **PlantUML** | UML 语法专业，inline 最简单 |
| 流程图/决策树/甘特图 | **Mermaid** | 语法简洁，中文完美 |
| 网络拓扑/架构图/需精确定位 | **draw.io** | 自由定位，可视化编辑器 |
| 含大量中文标签 | **Mermaid** | 浏览器端渲染，中文零乱码 |
| 用户需后续点击编辑 | **draw.io** | 内置可视化拖拽编辑 |
| 不确定/通用 | **Mermaid** | 适用性最广 |

### 方法1：PlantUML（宏名 `plantuml`，inline body）

最简单——代码直接写宏 body，Confluence 不丢弃。无需上传附件。

```python
def plantuml(code):
    cd_open = '<ac:plain-text-body>' + chr(60) + '![CDATA['
    cd_close = ']]' + chr(62) + '</ac:plain-text-body>'
    return ('<ac:structured-macro ac:name="plantuml" ac:schema-version="1">'
            + cd_open + '\n' + code + '\n' + cd_close + '</ac:structured-macro>')
```

语法：`@startuml` ... `@enduml`。中文乱码加 `skinparam defaultFontName "Noto Sans CJK SC"`。

### 方法2：Mermaid（宏名 `mermaid-cloud`，附件法）

中文完美（浏览器渲染）。Confluence 丢弃宏 body，须上传附件。

```bash
# 1. 上传Mermaid代码为附件（文件名=图表名，无扩展名！）
echo 'graph TD' > /tmp/图表名
echo 'A["开始"] --> B["结束"]' >> /tmp/图表名
curl -sk -H "X-Atlassian-Token: no-check" -H "Authorization: Bearer TOKEN" \
  -F "file=@/tmp/图表名;filename=图表名;type=text/plain" \
  "CONFLUENCE_URL/rest/api/content/PAGE_ID/child/attachment"
```
```python
# 2. 宏引用(filename=附件名) + PUT页面
def mm(name):
    return ('<ac:structured-macro ac:name="mermaid-cloud" ac:schema-version="1">'
            '<ac:parameter ac:name="toolbar">bottom</ac:parameter>'
            '<ac:parameter ac:name="filename">'+name+'</ac:parameter>'
            '<ac:parameter ac:name="format">svg</ac:parameter></ac:structured-macro>')
```

语法：`graph TD`(上下) / `graph LR`(左右) / `flowchart TD`(决策) / `sequenceDiagram` / `gantt`
着色：`style A fill:#bbdefb,stroke:#1565c0`

### 方法3：draw.io（宏名 `drawio`，附件法）

最强大——可视化编辑器，自由定位。需上传 mxfile XML + PNG 预览。

```bash
# 上传mxfile（无扩展名，type=application/vnd.jgraph.mxfile）+ PNG预览
curl -sk -H "Authorization: Bearer TOKEN" \
  -F "file=@/tmp/图名;filename=图名;type=application/vnd.jgraph.mxfile" \
  -F "file=@/tmp/图名.png;filename=图名.png;type=image/png" \
  "CONFLUENCE_URL/rest/api/content/PAGE_ID/child/attachment"
```
```python
def drawio(name):
    return ('<ac:structured-macro ac:name="drawio" ac:schema-version="1">'
            '<ac:parameter ac:name="diagramName">'+name+'</ac:parameter>'
            '<ac:parameter ac:name="revision">1</ac:parameter></ac:structured-macro>')
```

mxfile 格式：`<mxfile><diagram><mxGraphModel><root><mxCell.../>...</root></mxGraphModel></diagram></mxfile>`

---

## 能力二：页面创建/更新

### 更新已有页面
```python
# GET 当前版本
s, b = req(f"/rest/api/content/{PAGE_ID}?expand=version")
d = json.loads(b)
ver = d["version"]["number"]
title = d.get("title", "标题")

# PUT 更新
payload = {"id": PAGE_ID, "type": "page", "title": title,
           "version": {"number": ver + 1},
           "body": {"storage": {"value": html, "representation": "storage"}}}
s, b = req(f"/rest/api/content/{PAGE_ID}", payload, "PUT")
```

### 创建新页面
```python
payload = {"type": "page", "title": "标题",
           "space": {"key": "SPACE_KEY"},
           "body": {"storage": {"value": html, "representation": "storage"}}}
s, b = req("/rest/api/content", payload, "POST")
page_id = json.loads(b)["id"]
```

---

## 能力三：附件管理

### 上传附件
```bash
curl -sk -H "X-Atlassian-Token: no-check" -H "Authorization: Bearer TOKEN" \
  -F "file=@/path/to/file;filename=文件名;type=MIME类型" \
  "CONFLUENCE_URL/rest/api/content/PAGE_ID/child/attachment"
```

### 删除附件
```bash
curl -sk -X DELETE -H "X-Atlassian-Token: no-check" -H "Authorization: Bearer TOKEN" \
  "CONFLUENCE_URL/rest/api/content/PAGE_ID/child/attachment/ATTACHMENT_ID"
```

### 更新同名附件（先删再传）
Confluence 不允许直接覆盖同名附件，需先 DELETE 旧附件再 POST 新的。

---

## 常用 Confluence 宏

| 宏 | 用途 | 格式 |
|----|------|------|
| `toc` | 自动目录 | `<ac:structured-macro ac:name="toc"><ac:parameter ac:name="maxLevel">3</ac:parameter></ac:structured-macro>` |
| `info/warning/tip/note` | 信息面板 | `<ac:structured-macro ac:name="info"><ac:rich-text-body><p>...</p></ac:rich-text-body></ac:structured-macro>` |
| `code` | 代码块 | `<ac:structured-macro ac:name="code"><ac:parameter ac:name="language">python</ac:parameter><ac:plain-text-body><![CDATA[...]]></ac:plain-text-body></ac:structured-macro>` |
| `plantuml` | PlantUML 图 | 见方法1 |
| `mermaid-cloud` | Mermaid 图 | 见方法2 |
| `drawio` | draw.io 图 | 见方法3 |

**注意**：代码块内不能嵌套 `]]>`（CDATA 结束标记），需用 `] ] >` 替换。

---

## 常见问题

| 问题 | 解决 |
|------|------|
| PlantUML 中文乱码 | `skinparam defaultFontName "Noto Sans CJK SC"`，或换 Mermaid |
| Mermaid "diagram not found" | filename = 附件名（无扩展名，精确匹配含中文） |
| "Could not create Diagram" | 容器安装 graphviz |
| draw.io 404 | 附件名无扩展名 + type=application/vnd.jgraph.mxfile |
| PUT 400 "String ']]>' not allowed" | 代码块内有 `]]>`，替换为 `] ] >` |
| PUT 409 版本冲突 | 重新 GET 版本号再 PUT |
