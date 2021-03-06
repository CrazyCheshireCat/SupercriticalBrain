#include "SupercriticalBrain.h"

#define DEBUG_MODE_ON true

SupercriticalBrain::SupercriticalBrain()
{
	SetLanguage(LNGC_('r','u','R','U', CHARSET_UTF8));
	CtrlLayout(*this, "SupercriticalBrain");
	Icon(SupercriticalBrainImg::monitor_16(), SupercriticalBrainImg::monitor_32());
	
	//v_Pset = 100;
	//v_time = 100;
	//v_Tset = 100;
	
	v_Pmax  = 100;
	v_Pstart = 100;
	
	StandartInit();
	
	btn_stop.Disable();
	
	// Грузим конфиг
	if (!LoadConfig()) {
		Log_AddCritical("Неверный файл конфигурации, работа программы будет завершена");
		SetTimeCallback(10000, callback(this, &SupercriticalBrain::CloseMe));
		return;
	}
	PrintConfig();
	
	opt_param = 0;
		
	wnd_vals.AddColumn("Параметр", 3);
	wnd_vals.AddColumn("Время",    2);
	wnd_vals.AddColumn("Значение", 1);
	
	wnd_vals.Add("Сервер исходн. данных");					// 0
	wnd_vals.Add("Контрольная температура");				// 1
	wnd_vals.Add("Контрольное давление");					// 2
	wnd_vals.Add("Сервер управления нагревом");				// 3
	wnd_vals.Add("Управление");								// 4
	wnd_vals.Add("Текущее значение мощности");				// 5
	wnd_vals.Add("");
	wnd_vals.Add("Вычисленное значение мощности (по T°)");	// 7
	wnd_vals.Add("Время достижения заданной T°");			// 8
	wnd_vals.Add("Вычисленное значение мощности (по p)");	// 9
	wnd_vals.Add("Время достижения заданного p");			// 10
	wnd_vals.Add("");
	wnd_vals.Add("Производная T°");							// 12
	wnd_vals.Add("Производная p");							// 13
	wnd_vals.Add("Невязка T°");								// 14
	wnd_vals.Add("Невязка p");								// 15
	btn_start <<= THISBACK(Push_StartHeating);
	btn_stop  <<= THISBACK(Push_StopHeating);
	
	InitServers();
	// ----- WATCHDOG -----
	if (!RunWatchdog()) 
		Log_AddWarning("Не найдена конфигурация для Watchdog. Watchdog не запущен");
	// ----- DATA STORES -----
	store_t.SetMaxCount(20);
	store_p.SetMaxCount(20);
}

void SupercriticalBrain::UpdateValue(int pos, const Value& time, const Value& val)
{
	wnd_vals.Set(pos, 1, time);
	wnd_vals.Set(pos, 2, val);
}

void SupercriticalBrain::ClearValues()
{
	for (int i = 0; i < wnd_vals.GetCount(); ++i) {
		wnd_vals.Set(i, 1, "");
		wnd_vals.Set(i, 2, "");
	}
}

SupercriticalBrain::~SupercriticalBrain()
{
	KillTimeCallback(CLBK_ID_HEATING);
	terminated = 1; 
	while (thread_work) 
		Sleep(10);
	
	StandartKill();
	dog.Silence();
	SaveConfig();
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
	wnd_log.AddColumn("Время",     1);
	wnd_log.AddColumn("Сообщение", 5);
	wnd_log.NoCursor();
	
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