#ifndef _SupercriticalBrain_SupercriticalBrain_h
#define _SupercriticalBrain_SupercriticalBrain_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <SupercriticalBrain/SupercriticalBrain.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS SupercriticalBrainImg
#define IMAGEFILE <SupercriticalBrain/SupercriticalBrain.iml>
#include <Draw/iml_header.h>


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
