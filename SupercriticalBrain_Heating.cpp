#include "SupercriticalBrain.h"

bool SupercriticalBrain::CheckParams()
{
	if (v_Tset.GetData().IsNull()) {
		PromptOK("Не задана температура для нагрева");
		v_Tset.SetFocus();
		return false;
	}
	
	if (v_Pset.GetData().IsNull()) {
		PromptOK("Не задано давление для нагрева");
		v_Pset.SetFocus();
		return false;
	}
	
	if (v_time.GetData().IsNull()) {
		PromptOK("Не задана длительность поддержания температуры");
		v_time.SetFocus();
		return false;
	}
	
	if (v_Pmax.GetData().IsNull()) {
		PromptOK("Не задано максимальное значение мощности");
		v_Pmax.SetFocus();
		return false;
	}
	
	if (v_Pstart.GetData().IsNull()) {
		PromptOK("Не задано стартовое значение мощности");
		v_Pstart.SetFocus();
		return false;
	}
	
	if (v_Tset < 20 || v_Tset > 600) {
		PromptOK("Недопустимое значение температуры для нагрева");
		v_Tset.SetFocus();
		return false;
	}
	if (v_Pset < 1 || v_Pset > 600) {
		PromptOK("Недопустимое значение давления для нагрева");
		v_Tset.SetFocus();
		return false;
	}	

	if (v_time <= 0 || v_time > 24*60) {
		PromptOK("Недопустимое время поддержания заданной температуры");
		v_time.SetFocus();
		return false;
	}
	
	if (v_Kp_T < 0) {
		PromptOK("Недопустимое значение коэффициента Kp");
		v_Kp_T.SetFocus();
		return false;
	}
	
	if (v_Ki_T < 0 || v_Ki_T >= 1) {
		PromptOK("Недопустимое значение коэффициента Ki");
		v_Ki_T.SetFocus();
		return false;
	}
	
	if (v_Kd_T < 0) {
		PromptOK("Недопустимое значение коэффициента Kd");
		v_Kd_T.SetFocus();
		return false;
	}
	
	if (v_Kp_P < 0) {
		PromptOK("Недопустимое значение коэффициента Kp");
		v_Kp_P.SetFocus();
		return false;
	}
	
	if (v_Ki_P < 0 || v_Ki_P >= 1) {
		PromptOK("Недопустимое значение коэффициента Ki");
		v_Ki_P.SetFocus();
		return false;
	}
	
	if (v_Kd_P < 0) {
		PromptOK("Недопустимое значение коэффициента Kd");
		v_Kd_P.SetFocus();
		return false;
	}
	
	if (v_Pmax < 1 || v_Pmax > 100) {
		PromptOK("Недопустимое значение максимальной мощности");
		v_Pmax.SetFocus();
		return false;
	}
	
	if (v_Pstart < 1 || v_Pstart > 100) {
		PromptOK("Недопустимое значение стартовой мощности");
		v_Pstart.SetFocus();
		return false;
	}	
		
	cfg.pid_tempt.Kp = ~v_Kp_T;
	cfg.pid_tempt.Ki = ~v_Ki_T;
	cfg.pid_tempt.Kd = ~v_Kd_T;
	cfg.pid_press.Kp = ~v_Kp_P;
	cfg.pid_press.Ki = ~v_Ki_P;
	cfg.pid_press.Kd = ~v_Kd_P;
	
	heat_Tset        = ~v_Tset;
	heat_Pset        = ~v_Pset;
	heat_duration    = ~v_time;
	
	heat_start_pow   = ~v_Pstart;
	heat_max_pow     = ~v_Pmax;
	
	return true;
}

void SupercriticalBrain::Push_StartHeating()
{
	ClearValues();
	// ----- Проверяем введенные параметры перед началом нагрева -----
	if (!CheckParams()) {
		Log_AddError("Неверные значения для начала работы");
		return;
	}
	
	// ----- Настраиваем ПИД-регулятор (по температуре) -----
	if (!pid_T.SetPID_Coeff(cfg.pid_tempt.Kp, cfg.pid_tempt.Ki, cfg.pid_tempt.Kd)) {
		Log_AddError("Неверные значения для начала работы");
		return;
	}
	// Чувствительность для температуры - 1 градус
	pid_T.SetSensitivity(1.0);
	// Время для установки максимального значения u(t) - 60 сек
	pid_T.SetUVariation(60); 
	// Максимальное значение:
	pid_T.SetMaxPower(heat_max_pow); 
	// Стартовое значение:
	pid_T.SetStartPower(heat_start_pow); 
	
	// ----- Стартуем регулятор с заданной температурой и на заданное время -----
	if (!pid_T.Start(heat_Tset, heat_duration)) {
		Log_AddError("Неверные значения для начала работы");
		return;
	}
	
	// ----- Настраиваем ПИД-регулятор (по давлению) -----
	if (!pid_P.SetPID_Coeff(cfg.pid_press.Kp, cfg.pid_press.Ki, cfg.pid_press.Kd)) {
		Log_AddError("Неверные значения для начала работы");
		return;
	}
	// Чувствительность для давления - 1 бар
	pid_P.SetSensitivity(1.0);
	// Время для установки максимального значения u(t) - 60 сек
	pid_P.SetUVariation(60); 
	// Максимальное значение:
	pid_P.SetMaxPower(heat_max_pow); 
	// Стартовое значение:
	pid_P.SetStartPower(heat_start_pow); 
	// ----- Стартуем регулятор с заданным давлением и на заданное время -----
	if (!pid_P.Start(heat_Pset, heat_duration)) {
		Log_AddError("Неверные значения для начала работы");
		return;
	}

	
	// ----- GUI -----
	v_time.Disable();
	v_Tset.Disable();
	v_Pmax.Disable();
	v_Pstart.Disable();
	
	v_Kp_T.Disable();
	v_Ki_T.Disable();
	v_Kd_T.Disable();
	v_Kp_P.Disable();
	v_Ki_P.Disable();
	v_Kd_P.Disable();
	
	btn_start.Disable();
	btn_stop.Enable();
	
	// ----- Выводим в лог сообщение о начале работы -----
	Log_AddGood("Начат нагрев до " + FormatDouble(heat_Tset) + " °С и/или давления " + FormatDouble(heat_Pset) + " бар" );
	Log_AddMessage("Установленное время выдержки " + FormatInt64(heat_duration) + " мин");
	
	HeatingCallback();
	SetTimeCallback(-(cfg.work_freq * 1000), callback(this, &SupercriticalBrain::HeatingCallback), CLBK_ID_HEATING);
}

void SupercriticalBrain::StopHeating()
{
	v_time.Enable();
	v_Tset.Enable();
	v_Pmax.Enable();
	v_Pstart.Enable();
	v_Kp_T.Enable();
	v_Ki_T.Enable();
	v_Kd_T.Enable();
	v_Kp_P.Enable();
	v_Ki_P.Enable();
	v_Kd_P.Enable();
		
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
	// 
	String val;						// Для чтения значения о OPC-сервера
	Time   val_time;				// Для чтения метки времени с OPC-сервера
	bool   val_quality;				// Для чтения качества значения с OPC-сервера
	
	double T, P;					// Температура и давление по контрольным каналам
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
	
	int pow_T;
	int pow_P;	
	
	int controlling_option = ~opt_param;
	
	// ----- Текущее значение контрольной температуры ------
	val = opc_src.ReadTag(cfg.tagid_s_temperature, val_time, val_quality);
	T = ScanDouble(val);
	UpdateValue(1, val_time, T);

	if (pid_T.GetCurrentObtainTimestamp() > 0) {
		Time ts;
		ts.Set(pid_T.GetCurrentObtainTimestamp());
		Time ts1;
		ts1.Set(pid_T.GetCurrentSustainTime());
		UpdateValue(8, ts, FormatTime(ts1, "hh:mm:ss"));
	}
	
	if (val_quality) {
		if (store_t.Add(val_time, T)) { // Значение обновилось - можно пересчитать мощность
			pow_T = pid_T.GetPower(val_time, T);
			if (pid_T.IsStopped()) {
				Log_AddGood("Время поддержания заданной температуры истекло. Остановка нагрева!");
				// Вырубаем нагрев:
				pow_T = 0;
				StopHeating();
			}
			// Обновляем значение на экране:
			UpdateValue(7, now_time, pow_T);

			// =================================================================================
			// Пишем на управляющий сервер, если ручное управление отключено
			if (controlling_option == 0) {	// Если выставлен контроль по температуре
				if (!is_hand_controlling) {
					for (int i = 0; i < 5; ++i) {
						if (opc_ctr.WriteTag(cfg.tagid_r_power, FormatInt(pow_T))) break;
					}
				}
			}
			// =================================================================================
		}
	}



	// ----- Текущее значение контрольного давления ------
	val = opc_src.ReadTag(cfg.tagid_s_pressure, val_time, val_quality);
	P = ScanDouble(val);
	UpdateValue(2, val_time, P);
	
	if (pid_P.GetCurrentObtainTimestamp() > 0) {
		Time ts;
		ts.Set(pid_P.GetCurrentObtainTimestamp());
		Time ts1;
		ts1.Set(pid_P.GetCurrentSustainTime());
		UpdateValue(10, ts, FormatTime(ts1, "hh:mm:ss"));
	}
		
	if (val_quality) {
		if (store_p.Add(val_time, P)) {
			// Ура! Новое значение - считаем по нему все, что надо
			pow_P = pid_P.GetPower(val_time, T);
			if (pid_P.IsStopped()) {
				Log_AddGood("Время поддержания заданной температуры истекло. Остановка нагрева!");
				// Вырубаем нагрев:
				pow_P = 0;
				StopHeating();
			}
			// Обновляем значение на экране:
			UpdateValue(9, now_time, pow_P);
			
			// =================================================================================
			// Пишем на управляющий сервер, если ручное управление отключено
			if (controlling_option == 1) {	// Если выставлен контроль по давлению
				if (!is_hand_controlling) {
					for (int i = 0; i < 5; ++i) {
						if (opc_ctr.WriteTag(cfg.tagid_r_power, FormatInt(pow_P))) break;
					}
				}
			}
			// =================================================================================
		}
	}
}

