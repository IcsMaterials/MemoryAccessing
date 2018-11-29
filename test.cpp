#include <iostream>
#include <thread>
#include <cmath>
#include <numeric>

#include "Timer.hpp"
#include "Threadpool.hpp"

#define TEST_STRIDED
#define TEST_BLOCKING

using Data_t = double;
#ifndef MEM_SIZE
constexpr size_t MEM_SIZE   = (1ull<<32); // 4GB data
#endif
constexpr size_t MEM_OBJS   = MEM_SIZE / sizeof(Data_t);
constexpr Data_t DATA       = 3.141592e-3;
const     int    THREAD_CNT = std::thread::hardware_concurrency();

Data_t buffer[MEM_OBJS];
Util::Timer timer;
Util::FixedThreadPool pool(THREAD_CNT);

int Reduction(int start, int end, int stride, Data_t *buf)
{
    Data_t value = 0;
    Data_t tmp1, tmp2;
    int unroll = 4;
    int offset1 = stride << 1;
    int offset3 = stride << 2;
    int offset2 = offset1 + stride; 
    stride *= unroll;
    int i = start;
    for(i=start; i<end; i+=stride)
    {
        tmp1 = buf[i] + buf[i + offset1];
        tmp2 = buf[i + offset3] + buf[i + offset2];
        value += tmp1 + tmp2;
    }
    stride /= unroll;
    for(;i<end;i+=stride)
        value += buf[i];

    std::cout<<"G: "<<value<<std::endl;
    return value;
}


int main()
{
    /* inititalize buffer */
    std::fill(buffer, buffer+MEM_OBJS, DATA);

    /* start timer and do the job*/
#ifdef TEST_STRIDED
    {
        std::future<int> fu[THREAD_CNT];
        timer.Start();
        for(int i=0;i<THREAD_CNT;i++)
            fu[i] = pool.enqueue(Reduction, i, MEM_OBJS, 4, buffer);

        Data_t sum = 0;
        for(int i=0;i<THREAD_CNT;i++)
            sum += fu[i].get();
        timer.Pause();
        std::cout<<"Strided: ---------------------"<<std::endl;
        std::cout<<"Get: "<<sum<<" in "<<timer.GetTime()<<" sec"<<std::endl;
    }
#endif

#ifdef TEST_BLOCKING
    std::cout<<std::endl;
    timer.Reset();

    {
        std::future<int> fu[THREAD_CNT];
        timer.Start();
        for(int i=0;i<THREAD_CNT;i++)
            fu[i] = pool.enqueue(Reduction, 0, MEM_OBJS / THREAD_CNT, 1, buffer + i * MEM_OBJS / THREAD_CNT);

        Data_t sum = 0;
        for(int i=0;i<THREAD_CNT;i++)
            sum += fu[i].get();
        timer.Pause();
        std::cout<<"Blocking: ---------------------"<<std::endl;
        std::cout<<"Get: "<<sum<<" in "<<timer.GetTime()<<" sec"<<std::endl;
    }
#endif

    return 0;
}
