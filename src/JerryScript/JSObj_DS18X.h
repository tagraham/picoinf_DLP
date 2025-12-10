#pragma once

#include <string>
#include <unordered_set>
#include <vector>
using namespace std;

#include "JerryScriptUtl.h"

#include "DS18X.h"


class JSObj_DS18X
{
public:

    ///////////////////////////////////////////////////////////////////////////
    // Public Configuration Interface
    ///////////////////////////////////////////////////////////////////////////

    static void SetPinWhitelist(const vector<uint8_t> &pinList)
    {
        pinWhitelistSet_.clear();
        pinWhitelistSet_.insert(pinList.begin(), pinList.end());

        pinWhitelistStr_ = ContainerToString(pinList);
    }


    ///////////////////////////////////////////////////////////////////////////
    // Public Registration Interface
    ///////////////////////////////////////////////////////////////////////////

    static void Register()
    {
        JerryScript::UseThenFree(jerry_function_external(OnConstructed), [](auto jsFnObj){
            JerryScript::SetGlobalPropertyNoFree("DS18X", jsFnObj);

            JerryScript::UseThenFreeNewObj([&](auto prototype){
                JerryScript::SetPropertyNoFree(jsFnObj, "prototype", prototype);

                JerryScript::SetPropertyToJerryNativeFunction(prototype, "IsAlive",                  JsFnHandler);
                JerryScript::SetPropertyToJerryNativeFunction(prototype, "SetResolution",            JsFnHandler);
                JerryScript::SetPropertyToJerryNativeFunction(prototype, "GetTemperatureCelsius",    JsFnHandler);
                JerryScript::SetPropertyToJerryNativeFunction(prototype, "GetTemperatureFahrenheit", JsFnHandler);
            });
        });
    }


private:

    ///////////////////////////////////////////////////////////////////////////
    // JavaScript Function Handlers
    ///////////////////////////////////////////////////////////////////////////

    static jerry_value_t JsFnHandler(const jerry_call_info_t *callInfo,
                                     const jerry_value_t      argv[],
                                     const jerry_length_t     argc)
    {
        jerry_value_t retVal = jerry_undefined();

        DS18X *obj = (DS18X *)JerryScript::GetNativePointer(callInfo->this_value, &typeInfo_);

        string fnName = JerryScript::GetInternalPropertyAsString(callInfo->function, "name");

        if (!obj)
        {
            retVal = jerry_throw_sz(JERRY_ERROR_REFERENCE, "Failed to retrieve object");
        }
        else if (argc == 0)
        {
            if (fnName == "IsAlive")
            {
                retVal = jerry_number(obj->IsAlive());
            }
            else if (fnName == "GetTemperatureCelsius")
            {
                double val = obj->GetTemperatureCelsius();

                // check for sentinel value indicating either CRC error or IsAlive failure
                if (val != -1000)
                {
                    retVal = jerry_number(val);
                }
                else
                {
                    retVal = jerry_throw_sz(JERRY_ERROR_COMMON, "Device Error");
                }
            }
            else if (fnName == "GetTemperatureFahrenheit")
            {
                double val = obj->GetTemperatureFahrenheit();

                // check for sentinel value indicating either CRC error or IsAlive failure
                if (val != -1768)
                {
                    retVal = jerry_number(val);
                }
                else
                {
                    retVal = jerry_throw_sz(JERRY_ERROR_COMMON, "Device Error");
                }
            }
            else
            {
                retVal = jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid function arguments");
            }
        }
        else if (argc == 1)
        {
            if (fnName == "SetResolution")
            {
                if (jerry_value_is_number(argv[0]))
                {
                    uint8_t bits = (uint8_t)jerry_value_as_number(argv[0]);

                    obj->SetResolution(bits);
                }
                else
                {
                    return jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid function arguments");
                }
            }
            else
            {
                retVal = jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid function arguments");
            }
        }
        else
        {
            retVal = jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid function arguments");
        }

        return retVal;
    }


    ///////////////////////////////////////////////////////////////////////////
    // JavaScript Construction / Destruction
    ///////////////////////////////////////////////////////////////////////////

    static jerry_value_t OnConstructed(const jerry_call_info_t *callInfo,
                                       const jerry_value_t      argv[],
                                       const jerry_length_t     argc)
    {
        jerry_value_t retVal = jerry_undefined();

        if (JerryScript::CalledAsConstructor(callInfo) == false)
        {
            retVal = jerry_throw_sz(JERRY_ERROR_TYPE, "Improperly called as function and not constructor");
        }
        else if (argc != 1 || !jerry_value_is_number(argv[0]))
        {
            retVal = jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid arguments to constructor");
        }
        else
        {
            // Extract parameters
            uint8_t pin = (int)jerry_value_as_number(argv[0]);

            if (pinWhitelistSet_.contains(pin) == false)
            {
                string errMsg = "Invalid pin specified, must be one of: " + pinWhitelistStr_;

                retVal = jerry_throw_sz(JERRY_ERROR_TYPE, errMsg.c_str());
            }
            else
            {
                // Create a new object
                DS18X *obj = new DS18X(pin);

                if (!obj)
                {
                    retVal = jerry_throw_sz(JERRY_ERROR_TYPE, "Failed to allocate memory for object");
                }
                else
                {
                    // Associate the C state with the JS object
                    JerryScript::SetNativePointer(callInfo->this_value, &typeInfo_, obj);
                }
            }
        }

        return retVal;
    }

    static void OnGarbageCollected(void *native, struct jerry_object_native_info_t *callInfo)
    {
        if (native)
        {
            DS18X *obj = (DS18X *)native;

            delete obj;
        }
    }


private:

    // pin whitelist
    static inline unordered_set<uint8_t> pinWhitelistSet_;
    static inline string                 pinWhitelistStr_;

    // JavaScript Type Identifier
    static inline const jerry_object_native_info_t typeInfo_ =
    {
        .free_cb = OnGarbageCollected,
    };
};



