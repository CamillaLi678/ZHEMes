# MES接口修改说明

## 修改日期
2025-11-18

## 修改文件
`ZHEMes/ProgCtrl/MesInterface.cpp` - `GetACMesRecord()` 函数

## 修改内容

### 1. 读取API密钥
从 `APISLAK.txt` 文件读取API密钥，并添加到请求头中：
```cpp
// 读取APISLAK.txt获取API密钥
CString strApiKey = _T("");
CString strApiKeyPath;
strApiKeyPath.Format("%s\\APISLAK.txt", m_strProgFileFolder);

CStdioFile apiFile;
if (apiFile.Open(strApiKeyPath, CFile::modeRead | CFile::typeText)) {
    apiFile.ReadString(strApiKey);
    apiFile.Close();
    strApiKey.Trim();
}
```

### 2. 更新请求URL
使用新的URL格式（根据API文档）：
```
原URL: {baseUrl}/mes/api/v1/getworkorder
新URL: {baseUrl}/zqyb/ihtml?msclass=$APP&servclass=api.SYBACGETMO&weblis=api.Request
```

### 3. 请求头添加API密钥
```cpp
// 添加API密钥到请求头
if (!strApiKey.IsEmpty()) {
    strHeader.Format("Content-Type:application/json;charset:UTF-8\r\nAPISLAK: %s", strApiKey);
} else {
    strHeader.Format("Content-Type:application/json;charset:UTF-8");
}
```

### 4. 请求参数
发送四个参数到MES服务器：
- `mo_no`: 制令单号（工单号）
- `box_sn`: 芯片箱单条码（USB扫码枪输入）
- `rs_no`: 机台编号
- `wk_no`: 作业人员（操作员账号）

```json
{
  "mo_no": "工单号",
  "box_sn": "箱单条码",
  "rs_no": "机台编号",
  "wk_no": "操作员"
}
```

### 5. 响应解析优化
改进了返回数据的解析逻辑，提取以下关键信息并在日志中显示：

#### 必需字段：
- **ProjectChecksum**: 烧录校验值
- **ProjectName**: 工程文件路径
- **TaskName**: 自动化任务文件路径

#### 可选字段：
- `mo_no`: 工单号
- `MaterialID`: 物料编号
- `bat_no`: 批号（如果MES返回）
- `Quantity`: 工单数量
- `RemainQuantity`: 剩余数量
- `SoftVersion`: 软件版本
- `Id`: 工单ID

### 6. 日志输出增强
添加了详细的日志输出，显示关键信息：
```cpp
m_pILog->PrintLog(LOGLEVEL_LOG, "烧录校验值: %s", info.projChecksum);
m_pILog->PrintLog(LOGLEVEL_LOG, "工程文件路径: %s", info.projPath);
m_pILog->PrintLog(LOGLEVEL_LOG, "自动化任务文件路径: %s", info.autoTaskFilePath);
```

## 使用方法

### 界面调用流程（MesDlg.cpp）：

在 `OnBnClickedBtnGetmesrecord()` 函数中已实现完整的调用流程：

1. **参数检查**：
   - 检查工单号（`m_strWorkOrder`）是否为空
   - 检查料号（`m_strMaterialID`）是否为空
   - 检查箱单条码（`m_strBoxSN`）是否为空
   - 检查机台编号（`m_strStationId`），如果为空则使用默认值

2. **参数设置**：
```cpp
// 批号自动使用工单号
m_strBatNo = m_strWorkOrder;

// 设置MES额外参数
CMesInterface &MesInterface = CMesInterface::getInstance();
MesInterface.SetExtraParams(
    m_strBoxSN,              // 箱单条码（USB扫码枪）
    m_strBatNo,              // 批号（使用工单号）
    m_strStationId,          // 机台编号
    m_Setting.strOperator    // 操作员账号
);
```

3. **调用接口**：
```cpp
// 在 GetMesRecord() 中会根据配置调用相应的函数
Ret = GetMesRecord();
// 如果 m_Setting.nServerTest 为 true，则调用：
// mesResult = MesInterface.GetACMesRecord(m_strWorkOrder, m_strMaterialID, m_Setting.strCurExec);
```

4. **结果处理**：
```cpp
if (mesResult.errCode == 0) {
    // 成功获取工单信息
    // mesResult.projChecksum - 校验值
    // mesResult.projPath - 工程文件路径
    // mesResult.autoTaskFilePath - 自动化任务文件路径
    UpdateCtrlsValueFromRecord();  // 更新界面显示
} else {
    // 获取失败，查看日志了解详细错误
}
```

### 日志输出示例：
```
[LOG] MES请求参数:
[LOG]   工单号(mo_no): WO20251118001
[LOG]   箱单条码(box_sn): BOX20251118001
[LOG]   批号(bat_no): WO20251118001
[LOG]   机台编号(rs_no): STATION_001
[LOG]   操作员(wk_no): Admin
[LOG] 成功读取API密钥，路径: D:\...\Build\APISLAK.txt
[LOG] GetACMesRecord, strURL=..., strBody={...}, strResponse={...}
[LOG] 烧录校验值: 0x12345678
[LOG] 工程文件路径: project.apr
[LOG] 自动化任务文件路径: task.tsk
```

## 配置文件位置
- **APISLAK.txt**: 放置在程序运行目录（与exe文件同级目录，即Build文件夹）
  - 完整路径示例: `D:\浙玮\20251117\ZHEMes\Build\APISLAK.txt`
  - 内容格式：单行API密钥字符串
  - 当前密钥已存在于该位置

## 注意事项
1. 确保 `APISLAK.txt` 文件存在且格式正确
2. URL中的baseUrl需要在程序初始化时正确配置
3. 所有四个参数（box_sn, rs_no, wk_no）都应该在调用前设置好
4. 检查日志输出确认三个关键信息是否正确显示：
   - 烧录校验值
   - 工程文件路径
   - 自动化任务文件路径

## 错误处理
- HTTP请求失败：记录错误码
- JSON解析失败：记录原始响应
- 必需字段缺失：记录详细错误信息
- API密钥文件不存在：发出警告但继续请求（不带密钥）

