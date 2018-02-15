#include "SupercriticalBrain.h"

#define Tset_PRECISION 1.0 //, 

bool SupercriticalBrain::CheckParams()
{
	if (v_Tset.GetData().IsNull()) {
		PromptOK("Не задана температура для нагрева");
		v_Tset.SetFocus();
		return false;
	}
	
	if (v_time.GetData().IsNull()) {
		PromptOK("Не задана длительность поддержания температуры");
		v_time.SetFocus();
		return false;
	}
	
	if (v_Tset < 20 || v_Tset > 500) {
		PromptOK("Недопустимое значение температуры для нагрева");
		v_Tset.SetFocus();
		return false;
	}

	if (v_time <= 0 || v_time > 24*60) {
		PromptOK("Недопустимое время поддержания заданной температуры");
		v_time.SetFocus();
		return false;
	}
	
	if (v_Kp < 0) {
		PromptOK("Недопустимое значение коэффициента Kp");
		v_Kp.SetFocus();
		return false;
	}
	
	if (v_Ki < 0 || v_Ki >= 1) {
		PromptOK("Недопустимое значение коэффициента Ki");
		v_Ki.SetFocus();
		return false;
	}
	
	if (v_Kd < 0) {
		PromptOK("Недопустимое значение коэффициента Kd");
		v_Kd.SetFocus();
		return false;
	}
		
	cfg.pid_Kp = v_Kp;
	cfg.pid_Ki = v_Ki;
	cfg.pid_Kd = v_Kd;
	
	heat_Tset     = v_Tset;
	heat_duration = v_time;
	
	return true;
}

void SupercriticalBrain::Push_StartHeating()
{
	if (!CheckParams()) {
		Log_AddError("Неверные значения для начала работы");
		return;
	}
	v_time.Disable();
	v_Tset.Disable();
	v_Kp.Disable();
	v_Ki.Disable();
	v_Kd.Disable();
	btn_start.Disable();
	btn_stop.Enable();
	Log_AddGood("Начат нагрев до " + FormatDouble(heat_Tset) + " °С");
	Log_AddMessage("Установленное время поддержания температуры " + FormatInt64(heat_duration) + " мин");
	
	HeatingCallback();
	SetTimeCallback(-(cfg.work_freq * 1000), callback(this, &SupercriticalBrain::HeatingCallback), CLBK_ID_HEATING);
	
}

void SupercriticalBrain::StopHeating()
{
	v_time.Enable();
	v_Tset.Enable();
	v_Kp.Enable();
	v_Ki.Enable();
	v_Kd.Enable();
	btn_start.Enable();
	btn_stop.Disable();
	
	// Очищаем массивы с накопленными значениями:
	store_t.Clear();
	store_p.Clear();
	
	KillTimeCallback(CLBK_ID_HEATING);

}

void SupercriticalBrain::Push_StopHeating()
{
	StopHeating();
	
	Log_AddWarning("Нагрев останавливается вручную!");
	SetTimeCallback(5000, callback(this, &SupercriticalBrain::SetNullPower));
}

void SupercriticalBrain::SetNullPower()
{
	String val;
	Time   val_time;
	bool   val_quality;
	
	bool   is_hand_controlling;		// Включено ли ручное управление нагревом?
	val = opc_src.ReadTag(cfg.tagid_r_handcontrolling, val_time, val_quality);
	is_hand_controlling = val.IsEqual("true");
	
	
	// Пишем на управляющий сервер, если ручное управление отключено
	if (!is_hand_controlling) {
		for (int i = 0; i < 50; ++i) {
			if (opc_ctr.WriteTag(cfg.tagid_r_power, "0")) {
				Log_AddGood("Команда выключения нагрева отправлена");
				break;
			}
			Sleep(10);
		}
	}
}

static void Thread_HeatingCallback(SupercriticalBrain* gui)
{
	gui->RunRegulation();
	AtomicDec(gui->thread_work);
}

void SupercriticalBrain::HeatingCallback()
{
	if (terminated)  return;
	if (thread_work) return;
	
	AtomicInc(thread_work);
	Thread().Run(callback1(Thread_HeatingCallback, this));
}

void SupercriticalBrain::RunRegulation()
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
				
				//Log_AddService("<" + FormatDouble(e_now) + "> <" + FormatDouble(pid_mem.integral) + "> <" + FormatDouble(de) + "> = <" + FormatDouble(u) + "> <" + FormatDouble(pid_mem.max_u) + "> <" + FormatInt(pow) + ">");
				
			}
			// Обновляем значение на экране:
			UpdateValue(7, now_time, pow);
			
			
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