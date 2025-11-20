#include "stdafx.h"
#include "MesWorkflow.h"
#include "ComTool.h"
#include <stdarg.h>

CMesWorkflow::CMesWorkflow()
	: m_pILog(NULL)
	, m_bInited(FALSE)
	, m_bDataReady(FALSE)
	, m_pMesInterface(NULL)
{
	memset(&m_mesInfo, 0, sizeof(m_mesInfo));
}

CMesWorkflow::~CMesWorkflow()
{
	DeInit();
}

void CMesWorkflow::Init(ILog *pLog)
{
	if (m_bInited) {
		return;
	}

	m_pILog = pLog;
	m_pMesInterface = &CMesInterface::getInstance();
	m_bInited = TRUE;
	m_bDataReady = FALSE;

	LogInfo("MES工作流程管理器初始化完成");
}

void CMesWorkflow::DeInit()
{
	if (m_bInited) {
		m_bInited = FALSE;
		m_bDataReady = FALSE;
		m_pILog = NULL;
		m_pMesInterface = NULL;
	}
}

void CMesWorkflow::SetWorkOrderParams(const WorkOrderParams& params)
{
	m_workOrderParams = params;
	m_bDataReady = FALSE;  // 参数改变，数据需要重新获取

	LogInfo("设置工单参数: 工单号=%s, 料号=%s, 箱单=%s, 机台=%s, 操作员=%s",
		params.workOrder, params.materialID, params.boxSN, 
		params.stationId, params.operatorID);
}

INT CMesWorkflow::GetWorkOrderInfo(MesInfo& outMesInfo)
{
	if (!m_bInited) {
		SetLastError("MES工作流程管理器未初始化");
		return -1;
	}

	// 验证必填参数
	if (m_workOrderParams.workOrder.IsEmpty()) {
		SetLastError("工单号不能为空");
		return -2;
	}

	if (m_workOrderParams.materialID.IsEmpty()) {
		SetLastError("芯片料号不能为空");
		return -3;
	}

	if (m_workOrderParams.boxSN.IsEmpty()) {
		SetLastError("箱单条码不能为空（请使用USB扫码枪扫描）");
		return -4;
	}

	if (m_workOrderParams.stationId.IsEmpty()) {
		SetLastError("机台编号不能为空");
		return -5;
	}

	if (m_workOrderParams.operatorID.IsEmpty()) {
		SetLastError("操作员账号不能为空（请先登录）");
		return -6;
	}

	LogInfo("开始获取工单信息...");
	LogInfo("  工单号: %s", m_workOrderParams.workOrder);
	LogInfo("  料号: %s", m_workOrderParams.materialID);
	LogInfo("  箱单: %s", m_workOrderParams.boxSN);
	LogInfo("  机台: %s", m_workOrderParams.stationId);
	LogInfo("  操作员: %s", m_workOrderParams.operatorID);

	// 调用MES接口获取工单信息
	// 注意：这里使用AC MES接口，它会自动添加 box_sn, bat_no, rs_no, wk_no 参数
	m_mesInfo = m_pMesInterface->GetACMesRecord(
		m_workOrderParams.workOrder,
		m_workOrderParams.materialID,
		"Program"  // 执行功能模式：Program 或 Verify
	);

	if (m_mesInfo.errCode != 0) {
		SetLastError("获取工单信息失败");
		LogError("GetWorkOrderInfo失败, errCode=%d", m_mesInfo.errCode);
		return -10;
	}

	// 检查返回数据的完整性
	if (m_mesInfo.projPath.IsEmpty()) {
		SetLastError("MES未返回工程文件路径");
		return -11;
	}

	if (m_mesInfo.projChecksum.IsEmpty()) {
		SetLastError("MES未返回工程文件校验值");
		return -12;
	}

	// 设置数据就绪标志
	m_bDataReady = TRUE;

	// 输出到外部参数
	outMesInfo = m_mesInfo;

	LogInfo("获取工单信息成功!");
	LogInfo("  工程文件: %s", m_mesInfo.projPath);
	LogInfo("  校验值: %s", m_mesInfo.projChecksum);
	LogInfo("  任务文件: %s", m_mesInfo.autoTaskFilePath);
	LogInfo("  期望数量: %d", m_mesInfo.expectICNum);
	LogInfo("  版本号: %s", m_mesInfo.projVersion);

	return 0;
}

BOOL CMesWorkflow::ValidateProjectChecksum(const CString& realChecksum, const CString& expectedChecksum)
{
	LogInfo("开始校验工程文件checksum...");
	LogInfo("  实际校验值: %s", realChecksum);
	LogInfo("  期望校验值: %s", expectedChecksum);

	// 移除可能的0x前缀，统一为大写进行比较
	CString real = realChecksum;
	CString expected = expectedChecksum;

	real.Replace("0x", "");
	real.Replace("0X", "");
	expected.Replace("0x", "");
	expected.Replace("0X", "");

	real.MakeUpper();
	expected.MakeUpper();

	if (real == expected) {
		LogInfo("校验值匹配成功!");
		return TRUE;
	}
	else {
		LogError("校验值不匹配! 实际=%s, 期望=%s", real, expected);
		SetLastError("工程文件校验值不匹配，请确认工程文件是否正确");
		return FALSE;
	}
}

INT CMesWorkflow::UploadSlotInfo(const CString& slotInfoJson)
{
	if (!m_bInited) {
		SetLastError("MES工作流程管理器未初始化");
		return -1;
	}

	if (!m_bDataReady) {
		SetLastError("请先获取工单信息");
		return -2;
	}

	if (slotInfoJson.IsEmpty()) {
		SetLastError("座子信息不能为空");
		return -3;
	}

	LogInfo("开始上传座子信息...");
	LogInfo("座子信息JSON: %s", slotInfoJson);

	// 调用MES接口上传座子信息
	INT ret = m_pMesInterface->CommitProgramerInfo2Mes(slotInfoJson);

	if (ret != 0) {
		SetLastError("上传座子信息失败");
		LogError("UploadSlotInfo失败, ret=%d", ret);
		return -10;
	}

	LogInfo("上传座子信息成功!");
	return 0;
}

INT CMesWorkflow::UploadProgramResult(const CString& resultJson)
{
	if (!m_bInited) {
		SetLastError("MES工作流程管理器未初始化");
		return -1;
	}

	if (!m_bDataReady) {
		SetLastError("请先获取工单信息");
		return -2;
	}

	if (resultJson.IsEmpty()) {
		SetLastError("烧录结果不能为空");
		return -3;
	}

	LogInfo("开始上传烧录结果...");

	// 调用MES接口上传烧录结果
	// 注意：使用AC MES接口，它会自动添加额外的字段
	INT ret = m_pMesInterface->CommitProgramRet2ACMes(resultJson);

	if (ret != 0) {
		SetLastError("上传烧录结果失败");
		LogError("UploadProgramResult失败, ret=%d", ret);
		return -10;
	}

	LogInfo("上传烧录结果成功!");
	return 0;
}

INT CMesWorkflow::UploadTaskInfo(const CString& timeStart, const CString& timeEnd,
	const CString& timeRun, const CString& timeStop)
{
	if (!m_bInited) {
		SetLastError("MES工作流程管理器未初始化");
		return -1;
	}

	if (!m_bDataReady) {
		SetLastError("请先获取工单信息");
		return -2;
	}

	LogInfo("开始上传任务信息...");
	LogInfo("  开始时间: %s", timeStart);
	LogInfo("  结束时间: %s", timeEnd);
	LogInfo("  运行时间: %s", timeRun);
	LogInfo("  停止时间: %s", timeStop);

	// 调用MES接口上传任务信息
	INT ret = m_pMesInterface->CommitTaskInfo2Mes(timeStart, timeEnd, timeRun, timeStop);

	if (ret != 0) {
		SetLastError("上传任务信息失败");
		LogError("UploadTaskInfo失败, ret=%d", ret);
		return -10;
	}

	LogInfo("上传任务信息成功!");
	return 0;
}

void CMesWorkflow::LogInfo(const char* fmt, ...)
{
	if (m_pILog) {
		char buffer[2048];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
		va_end(args);
		m_pILog->PrintLog(LOGLEVEL_LOG, buffer);
	}
}

void CMesWorkflow::LogError(const char* fmt, ...)
{
	if (m_pILog) {
		char buffer[2048];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
		va_end(args);
		m_pILog->PrintLog(LOGLEVEL_ERR, buffer);
	}
}

void CMesWorkflow::SetLastError(const CString& error)
{
	m_strLastError = error;
	LogError("%s", error);
}

