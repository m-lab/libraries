// Copyright 2012 M-Lab. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "log.h"

#include <stdio.h>

namespace mlab {
namespace {
#if defined(DEBUG)
  LogSeverity min_severity = VERBOSE;
#else
  LogSeverity min_severity = INFO;
#endif
}  // namespace

void SetLogSeverity(LogSeverity s) {
  min_severity = s;
}

FILE* GetSeverityFD(LogSeverity s) {
  if (s < min_severity)
    return NULL;

  switch (s) {
    case VERBOSE: return stdout;
    case INFO: return stdout;
    case WARNING: return stdout;
    case ERROR: return stderr;
    case FATAL: return stderr;
  }
  return NULL;
}

const char* GetSeverityTag(LogSeverity s) {
  switch (s) {
    case VERBOSE: return "V";
    case INFO: return "I";
    case WARNING: return "W";
    case ERROR: return "E";
    case FATAL: return "F";
  }
  return "";
}

}  // namespace mlab
