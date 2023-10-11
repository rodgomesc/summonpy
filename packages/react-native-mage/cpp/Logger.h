//
// Created by rodrigo gomes on 28/07/23.
//

#include <jsi/jsilib.h>
#include <jsi/jsi.h>

void installLogger(facebook::jsi::Runtime &jsiRuntime);
void logMessage(facebook::jsi::Runtime &jsiRuntime, const std::string &tag, const std::string &message);
