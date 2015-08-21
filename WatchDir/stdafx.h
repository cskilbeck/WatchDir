//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#include "targetver.h"

//////////////////////////////////////////////////////////////////////
// stdlib

#include <stdio.h>
#include <tchar.h>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <functional>
#include <algorithm>
#include <map>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

//////////////////////////////////////////////////////////////////////
// windows

#include <Windows.h>
#include <Shlobj.h>

//////////////////////////////////////////////////////////////////////
// libs

#include "TinyXML.h"
#include "optionparser.h"

//////////////////////////////////////////////////////////////////////
// mine

#include "Types.h"
#include "Errors.h"
#include "Util.h"
#include "Queue.h"
#include "FileEvent.h"
#include "Command.h"
#include "Condition.h"
#include "Watcher.h"
#include "WatcherList.h"
#include "Args.h"

