#include "SupercriticalBrain.h"

#define DEBUG_MODE_ON true

SupercriticalBrain::SupercriticalBrain()
{
	SetLanguage(LNGC_('r','u','R','U', CHARSET_UTF8));
	CtrlLayout(*this, "SupercriticalBrain");
	Icon(SupercriticalBrainImg::monitor_16(), SupercriticalBrainImg::monitor_32());
	
	StandartInit();
	
	// Грузим конфиг
	if (!LoadConfig()) {
		Log_AddCritical("Неверный файл конфигурации, работа программы будет завершена");
		SetTimeCallback(10000, callback(this, &SupercriticalBrain::CloseMe));
		return;
	}
	PrintConfig();
		
	btn_start <<= THISBACK(StartHeating);
	btn_stop  <<= THISBACK(StopHeating);
}

SupercriticalBrain::~SupercriticalBrain()
{
	terminated = 1; 
	while (thread_work) 
		Sleep(10);
	
	StandartKill();
}

void SupercriticalBrain::Log_AddText(AttrText mes)
{
	if (wnd_log.GetCount() > 200) wnd_log.Clear();
	Time ts = GetSysTime();
	if (mes.text.GetCount() <= 0) {
		wnd_log.Add("", "");
	} else {
		wnd_log.Add(ts, mes);
	}
	wnd_log.GoEnd();
	wnd_log.Refresh();
}

void SupercriticalBrain::TrayMenu(Bar& bar)
{
	if (this->IsShown())
		bar.Add(t_("Скрыть"),   SupercriticalBrainImg::hide_16(),  THISBACK(HideMe));
	else 	
		bar.Add(t_("Показать"), SupercriticalBrainImg::show_16(),  THISBACK(ShowMe));
	
	bar.Separator();
	bar.Add(t_("Выход"),        SupercriticalBrainImg::exit_16(),  THISBACK(CloseMe));	
}

void SupercriticalBrain::StandartInit()
{
	terminated  = 0;		//< Сигнал завершения работы приложения
	thread_work = 0;		//< Поток для работы
	
	// Лог	
	wnd_log.AddColumn("Время",     3);
	wnd_log.AddColumn("Сообщение", 5);
	
	// Окно	
	this->Sizeable(true);
	this->MinimizeBox(true);
	this->MaximizeBox(true);
	
	if (DEBUG_MODE_ON)
		this->WhenClose = THISBACK(CloseMe);
	else 
		this->WhenClose = THISBACK(HideMe);
	
	// Иконка в трее
	trayicon.Icon(SupercriticalBrainImg::monitor_16());
	trayicon.Tip("SupercriticalBrain");
	trayicon.WhenBar = THISBACK(TrayMenu);	
}

void SupercriticalBrain::StandartKill()
{
	trayicon.Hide();
}