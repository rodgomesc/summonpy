//
// Created by rodrigo gomes on 28/07/23.
//

#include "Logger.h"
#include "jsi/jsi.h"
#include <mutex>

std::mutex logMutex;


using namespace facebook;

void installLogger(facebook::jsi::Runtime &rt)
{
  auto logMessage = jsi::Function::createFromHostFunction(
      rt,
      jsi::PropNameID::forAscii(rt, "logMessage"),
      2, // number of arguments
      [](jsi::Runtime &runtime,
         const jsi::Value &thisValue,
         const jsi::Value *arguments,
         size_t count) -> jsi::Value
      {
        // Getting the global object
        auto global = runtime.global();

        // Getting the console object
        auto console = global.getProperty(runtime, "console");

        // Getting the log function
        auto log = console.asObject(runtime).getProperty(runtime, "log");

        // Check if there are two arguments and both are strings
        if (count > 1 && arguments[0].isString() && arguments[1].isString())
        {
          // Concatenating tag and message with ': ' separator
          std::string logString = arguments[0].getString(runtime).utf8(runtime) + ": " + arguments[1].getString(runtime).utf8(runtime);

          // Calling log function with the concatenated string as the message
          log.asObject(runtime).asFunction(runtime).call(runtime, logString);
        }

        return jsi::Value::undefined();
      });

  rt.global().setProperty(rt, "logMessage", std::move(logMessage));
}
void logMessage(facebook::jsi::Runtime &rt, const std::string &tag, const std::string &message) {
    std::lock_guard<std::mutex> lock(logMutex); // Automatically locks the mutex and unlocks it when out of scope

  // Getting the logMessage function from the global object
  auto msg = rt.global().getProperty(rt, "logMessage");

  // Calling the function with the tag and message as parameters
    msg.asObject(rt).asFunction(rt).call(rt, jsi::String::createFromAscii(rt, tag), jsi::String::createFromAscii(rt, message));
}
