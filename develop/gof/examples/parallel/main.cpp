#include <thread>
#include <numeric>
#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>

template<typename Iterator,typename T>
struct accumulate_block
{
  void operator()(Iterator first,Iterator last,T& result)
  {
    result=std::accumulate(first,last,result);
  }
};

template<typename Iterator,typename T>
void accumulate_block2(Iterator first,Iterator last,T& result)
{
  result=std::accumulate(first,last,result);
}



template<typename Iterator,typename T>
T parallel_accumulate(Iterator first,Iterator last,T init)
{
  unsigned long const length=std::distance(first,last);

  // 若输入数据为空，则返回初始值
  if(!length)
    return init;

  // 计算所需要的最大线程数量，每个线程至少计算25个数据
  unsigned long const min_per_thread=25;
  unsigned long const max_threads=
      (length+min_per_thread-1)/min_per_thread;

  // 获取硬件可并发线程数量
  unsigned long const hardware_threads=
      std::thread::hardware_concurrency();

  // 计算实际要创建的线程数量
  unsigned long const num_threads=
      std::min(hardware_threads!=0?hardware_threads:2,max_threads);

  // 根据线程数量，拆分数据
  unsigned long const block_size=length/num_threads;

  // 创建用于存放每个线程计算结果的容器和线程
  std::vector<T> results(num_threads);
  std::vector<std::thread>  threads(num_threads-1);

  Iterator block_start=first;
  for(unsigned long i=0;i<(num_threads-1);++i)
  {
    Iterator block_end=block_start;
    // 移动迭代器
    std::advance(block_end,block_size);
    // 启动新线程，对一块数据进行处理
    threads[i]=std::thread(accumulate_block2<Iterator,T>,
                           block_start,block_end,std::ref(results[i]));
    // 为下一个线程准备数据
    block_start=block_end;
  }

  // 当启动了所有的子线程对数据进行计算，本线程就对数据的最后一块进行计算
  accumulate_block<Iterator,T>()(block_start,last,results[num_threads-1]);

  // 使用fore_each对所有的线程执行join操作，等待它们执行结束
  std::for_each(threads.begin(),threads.end(),
                std::mem_fn(&std::thread::join));

  std::for_each(results.cbegin(), results.cend(), [](T r) {
    std::cout << r << std::endl;
  });
  // 最后对所有的计算结果求和
  return std::accumulate(results.begin(),results.end(),init);
}

int main()
{
  std::cout << "threads: " << std::thread::hardware_concurrency() << std::endl;
  std::vector<int> vi;
  for(int i=0;i<1000000;++i)
  {
    vi.push_back(i);
  }
  int sum=parallel_accumulate(vi.begin(),vi.end(),5ul);
  std::cout<<"sum="<<sum<<std::endl;
}
