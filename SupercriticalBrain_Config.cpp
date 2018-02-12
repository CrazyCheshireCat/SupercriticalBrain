#include "SupercriticalBrain.h"

#include <HeapOfNeeds/Heap_JsonConfig.h>

#define DEFAULT_WORK_FREQ 5

namespace SupercriticalBrainConfigNames
{
	const char* n_MainSettings       = "MainSettings";
	const char* n_DataSource         = "DataSource";
	const char* n_DS_ReadFreq        = "ReadDataFrequency_sec";
	const char* n_OpcName            = "OPCServerName";
	const char* n_DS_Tag_Temperature = "TagName_Temperature";
	const char* n_DS_Tag_Pressure    = "TagName_Pressure";
	const char* n_DataControl        = "DataControl";
	const char* n_DC_PowerControl    = "TagName_Power";
	const char* n_DC_ByHandControl   = "TagName_ByHandControl";
	
	const char* n_PID                = "PidSettings";
	
	const char* n_PID_Kp             = "Kp";
	const char* n_PID_Ki             = "Ki";
	const char* n_PID_Kd             = "Kd";
	
};

/*
{
	"MainSettings" : {
	}
	"DataSource" : {
		"ReadDataFrequency_sec" : 5,
		"OPCServerName"         : "BinarControllingServer",
		"TagName_Temperature"   : "UIM_001/ch04",
		"TagName_Pressure"      : "UIM_001/ch00"
	},
	"DataControl" :
	{
		"OPCServerName"         : "BinarControllingServer",
		"TagName_Power"         : "Regulator/power02",
		"TagName_ByHandControl" : "Regulator/regultator_controlling_byHand"
	},
	"PidSettings" : {
		"Kp" : 10,
		"Ki" : 0.01,
		"Kd" : 10
	}
}
*/

bool SupercriticalBrain::LoadConfig()
{
	if (!Configurate_SupercriticalBrain(cfg)) {
		return false;
	}
	
	v_Kp = cfg.pid_Kp;
	v_Ki = cfg.pid_Ki;
	v_Kd = cfg.pid_Kd;
	return true;
}

void SupercriticalBrain::PrintConfig()
{
	Log_AddGood("Конфигурационный файл загружен успешно");
	Log_AddService("");
	Log_AddService("-------------------------------------------");
	Log_AddService("");
	Log_AddService("----- Общие -----");
	Log_AddService(" Частота считывания данных: " + FormatInt(cfg.work_freq) + " сек");
	//Log_AddService("-------------------------------------------");
	Log_AddService("");
	Log_AddService("----- Сервер данных -----");
	Log_AddService(" Имя сервера данных: "      + cfg.opc_source_name);
	Log_AddService(" Контрольная температура: " + cfg.opc_s_tag_temperature);
	Log_AddService(" Контрольное давление: "    + cfg.opc_s_tag_pressure);
	//Log_AddService("-------------------------------------------");
	Log_AddService("");
	Log_AddService("----- Сервер управления -----");
	Log_AddService(" Имя сервера управления: "           + cfg.opc_result_name);
	Log_AddService(" Канал управления мощностью: "       + cfg.opc_r_tag_power);
	Log_AddService(" Ручное/автоматическое управление: " + cfg.opc_r_tag_handcontrolling);
	//Log_AddService("-------------------------------------------");
	Log_AddService("");
	Log_AddService("----- Коэффициенты ПИД-регулирования -----");
	Log_AddService(" Пропорциональный: Kp = "     + FormatDouble(cfg.pid_Kp));
	Log_AddService(" Интегральный: Ki = "         + FormatDouble(cfg.pid_Ki));
	Log_AddService(" Дифференциальный: Kd =  "    + FormatDouble(cfg.pid_Kd));
	Log_AddService("");
	Log_AddService("-------------------------------------------");
	Log_AddService("");
}


bool Configurate_SupercriticalBrain(SupercriticalBrainCfg& cfg)
{
	cfg.work_freq                  = 5; // сек
	cfg.opc_source_name            = "BinarControllingServer";
	cfg.opc_s_tag_temperature      = "UIM_001/ch04";
	cfg.opc_s_tag_pressure         = "UIM_001/ch00";
	cfg.opc_result_name            = "BinarControllingServer";
	cfg.opc_r_tag_handcontrolling  = "Regulator/regultator_controlling_byHand";
	cfg.opc_r_tag_power            = "Regulator/power02";
	
	cfg.pid_Kp = 10;
	cfg.pid_Ki = 0.01;
	cfg.pid_Kd = 10;
	cfg.pid_start_pow = 100;
	
	return true;
}
/*
bool Configurate_SupercriticalBrain(const Value& j_conf, SupercriticalBrainCfg& cfg)
{
	if (j_conf.IsNull()) return false;
	
	if ()
	
	return false;
}
*/

bool Configurate_SupercriticalBrain(const char* path, SupercriticalBrainCfg& cfg)
{
	if (!FileExists(path)) return false;
	try {
		String  s = LoadFile(path);
		CParser p(s);
		//Value   j_conf = ParseJSON(p);
		cfg.j_conf = ParseJSON(p);
		using namespace SupercriticalBrainConfigNames;
		j_int    freq; freq.SetName(n_DS_ReadFreq);
		j_string str; 
		j_double K;
		
		// ----- Read Data frequency -----
		if (!freq.SetValueJson(cfg.j_conf[n_DataSource])) return false; 
		if (freq.GetValue() <= 0)                         return false;
		cfg.work_freq = freq.GetValue();
		
		// ----- Opc name -----
		str.SetName(n_OpcName);
		if (!str.SetValueJson(cfg.j_conf[n_DataSource])) return false;
		if (str.GetValue().IsEmpty())                    return false;
		cfg.opc_source_name = str.GetValue();
		// -----
		str.Clear();
		str.SetName(n_DS_Tag_Temperature);
		if (!str.SetValueJson(cfg.j_conf[n_DataSource])) return false;
		if (str.GetValue().IsEmpty())                    return false;
		cfg.opc_s_tag_temperature = str.GetValue();
		// -----
		str.Clear();
		str.SetName(n_DS_Tag_Pressure);
		if (!str.SetValueJson(cfg.j_conf[n_DataSource])) return false;
		if (str.GetValue().IsEmpty())                    return false;
		cfg.opc_s_tag_pressure = str.GetValue();
		// ----- Opc name -----
		str.SetName(n_OpcName);
		if (!str.SetValueJson(cfg.j_conf[n_DataControl])) return false;
		if (str.GetValue().IsEmpty())                    return false;
		cfg.opc_result_name = str.GetValue();
		// -----
		str.Clear();
		str.SetName(n_DC_PowerControl);
		if (!str.SetValueJson(cfg.j_conf[n_DataControl])) return false;
		if (str.GetValue().IsEmpty())                     return false;
		cfg.opc_r_tag_power = str.GetValue();
		// -----
		str.Clear();
		str.SetName(n_DC_ByHandControl);
		if (!str.SetValueJson(cfg.j_conf[n_DataControl])) return false;
		if (str.GetValue().IsEmpty())                     return false;
		cfg.opc_r_tag_handcontrolling = str.GetValue();
		
		// ----- PID Kp-----
		K.Clear();
		K.SetName(n_PID_Kp);
		if(!K.SetValueJson(cfg.j_conf[n_PID])) return false;
		//if(K.GetValue() )
		cfg.pid_Kp = K.GetValue();
		// ----- PID Ki-----
		K.Clear();
		K.SetName(n_PID_Ki);
		if(!K.SetValueJson(cfg.j_conf[n_PID]))    return false;
		if(K.GetValue() >= 1 || K.GetValue() < 0) return false;
		cfg.pid_Ki = K.GetValue();
		
		K.Clear();
		K.SetName(n_PID_Kd);
		if(!K.SetValueJson(cfg.j_conf[n_PID])) return false;
		//if(K.GetValue() )
		cfg.pid_Kd = K.GetValue();
		
		
	} catch (CParser::Error) {
		cfg.j_conf = NULL;
		return false;
	} catch (...) {
		cfg.j_conf = NULL;
		return false;
	}
	return true;
}

void UpdateConfig_SupercriticalBrain(const char* cfg_path, SupercriticalBrainCfg& cfg)
{
}
