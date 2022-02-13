#ifndef _LOG_H_
#define _LOG_H_

#include "pch.h"

#ifdef __WIN32__
    #include <time.h>
#endif

#define WoWParserLogOutPut "wowparser3.log"

template <typename T>
class CSingleton
{
    public:
        CSingleton()
        {
            printf("cuantas veces se creo CSingleton\n");
        };
        ~CSingleton()
        {
            printf("cuantas destruimos singleton\n");
        };
        static T* Instance()
        {
            if (!m_instance.get())
            {
                printf("cuantas veces?\n");
                m_instance = std::auto_ptr<T>(new T);
            }

            return m_instance.get();
        };
    protected:
    private:
        //CSingleton(const CSingleton& source) {};
        static std::auto_ptr<T> m_instance;
};

//! static class member initialisation.
//template <typename T> T* CSingleton<T>::m_instance = NULL;
template <class T> std::auto_ptr<T> CSingleton<T>::m_instance;

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
