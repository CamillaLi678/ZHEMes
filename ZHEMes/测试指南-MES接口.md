# MES接口测试指南

## 测试前准备

### 1. 确认文件存在
- [ ] `Build\APISLAK.txt` 文件存在
- [ ] 文件内容为单行API密钥（已确认存在）

### 2. 配置MES服务器地址
确保配置文件中的baseUrl正确设置，例如：
```
http://192.168.120.101:8080
```

### 3. 准备测试数据
需要准备以下测试数据：
- 工单号 (mo_no): 例如 "制令单号"
- 箱单条码 (box_sn): 通过USB扫码枪扫描获取，例如 "芯片箱单条码"
- 机台编号 (rs_no): 从界面IDC_EDITWORKOERDERICNUM获取
- 操作员 (wk_no): 登录用户账号

## 测试步骤

### 步骤1：界面输入
1. 启动程序并登录
2. 在界面上输入或扫描：
   - 工单号
   - 使用USB扫码枪扫描箱单条码
   - 确认机台编号已显示
   - 确认操作员账号已显示

### 步骤2：获取工单信息
1. 点击"获取工单"或相关按钮
2. 程序会调用 `GetACMesRecord()` 函数

### 步骤3：查看日志输出
在日志中应该能看到以下信息：

#### 成功的情况：
```
[LOG] 成功读取API密钥，路径: D:\...\Build\APISLAK.txt
[LOG] GetACMesRecord, strURL=http://192.168.120.101:8080/zqyb/ihtml?msclass=$APP&servclass=api.SYBACGETMO&weblis=api.Request, strBody={"mo_no":"xxx","box_sn":"xxx","rs_no":"xxx","wk_no":"xxx"}, strResponse=..., HttpPost Ret=0
[LOG] 烧录校验值: xxxxxxxx
[LOG] 工程文件路径: xxx.apr
[LOG] 自动化任务文件路径: xxx.tsk
```

#### 失败的情况：
```
[WARN] 无法读取APISLAK.txt（路径: ...），将不带API密钥请求
或
[ERR] HTTP请求失败，错误码: x
或
[ERR] MES返回错误: xxxxx
或
[ERR] 解析MES返回的ProjectChecksum字段错误...
```

## 验证要点

### 1. 请求发送验证
查看日志确认：
- [ ] URL格式正确（包含 `/zqyb/ihtml?msclass=$APP&servclass=api.SYBACGETMO&weblis=api.Request`）
- [ ] 请求体包含所有四个参数（mo_no, box_sn, rs_no, wk_no）
- [ ] 请求头包含API密钥（如果文件存在）

### 2. 响应解析验证
查看日志确认：
- [ ] 烧录校验值（ProjectChecksum）已正确解析并显示
- [ ] 工程文件路径（ProjectName）已正确解析并显示
- [ ] 自动化任务文件路径（TaskName）已正确解析并显示

### 3. 数据保存验证
- [ ] 如果MES返回批号（bat_no），应该在日志中看到："MES返回批号: xxx"
- [ ] 工单信息已保存到 `m_mesInfo` 变量中

## 常见问题排查

### 问题1：无法读取API密钥
**症状**: 日志显示 "无法读取APISLAK.txt"
**解决方法**:
1. 检查 `Build\APISLAK.txt` 文件是否存在
2. 检查文件权限是否正确
3. 确认程序运行目录是否正确

### 问题2：HTTP请求失败
**症状**: 日志显示 "HTTP请求失败，错误码: x"
**解决方法**:
1. 检查网络连接
2. 确认MES服务器地址配置正确
3. 使用Postman等工具测试URL是否可访问

### 问题3：解析字段错误
**症状**: 日志显示 "解析MES返回的xxx字段错误"
**解决方法**:
1. 查看完整的响应内容（strResponse）
2. 确认MES返回的JSON格式是否正确
3. 确认必需字段（ProjectChecksum, ProjectName, TaskName）是否存在

### 问题4：MES返回错误
**症状**: 日志显示 "MES返回错误: xxx"
**解决方法**:
1. 查看错误信息内容
2. 确认工单号是否正确
3. 确认箱单条码是否有效
4. 联系MES管理员检查服务端配置

## 测试用例示例

### 测试用例1：正常流程
**输入**:
- mo_no: "WO20251118001"
- box_sn: "BOX20251118001"
- rs_no: "STATION01"
- wk_no: "operator01"

**预期输出**:
- HTTP返回码: 0
- 成功解析三个必需字段
- 日志显示完整信息

### 测试用例2：缺少箱单条码
**输入**:
- mo_no: "WO20251118001"
- box_sn: ""（空）
- rs_no: "STATION01"
- wk_no: "operator01"

**预期输出**:
- 请求仍然发送（box_sn字段不在请求中）
- 取决于MES服务器是否允许该字段为空

### 测试用例3：API密钥不存在
**测试方法**: 临时重命名或删除 `APISLAK.txt`
**预期输出**:
- 日志显示警告信息
- 请求仍然发送（不带APISLAK请求头）
- 取决于MES服务器是否验证该密钥

## 调试技巧

1. **启用详细日志**: 确保日志级别设置为LOGLEVEL_LOG
2. **使用网络抓包工具**: 如Fiddler或Wireshark查看实际HTTP请求
3. **逐步测试**: 先测试简单场景，再测试复杂场景
4. **对比原有接口**: 与旧版MES接口对比，确认差异

## 性能测试

- 测试连续调用多次的情况
- 测试网络延迟较高的情况
- 测试并发请求的情况（如果适用）

## 测试完成检查清单

- [ ] 成功读取API密钥
- [ ] 正确发送四个参数
- [ ] 正确显示烧录校验值
- [ ] 正确显示工程文件路径
- [ ] 正确显示自动化任务文件路径
- [ ] 错误处理正常工作
- [ ] 日志输出完整清晰
- [ ] 与MES服务器通信正常

