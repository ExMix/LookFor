#pragma once

#include <QtGlobal>

#ifdef QT_DEBUG
  #define VERIFY(x) Q_ASSERT(x)
#else
  #define VERIFY(x) x
#endif

#include <functional>
using std::bind;
using std::function;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;
using std::placeholders::_6;
using std::placeholders::_7;
using std::placeholders::_8;

