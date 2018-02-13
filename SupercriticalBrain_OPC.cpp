#include "SupercriticalBrain.h"

static bool ConnectOPCServer(SimpleClientOPC& opc, const String& opc_name) 
{
	if (!opc.ServerStatus_IsRunning()) {
		opc.Disconnect();
		if (!opc.Connect(opc_name, "127.0.0.1", OPC_DFT_SEP)) {
			opc.Disconnect();
			return false;
		}
	}
	return true;	
}
static bool CheckOPCServer(SimpleClientOPC& opc, int& t1, int& t2) 
{
	return (opc.ServerStatus_IsRunning() && t1 >= 0 && t2 >= 0);
}

static int FindOPCTag(SimpleClientOPC& opc, const String& tag_name) 
{
	for (int i = 0; i < opc.Items_GetCount();++i) {
		if (tag_name.IsEqual(opc.Items_GetName(i))) {
			return i;
		}
	}
	return -1;
}

bool SupercriticalBrain::ConnectOPC_SRC()
{
	if (CheckOPCServer(opc_src, cfg.tagid_s_pressure, cfg.tagid_s_temperature)) return true;
	opc_src.Disconnect();
	cfg.tagid_s_temperature = -1;
	cfg.tagid_s_pressure    = -1;
	if (!ConnectOPCServer(opc_src, cfg.opc_source_name))
		return false;
	cfg.tagid_s_temperature = FindOPCTag(opc_src, cfg.opc_s_tag_temperature);
	cfg.tagid_s_pressure    = FindOPCTag(opc_src, cfg.opc_s_tag_pressure);			
		
	return CheckOPCServer(opc_src, cfg.tagid_s_pressure, cfg.tagid_s_temperature);
}

bool SupercriticalBrain::ConnectOPC_CTR()
{
	if (CheckOPCServer(opc_ctr, cfg.tagid_r_handcontrolling, cfg.tagid_r_power)
	&& cfg.tagid_r_last_power >= 0) return true;
	opc_ctr.Disconnect();
	cfg.tagid_r_power           = -1;
	cfg.tagid_r_handcontrolling = -1;
	cfg.tagid_r_last_power      = -1;
	if (!ConnectOPCServer(opc_ctr, cfg.opc_result_name))
		return false;
	cfg.tagid_r_power           = FindOPCTag(opc_ctr, cfg.opc_r_tag_power);
	cfg.tagid_r_last_power      = FindOPCTag(opc_ctr, cfg.opc_r_tag_last_power);
	cfg.tagid_r_handcontrolling = FindOPCTag(opc_ctr, cfg.opc_r_tag_handcontrolling);			
	
		
	return (CheckOPCServer(opc_ctr, cfg.tagid_r_handcontrolling, cfg.tagid_r_power) && cfg.tagid_r_last_power >= 0);
}

void SupercriticalBrain::InitServers()
{
	if (ConnectOPC_SRC()) 
		Log_AddGood("Подключение к серверу с исходными данными установлено");
	else 
		Log_AddError("Нет подключения к серверу исходных данных или отсутствуют заданные теги");
	if (ConnectOPC_CTR()) 
		Log_AddGood("Подключение к серверу управления нагревом установлено");
	else
		Log_AddError("Нет подключения к серверу управления нагревом или отсутствуют заданные теги");
}