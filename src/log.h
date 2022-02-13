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
            delete m_instance;
            m_instance = NULL;
        };
        static T* Instance()
        {
            if (m_instance == NULL)
            {
                printf("cuantas veces?\n");
                m_instance = new T;
            }

            return m_instance;
        };
    protected:
    private:
        //CSingleton(const CSingleton& source) {};
        static T* m_instance;
};

//! static class member initialisation.
template <typename T> T* CSingleton<T>::m_instance = NULL;

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
