#include "SupercriticalBrain.h"

#define DEFAULT_WORK_FREQ 5


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

bool Configurate_SupercriticalBrain(const Value& j_conf, SupercriticalBrainCfg& cfg)
{
	return false;
}

bool Configurate_SupercriticalBrain(const char* cfg_path, SupercriticalBrainCfg& cfg)
{
	return false;
}

