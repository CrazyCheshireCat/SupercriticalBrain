#include "SupercriticalBrain.h"

#define DFT_POWER_LIMIT 100

////////////////////////////////////////////////////////////////////////////////////////////////
// class PID_Regulator : Moveable <PID_Regulator>

void PID_Regulator::SetBy(const PID_Regulator& src)
{
	T_set         = src.T_set;
	t_sustain     = src.t_sustain;
	Kp            = src.Kp;
	Ki            = src.Ki;
	Kd            = src.Kd;
	T_sensitivity = src.T_sensitivity;
	t_u_variation = src.t_u_variation;
	ts_start      = src.ts_start;
	ts_obtain     = src.ts_obtain;
	ts_last       = src.ts_last;
	e_last        = src.e_last;
	i_last        = src.i_last;
	u_max         = src.u_max;
}

void PID_Regulator::Stop()
{
	T_set     = -1;
	t_sustain = 0;
}

void PID_Regulator::Reset()
{
	ts_start  = 0;
	ts_obtain = 0;
	ts_last   = 0;
	e_last    = 0;
	i_last    = 0;
	u_max     = 0;	
	pow       = 0;
	pow_last  = 0;
}

void PID_Regulator::Clear()
{
	Stop();
		
	Kp = 0;
	Ki = 0;
	Kd = 0;
	
	T_sensitivity = 0.0;
	t_u_variation = 0;
	
	Reset();
}

bool PID_Regulator::SetPID_Coeff(double K_proportional, double K_integral, double K_differential)
{
	if (K_proportional < 0)               return false;
	if (K_differential < 0)               return false;
	if (K_integral < 0 || K_integral >=1) return false;
	Kp = K_proportional;
	Ki = K_integral;
	Kd = K_differential;
	return true;
}

bool PID_Regulator::SetPID_Coeff_Kp(double K_proportional)
{
	if (K_proportional < 0)               return false;
	Kp = K_proportional;
	return true;	
}

bool PID_Regulator::SetPID_Coeff_Ki(double K_integral)
{
	if (K_integral < 0 || K_integral >=1) return false;
	Ki = K_integral;
	return true;	
}

bool PID_Regulator::SetPID_Coeff_Kd(double K_differential)
{
	if (K_differential < 0)               return false;
	Kd = K_differential;
	return true;	
}
	
bool PID_Regulator::Start(double temperature_to_set, int64 time_to_sustain_min)
{
	if (temperature_to_set <= 10) return false;
	if (temperature_to_set > 600) return false;
	if (time_to_sustain_min <= 0) time_to_sustain_min = -1;	// отрицательное значение означает, что время поддержания
															// не отслеживается
	Reset();
	T_set     = temperature_to_set;
	t_sustain = time_to_sustain_min * 60;	//переводим в секунды
	
	return true;
}

int64 PID_Regulator::GetCurrentSustainTime() const
{
	if (t_sustain <= 0) return 0;
	return GetSysTime().Get() - ts_obtain;
}

int PID_Regulator::GetPower(const Time& t, const double& v)
{
	// Если нагрев был остановлен - мощность нулевая
	if (IsStopped()) return 0;
	// Текущая невязка:
	e = T_set - v;
	// Текущее время
	t_now = GetSysTime().Get();
	// Для первого значения не считаем мощность - все обновляем, все инициализируем
	if (ts_start == 0) {
		ts_start = t.Get();		//< Время начала нагрева
		e_last   = e;			//< Прошлое значение невязки
		ts_last  = t.Get();		//< Временная метка для прошлого значения невязки
		
		return DFT_POWER_LIMIT;
	}
	
	// ----- Если значение не первое - уже можно что-то посчитать: -----
	// Шаг:
	dt = (double)(t.Get() - ts_last);
	ts_last = t.Get();
	// Интеграл по невязке:
	i  = i_last + e * dt;
	i_last = i;
	// Скорость изменения невязки
	de = (e - e_last) / dt;
	e_last = e;
	// u(t)
	u = Kp * e +
		Ki * i +
		Kd * de;
	
	// Определяем, достигнута ли нужная температура
	if (e <= T_sensitivity || e < 0) {	// Или в заданных пределах или чуть перегрето
		if (ts_obtain == 0) 
			ts_obtain = t_now;
	}
	// Если заданная температура была достигнута, проверяем, сколько мы уже греем на этом уровне температуры
	if (t_sustain > 0 && ts_obtain > 0) {	// Если время нагрева отслеживается программно - проверяем его
		if (t_now - ts_obtain >= t_sustain) {
			// Греем уже нужное время, поэтому вырубаем все
			Stop();
			pow_last = pow;	// Прошлое значение запоминаем
			pow      = 0;	// Новое значение - "выкл."
			return pow;
		}
	}
	
	// Определяем макмимальное u(t)
	if (t_u_variation > 0) {
		if (t_now - ts_start <= t_u_variation) u_max = u;
	} else {
		if (u > u_max)                         u_max = u;
	}
	// Запоминаем прошлое значение:
	pow_last = pow;
	// Вычисляем процент процент подаваемой мощности:
	pow = (int)((u / u_max) * DFT_POWER_LIMIT + 0.5); // Плюс округляем в большую сторону
	if (pow < 0)               pow = 0;
	if (pow > DFT_POWER_LIMIT) pow = DFT_POWER_LIMIT;
	return pow;
}
