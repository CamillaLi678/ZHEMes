#pragma once

#include "ILog.h"
#include "MesCommon.h"
#include "MesInterface.h"
#include <functional>

// MES业务流程管理类
// 负责协调整个MES工作流程，包括：
// 1. 获取工单信息
// 2. 校验工程文件
// 3. 上传座子信息
// 4. 管理烧录过程
// 5. 上传结果和任务信息

class CMesWorkflow
{
public:
	CMesWorkflow();
	~CMesWorkflow();

	// 初始化
	void Init(ILog *pLog);
	void DeInit();

	// 设置工单参数
	struct WorkOrderParams {
		CString workOrder;      // 工单号 (mo_no)
		CString materialID;     // 芯片料号 (MaterialID)
		CString boxSN;          // 箱单条码 (box_sn) - USB扫码枪输入
		CString stationId;      // 机台编号 (rs_no)
		CString operatorID;     // 操作员账号 (wk_no) - 登录用户名
	};
	void SetWorkOrderParams(const WorkOrderParams& params);

	// 步骤1: 获取工单信息
	// 调用 getworkorder API
	// 返回: 0成功, <0失败
	INT GetWorkOrderInfo(MesInfo& outMesInfo);

	// 步骤2: 校验工程文件checksum
	// 参数: realChecksum - 实际工程文件的校验值
	//       expectedChecksum - MES返回的期望校验值
	// 返回: TRUE匹配, FALSE不匹配
	BOOL ValidateProjectChecksum(const CString& realChecksum, const CString& expectedChecksum);

	// 步骤3: 上传座子信息
	// 调用 sendproginfo API
	// 参数: slotInfoJson - 座子信息JSON字符串
	// 返回: 0成功, <0失败
	INT UploadSlotInfo(const CString& slotInfoJson);

	// 步骤4: 上传烧录结果
	// 调用 sendprogresult API
	// 参数: resultJson - 烧录结果JSON字符串（包含Pass/Fail统计和不良原因）
	// 返回: 0成功, <0失败
	INT UploadProgramResult(const CString& resultJson);

	// 步骤5: 上传任务信息
	// 调用 sendtaskinfo API
	// 参数: timeStart - 任务开始时间
	//       timeEnd - 任务结束时间
	//       timeRun - 运行时间
	//       timeStop - 停止时间
	// 返回: 0成功, <0失败
	INT UploadTaskInfo(const CString& timeStart, const CString& timeEnd, 
		               const CString& timeRun, const CString& timeStop);

	// 获取最后的错误信息
	CString GetLastError() const { return m_strLastError; }

	// 获取当前MES数据
	const MesInfo& GetMesInfo() const { return m_mesInfo; }

	// 数据是否就绪
	BOOL IsDataReady() const { return m_bDataReady; }

private:
	ILog* m_pILog;
	BOOL m_bInited;
	BOOL m_bDataReady;           // MES数据是否已获取并就绪

	WorkOrderParams m_workOrderParams;  // 工单参数
	MesInfo m_mesInfo;                  // MES返回的工单信息
	CString m_strLastError;             // 最后的错误信息

	CMesInterface* m_pMesInterface;     // MES接口实例

	// 内部辅助函数
	void LogInfo(const char* fmt, ...);
	void LogError(const char* fmt, ...);
	void SetLastError(const CString& error);
};

