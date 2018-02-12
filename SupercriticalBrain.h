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

// -----


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
protected:
	int max_count;
	
	void SetBy(const SensDataStore& src);
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
	
	// -----
	double pid_Kp;
	double pid_Ki;
	double pid_Kd;
	int    pid_start_pow;
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

protected:
	SupercriticalBrainCfg cfg;
	double heat_Tset;
	int64  heat_duration;
	
	// Загрузка конфигурации
	bool LoadConfig();	
	void PrintConfig();	
	
	bool CheckParams();
	void StartHeating();
	void StopHeating();
};

bool Configurate_SupercriticalBrain(const char* cfg_path, SupercriticalBrainCfg& cfg);
bool Configurate_SupercriticalBrain(const Value& j_conf, SupercriticalBrainCfg& cfg);
bool Configurate_SupercriticalBrain(SupercriticalBrainCfg& cfg);

#endif
