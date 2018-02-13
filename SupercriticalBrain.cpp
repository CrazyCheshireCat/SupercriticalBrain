#include "SupercriticalBrain.h"

#define DEBUG_MODE_ON true

SupercriticalBrain::SupercriticalBrain()
{
	SetLanguage(LNGC_('r','u','R','U', CHARSET_UTF8));
	CtrlLayout(*this, "SupercriticalBrain");
	Icon(SupercriticalBrainImg::monitor_16(), SupercriticalBrainImg::monitor_32());
	
	StandartInit();
	
	btn_stop.Disable();
	
	// Грузим конфиг
	if (!LoadConfig()) {
		Log_AddCritical("Неверный файл конфигурации, работа программы будет завершена");
		SetTimeCallback(10000, callback(this, &SupercriticalBrain::CloseMe));
		return;
	}
	PrintConfig();
		
	wnd_vals.AddColumn("Параметр", 3);
	wnd_vals.AddColumn("Время",    2);
	wnd_vals.AddColumn("Значение", 1);
	wnd_vals.Add("Сервер исходн. данных");
	wnd_vals.Add("Контрольная температура");
	wnd_vals.Add("Контрольное давление");
	wnd_vals.Add("Сервер управления нагревом");
	wnd_vals.Add("Ручное управление");
	wnd_vals.Add("Текущее значение мощности");
	wnd_vals.Add("");
	wnd_vals.Add("Вычисленное значение мощности");
	
		
	btn_start <<= THISBACK(StartHeating);
	btn_stop  <<= THISBACK(StopHeating);
	
	InitServers();
	// ------- DATA STORES -------
	store_t.SetMaxCount(20);
	store_p.SetMaxCount(20);
}

void SupercriticalBrain::UpdateValue(int pos, const Value& time, const Value& val)
{
	wnd_vals.Set(pos, 1, time);
	wnd_vals.Set(pos, 2, val);
}

SupercriticalBrain::~SupercriticalBrain()
{
	KillTimeCallback(CLBK_ID_HEATING);
	terminated = 1; 
	while (thread_work) 
		Sleep(10);
	
	StandartKill();
	
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