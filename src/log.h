#ifndef _LOG_H_
#define _LOG_H_

#include "pch.h"

#ifdef __WIN32__
    #include <time.h>
#endif

#define WoWParserLogOutPut "wowparser3.log"

#include <memory>

using namespace std;

template <typename T>
class CSingleton
{
    public:
        static T* Instance()
        {
            if (!m_instance.get())
                m_instance = auto_ptr<T>(new T);

            return m_instance.get();
        };
    protected:
        //CSingleton();
        //~CSingleton();
    private:
        //CSingleton(CSingleton const&);
        //CSingleton& operator = (CSingleton const*);
        static auto_ptr<T> m_instance;
};

template <typename T> auto_ptr<T> CSingleton<T>::m_instance;

class Log
{
    public:
        Log();
        void WriteLog(const char* args, ...);
        void WriteLogNoTime(const char* args, ...);
        void WriteLogAndPrint(const char* args, ...);
};

#define sLog CSingleton<Log>::Instance()
#endif
