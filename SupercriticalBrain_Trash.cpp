#include "SupercriticalBrain.h"


#define Tset_PRECISION 1.0 //, 


void SupercriticalBrain::RunRegulation_old()
{
	Time now_time = GetSysTime();
	// Проверяем подключение к серверам:
	if(!ConnectOPC_SRC()) {
		UpdateValue(0, now_time, "BAD");
		return;
	} else {
		UpdateValue(0, now_time, "GOOD");
	}
	if(!ConnectOPC_CTR()) {
		UpdateValue(3, now_time, "BAD");
		return;
	} else {
		UpdateValue(3, now_time, "GOOD");
	}
	String val;
	Time   val_time;
	double T, P;
	bool   val_quality;
	
	bool   is_hand_controlling;		// Включено ли ручное управление нагревом?
	int    current_power;			// Последнее установленное значение мощности
									// (при непрерывной работе = текущее значение мощности)
	// ----- Включено ли ручное управление -----
	val = opc_src.ReadTag(cfg.tagid_r_handcontrolling, val_time, val_quality);
	is_hand_controlling = val.IsEqual("true");
	UpdateValue(4, val_time, is_hand_controlling ? "да" : "нет");
	// ----- Текущее значение мощности ------
	val = opc_src.ReadTag(cfg.tagid_r_last_power, val_time, val_quality);
	current_power = ScanInt(val);
	UpdateValue(5, val_time, current_power);
	
	// Вспомогательные переменные для вычисления u(t)
	double e_now;
	double dt, de;
	double u;
	int    pow;
	
	// ----- Текущее значение контрольной температуры ------
	val = opc_src.ReadTag(cfg.tagid_s_temperature, val_time, val_quality);
	T = ScanDouble(val);
	UpdateValue(1, val_time, T);
	if (val_quality) {
		if (store_t.Add(val_time, T)) { // Значение обновилось - можно пересчитать мощность
			// Для первого значения - ничего не считаем, просто отмечаем его время
			if (store_t.GetCount() == 1) {
				pid_mem.start_time = val_time;
				pid_mem.e_last     = heat_Tset - T;
				pid_mem.t_last     = val_time;
				pid_mem.integral   = 0;
				pid_mem.max_u      = 0;
				pid_mem.Tset_timer = 0;
				// Выставляем мощность в 100%
				pow = 100;
				Log_AddMessage("Включаем максимальный нагрев");
			} else {
				if (pid_mem.max_u <= 0) {
					Log_AddMessage("Включаем механизм ПИД-регулирования нагрева");
				}
				// Считаем e(t) - текущую невязку
				e_now = heat_Tset - T;
				// dt
				dt = (double)(val_time.Get() - pid_mem.t_last.Get());
				// Интеграл по невязке
				//pid_mem.integral = pid_mem.integral;// + e_now * dt;
				pid_mem.integral = pid_mem.integral + e_now * dt;
				// Скорость для невязки
				de = (e_now - pid_mem.e_last) / dt;
				
				// Обновляем временные хранилища прошлых значений
				pid_mem.e_last = e_now;
				pid_mem.t_last = val_time;
				
				// Считаем u(t):
				u = cfg.pid_Kp * e_now 
				  + cfg.pid_Ki * pid_mem.integral 
				  + cfg.pid_Kd * de;
				
				// Ищем максимальное u(t)
				
				//if (u > pid_mem.max_u) pid_mem.max_u = u;
				if (now_time.Get() - pid_mem.start_time.Get() <= 60) {
					pid_mem.max_u = u;
				}
				// Ищем мощность в процентах
				pow = (int)((u * 100.0/ pid_mem.max_u) + 0.5);
				if (pow <   0) pow = 0;
				if (pow > 100) pow = 100;
				
				//Log_AddService("<" + FormatDouble(e_now) + "> <" + FormatDouble(pid_mem.integral) + "> <" + FormatDouble(de) + "> = <" + FormatDouble(u) + "> <" + FormatDouble(pid_mem.max_u) + "> <" + FormatInt(pow) + ">");
				
			}

			
			
			if (e_now <= Tset_PRECISION) { //  && T <= heat_Tset + Tset_PRECISION) {
				if (pid_mem.Tset_timer == 0) {
					Log_AddMessage("Заданная температура достигнута!");
					pid_mem.Tset_timer = now_time.Get();
				}
			}
			if (pid_mem.Tset_timer > 0) {
				// Проверяем, не прошло ли заданное время поддержания температуры Tset
				if (now_time.Get() - pid_mem.Tset_timer >= heat_duration * 60) {
					Log_AddGood("Время поддержания заданной температуры истекло. Остановка нагрева!");
					// Вырубаем нагрев:
					pow = 0;
					StopHeating();
				}
			}
			// Обновляем значение на экране:
			UpdateValue(7, now_time, pow);
			
			// Пишем на управляющий сервер, если ручное управление отключено
			if (!is_hand_controlling) {
				for (int i = 0; i < 5; ++i) {
					if (opc_ctr.WriteTag(cfg.tagid_r_power, FormatInt(pow))) break;
				}
			}
			// -------------------------
		}
	}
	
	// ----- Текущее значение контрольного давления ------
	val = opc_src.ReadTag(cfg.tagid_s_pressure, val_time, val_quality);
	P = ScanDouble(val);
	UpdateValue(2, val_time, P);
	if (val_quality) {
		if (store_p.Add(val_time, P)) {
			// Ура! Новое значение - считаем по нему все, что надо
		
			// -------------------------
		}
	}
	
	
}
