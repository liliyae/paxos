# paxos
云计算实验 设计并实现Paxos算法的写入功能 系统包含2个Proposor和3个Acceptor
1.	整体思想</br>
实验采用socket套接字加共享内存的方式完成。
*	三个acceptor监听三个端口，两个proposer根据多数派读写原则，每个proposer向两个acceptor通过socket建立tcp连接，发送接受消息。
	由于socket套接字采用多线程的方式建立连接，为了保证每个节点数据一致性，采用共享内存的方式为所有acceptor的vrnd、lastrnd、v创建存储空间。由于一个acceptor可能与多个proposer相连，所以建立连接的过程是动态的，即每次使用完就关闭，acceptor循环连接新的proposer请求。
*	不同acceptor的vrnd、lastrnd、v是相互独立的，所以不能像proposer代码一样共用一份代码，因为acceptor采用了共享内存，这会导致多次运行一份代码下，多进程之间，相同key的vrnd、lastrnd、v是共享的，解决方法可以是为根据传入的编号创建key不同的共享内存，或者分成三个acceptor的代码，不同代码建立的共享内存key不同。
*	不同的proposer需要保证rnd全局唯一且单调递增，所以创建一个共享内存变量currentrnd，用于存储当前轮中已经使用到的最大rnd，从而生成本次proposer的rnd。
*	当某个proposer第二轮写失败后，可以更改rnd重新申请读写，并将要写入的v改为读到的acceptor中最大lastrnd的v（替别人写）

2.	流程图</br>

![啊啊](https://user-images.githubusercontent.com/58354216/131122777-8d54e965-a1d8-4933-83e9-22e05f594b4f.png)



3.	遇到的问题及解决方案</br>
实验过程中遇到问题的解决方案如下，设计为下述模式主要考虑到以下几点：
*	为什么要Socket动态建立，每次传递消息后关闭连接？</br>
因为同一个proposer第一轮和第二轮之间可能会有较大的时间差，对于socket线程池可容纳的线程数是有上限的，当高并发的时候，为了防止线程过多导致崩溃，最好的方式就是每轮传递数据时建立连接，传递完毕后释放连接。</br>
*	为什么要使用共享内存？</br>
同一个acceptor的多线程之间保证数据一致性，对于每个acceptor来说，可能有多个proposer同时访问，socket采用多线程的机制为每个请求建立对应的连接，由于线程的同步问题以及线程使用完会直接释放，如果不采用共享内存，不同线程的lastrnd、v、vrnd是不一致的，例如proposer1写入时，更改了lastrnd、v、vrnd，不采用共内存，则写入后线程结束后进程的lastrnd、v、vrnd不会被更新，则proposer2写入时仍然认为是空的，可以直接写入，导致错误。所以必须使用共享内存。</br>
*	不同acceptor进程之间数据不相关，注意设置不同的共享内存的key值</br>
刚开始我是采用了同一个acceptor.cpp，传参不同进行区分acceptor，但是问题是采用共享内存后，不同acceptor进程的lastrnd、v、vrnd也被共享为一个，也就是说共享内存不是仅存在于一个进程的多线程之间，而是存在于操作系统层的全局的内存空间，多进程之间也共享，解决方式是设置多个共享内存单元，对应不同的acceptor，key值唯一标识他们。</br>
