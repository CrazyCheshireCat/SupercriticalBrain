#ifndef _SupercriticalBrain_SupercriticalBrain_h
#define _SupercriticalBrain_SupercriticalBrain_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <SupercriticalBrain/SupercriticalBrain.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS SupercriticalBrainImg
#define IMAGEFILE <SupercriticalBrain/SupercriticalBrain.iml>
#include <Draw/iml_header.h>

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

protected:
	void SetBy(const SensDataPoint& src)               { ts = src.ts; v = src.v; }
};

class SensDataStore : Moveable <SensDataStore>
{
public:
	Vector <SensDataPoint> line;	
	
	SensDataStore() { max_count = 0; }
	SensDataStore(const SensDataStore& src) { SetBy(src); }
	~SensDataStore() { line.Clear(); }
	
	int GetMaxCount() const     { return max_count; }
	void SetMaxCount(int count) { if (count >= 0) max_count = count; else max_count = 0; }
	bool IsMaxCount() const     { return max_count > 0; }
	
	bool Add(const Time& ts, const double& v) { SensDataPoint d_point(ts, v); return Add(d_point); }
	bool Add(const SensDataPoint& d_point);
	
	void Clear() { line.Clear(); }
protected:
	int max_count;
	
	void SetBy(const SensDataStore& src);
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
	// Загрузка конфигурации
	bool LoadConfig(const char* path);	
};

#endif
