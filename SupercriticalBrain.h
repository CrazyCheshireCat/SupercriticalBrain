#ifndef _SupercriticalBrain_SupercriticalBrain_h
#define _SupercriticalBrain_SupercriticalBrain_h

#include <OPC_Client/OPC_Client.h>

#include <CtrlLib/CtrlLib.h>
#include <Core/Core.h>

using namespace Upp;

#define LAYOUTFILE <SupercriticalBrain/SupercriticalBrain.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS SupercriticalBrainImg
#define IMAGEFILE <SupercriticalBrain/SupercriticalBrain.iml>
#include <Draw/iml_header.h>

#include <WatchDogWarder/WatchDogWarder.h>

// -----

#define OPC_DFT_SEP '/'
#define CLBK_ID_HEATING 101

class SensDataPoint : Moveable <SensDataPoint> 
{
public:
	Time   ts;	// временная метка
	double v;	// значение
	
	SensDataPoint()                                        { Clear();                }
	SensDataPoint(const Time& src_ts, const double& src_v) { ts = src_ts; v = src_v; }
	SensDataPoint(const SensDataPoint& src)                { SetBy(src);             }
	~SensDataPoint()                                       { Clear();                }
	
	SensDataPoint& operator=(const SensDataPoint& src) { SetBy(src); return *this;                            }
	bool operator==(const SensDataPoint& right)        { return (ts.Get() == right.ts.Get() && v == right.v); }
	inline int64 ts_i() const                          { return ts.Get();                                     }
	
	void Clear() 									    { ts.Set(0); v = 0.0; }
protected:
	void SetBy(const SensDataPoint& src)               { ts = src.ts; v = src.v; }
};

class SensDataStore : Moveable <SensDataStore>
{
	friend SensDataStore;
public:
	Vector <SensDataPoint> line;	
	
	SensDataStore()                          { max_count = 0; }
	SensDataStore(const SensDataStore& src)  { SetBy(src);    }
	~SensDataStore()                         { line.Clear();  }
	
	int GetMaxCount() const     { return max_count;                                      }
	void SetMaxCount(int count) { if (count >= 0) max_count = count; else max_count = 0; }
	bool IsMaxCount() const     { return max_count > 0;                                  }
	
	bool Add(const Time& ts, const double& v) { SensDataPoint d_point(ts, v); return Add(d_point); }
	bool Add(const SensDataPoint& d_point);
	
	void Clear() { line.Clear(); }
	int GetCount() const 		{ return line.GetCount(); }
protected:
	int max_count;
	
	void SetBy(const SensDataStore& src);
};

struct PIDCfg
{
	double Kp;
	double Ki;
	double Kd;
};

struct SupercriticalBrainCfg
{
	int work_freq;
	String opc_source_name;
	String opc_s_tag_temperature;
	String opc_s_tag_pressure;
	String opc_result_name;
	String opc_r_tag_handcontrolling;
	String opc_r_tag_power;
	String opc_r_tag_last_power;
	
	int tagid_s_temperature;
	int tagid_s_pressure;
	int tagid_r_handcontrolling;
	int tagid_r_power;
	int tagid_r_last_power;
	// -----

	PIDCfg pid_tempt;
	PIDCfg pid_press;

	//int    pid_start_pow;
	//int    pid_max_pow;
	
	Value j_conf;
	
	bool is_ok;
};

class PID_Regulator// : Moveable <PID_Regulator>
{
	friend PID_Regulator;
public:
	PID_Regulator()                         { Clear();    }
	PID_Regulator(const PID_Regulator& src) { SetBy(src); }
	~PID_Regulator()                        { Clear();    }
	
	bool SetPID_Coeff(double K_proportional, double K_integral, double K_differential);
	bool SetPID_Coeff_Kp(double K_proportional);
	bool SetPID_Coeff_Ki(double K_integral);
	bool SetPID_Coeff_Kd(double K_differential);
	
	bool Start(double temperature_to_set, int64 time_to_sustain_min);
	bool IsStopped() const { return (T_set < 0); }
	bool IsStarted() const { return (T_set > 0); }
	void Stop();
	int  GetPower(const Time& t, const double& v);
	
	double GetCurrentDiff() const 			 { return e_last; }
	double GetCurrentValueDerivative() const { return -de; }
	
	void SetSensitivity(double sensitivity)			   { if (sensitivity > 0) T_sensitivity = sensitivity;   }
	void SetUVariation(int variation_sec)              { t_u_variation = variation_sec; }
	bool SetMaxPower(int max_pow);
	bool SetStartPower(int start_pow);
	
	void Clear();
	
	int64 GetCurrentSustainTime() const;
	int64 GetCurrentObtainTimestamp() const { return ts_obtain; }
protected:
	double T_set;
	int64  t_sustain;
	
	double Kp;
	double Ki;
	double Kd;
	
	double T_sensitivity;
	int    t_u_variation;
	int    p_maximum;
	int    p_start;
	
	int64  ts_start;
	int64  ts_obtain;
	int64  ts_last;
	double e_last;
	double i_last;
	double u_max;	
	int    pow;
	int    pow_last;
	
	void Reset();
	
private:
	void SetBy(const PID_Regulator& src);
	double e, i, de, dt, u;
	int64  t_now;
};

class SupercriticalBrain : public WithSupercriticalBrainLayout<TopWindow> 
{
	typedef SupercriticalBrain CLASSNAME;
public:

	volatile Atomic terminated;			//< Сигнал завершения работы приложения
	volatile Atomic thread_work;		//< Поток для работы
	
	SupercriticalBrain();
	~SupercriticalBrain();
	
	// Работа c окном
	void CloseMe()				  { terminated = 1; Break(); /* TopWindow::Close(); */                 }
	void HideMe() 				  { if (this->IsShown())  this->Hide();                                }
	void ShowMe()                 { if (!this->IsShown()) this->Show();                                }
	void AutoHide()				  { SetTimeCallback(100, callback(this, &SupercriticalBrain::HideMe)); }	
	// Работа с логом
	void Log_AddText(AttrText mes);
	void Log_AddMessage(const char* mes)  { Log_AddText(AttrText(mes));                           }
	void Log_AddError(const char* mes)    { Log_AddText(AttrText(mes).Ink(Red()));                }
	void Log_AddWarning(const char* mes)  { Log_AddText(AttrText(mes).Ink(Color(255, 127, 39)));  }
	void Log_AddService(const char* mes)  { Log_AddText(AttrText(mes).Ink(Gray()));               }
	void Log_AddGood(const char* mes)     { Log_AddText(AttrText(mes).Ink(Green()));              }
	void Log_AddCritical(const char* mes) {	Log_AddText(AttrText(mes).Ink(White()).Paper(Red())); }	
protected:
	TrayIcon trayicon;
	void TrayMenu(Bar& bar);
	void StandartInit();
	void StandartKill();
		
public:
	void RunRegulation();
		
protected:
	SupercriticalBrainCfg cfg;
	double heat_Tset;
	double heat_Pset;
	int64  heat_duration;
	
	int    heat_start_pow;
	int    heat_max_pow;
	
	SimpleClientOPC opc_src;	// OPC-клиент для источника данных
	SimpleClientOPC opc_ctr;	// OPC-клиент для управления
	
	UnderControlDog dog;
	
	// Загрузка конфигурации
	bool LoadConfig();	
	void PrintConfig();	
	bool SaveConfig();
	
	void InitServers();
	
	bool CheckParams();
	void Push_StartHeating();
	void HeatingCallback();
	void Push_StopHeating();
	void StopHeating();
	
	bool ConnectOPC_SRC();
	bool ConnectOPC_CTR();
	
	void UpdateValue(int pos, const Value& time, const Value& val);
	void ClearValues();
private:
	SensDataStore store_t;
	SensDataStore store_p;
	PID_Regulator pid_T;
	PID_Regulator pid_P;
		
	void SetNullPower();
	bool RunWatchdog();
};

bool Configurate_SupercriticalBrain(const char* cfg_path, SupercriticalBrainCfg& cfg);
//bool Configurate_SupercriticalBrain(const Value& j_conf, SupercriticalBrainCfg& cfg);
bool Configurate_SupercriticalBrain(SupercriticalBrainCfg& cfg);
void UpdateConfig_SupercriticalBrain(const char* cfg_path, SupercriticalBrainCfg& cfg);

#endif
