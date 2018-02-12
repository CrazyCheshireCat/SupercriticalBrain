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
	Log_AddGood("Начат нагрев до " + FormatDouble(heat_Tset) + " °С");
	Log_AddMessage("Установленное время поддержания температуры " + FormatInt64(heat_duration) + " мин");
}

void SupercriticalBrain::StopHeating()
{
	v_time.Enable();
	v_Tset.Enable();
	Log_AddWarning("Нагрев остановлен вручную");
}