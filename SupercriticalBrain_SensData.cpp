#include "SupercriticalBrain.h"


bool SensDataStore::Add(const SensDataPoint& d_point)
{
	// Добавляем новое значение в конец массива, если его временная метка БОЛЬШЕ,
	// чем у последнего 
	if (line.GetCount() > 0) {
		if (line[line.GetCount() - 1].ts_i() >= d_point.ts_i())
			return false;
	}
	line.Add(d_point);
	// Убираем первое значение, если превысили допустимый размер
	if (max_count > 0 && line.GetCount() > max_count) {
		line.Remove(0, 1);
	}
	return true;
}
	
void SensDataStore::SetBy(const SensDataStore& src)
{
	line.Clear();
	max_count = GetMaxCount();
	for (int i = 0; i < src.line.GetCount(); ++i)
		line.Add(src.line[i]);
}