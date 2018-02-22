#include "SupercriticalBrain.h"

#include <HeapOfNeeds/Heap_JsonConfig.h>

bool SupercriticalBrain::RunWatchdog()
{
	/*
	j_int64  w_port;     w_port.SetName(    "WarderPort");
	j_string key_phrase; key_phrase.SetName("KeyPhrase");

	try {
		if (!w_port.SetValueJson(cfg.j_conf["WatchDog"])) {
			return false;
		}
		if (w_port.GetValue() < 0 || w_port.GetValue() > 65535) {
			// Неверный номер порта
			return false;
		}
		if (w_port.GetValue() < 1024) {
			// Указан системный порт
			return false;
		}	
		// ----- Ключевая фраза -----
		if (!key_phrase.SetValueJson(cfg.j_conf["WatchDog"])) {
			// Не указана ключевая фраза
			return false;
		}
		if (key_phrase.GetValue().IsEmpty()) {
			return false;
		}
	} catch (...) {
		return false;
	}
	
	if (!dog.Setup((unsigned int)w_port.GetValue(), key_phrase.GetValue(), cfg.work_freq * 1000))
		return false;
	
	if (!dog.IsSetup()) 
		return false;
	
	dog.BowWow();
	return true;
	*/
	
	if (!Configurate_UnderControlDog(cfg.j_conf["WatchDog"], dog)) 
		return false;
	if (!dog.IsSetup()) 
		return false;
	
	dog.BowWow();
	return true;
}