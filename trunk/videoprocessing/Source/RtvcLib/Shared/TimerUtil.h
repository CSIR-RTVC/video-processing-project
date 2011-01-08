#pragma once

#include <windows.h>

class TimerUtil
{
public:

  TimerUtil()
    :m_dPCFreq(0.0),
    m_tStart(0)
  {

  }


  void start()
  {
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li)) return;
      //cout << "QueryPerformanceFrequency failed!\n";

    m_dPCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    m_tStart = li.QuadPart;
  }

  double stop()
  {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-m_tStart)/m_dPCFreq;
  }

private:
  double m_dPCFreq;
  __int64 m_tStart;
};
