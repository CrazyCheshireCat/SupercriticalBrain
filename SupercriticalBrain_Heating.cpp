#include "SupercriticalBrain.h"

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
	
	if (v_Tset < 50 || v_Tset > 500) {
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

void SupercriticalBrain::StartHeating()
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
	
	Log_AddWarning("Нагрев остановлен вручную");
	KillTimeCallback(CLBK_ID_HEATING);
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
	Time val_time;
	double val_double;
	bool val_quality;
	bool is_hand_controlling;
	int  current_power;
	// Включено ли ручное управление:
	val = opc_src.ReadTag(cfg.tagid_r_handcontrolling, val_time, val_quality);
	is_hand_controlling = val.IsEqual("true");
	UpdateValue(4, val_time, is_hand_controlling ? "да" : "нет");
	// Текущее значение мощности
	val = opc_src.ReadTag(cfg.tagid_r_last_power, val_time, val_quality);
	current_power = ScanInt(val);
	UpdateValue(5, val_time, current_power);
	
	
	// Считываем новые значения:
	val = opc_src.ReadTag(cfg.tagid_s_temperature, val_time, val_quality);
	val_double = ScanDouble(val);
	UpdateValue(1, val_time, val_double);
	if (store_t.Add(val_time, val_double)) {
		// Ура! Новое значение - считаем по нему все, что надо
		
		// -------------------------
	}
	val = opc_src.ReadTag(cfg.tagid_s_pressure, val_time, val_quality);
	val_double = ScanDouble(val);
	UpdateValue(2, val_time, val_double);
	if (store_p.Add(val_time, val_double)) {
		// Ура! Новое значение - считаем по нему все, что надо
		
		// -------------------------
	}
}