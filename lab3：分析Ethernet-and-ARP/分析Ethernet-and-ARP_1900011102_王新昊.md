# LAB 3: Ethernet & ARP

王新昊 1900011102



### 1.  Capturing and analyzing Ethernet frames

#### 1.1   find the packet numbers of the HTTP GET message that was sent from your computer to gaia.cs.umass.eduas well as the beginning of the HTTP response message sent to your computer by gaia.cs.umass.edu.

如图：

![image-20220402231810209](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220402231810209.png)

我的电脑IP地址是10.2.76.145， 目标IP地址是128.119.245.12。

HTTP GET message的packet号为1621。

HTTP response message的packet号有很多，开头的是1679。





#### 1.2  What is the 48-bit Ethernet address of your computer?

如图：

![image-20220403035057123](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403035057123.png)

48位以太网地址：a4:83:e7:5c:09:d4。





#### 1.3  What is the 48-bit destination address in the Ethernet frame? Is this the Ethernet address of gaia.cs.umass.edu? (Hint: the answer is *no*). What device has this as its Ethernet address? [Note: this is an important question, and one that students sometimes get wrong. Re-read pages 468-469 in the text and make sure you understand the answer here.]

如图：

![image-20220403035605219](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403035605219.png)

48位目的以太网地址：84:b8:02:59:98:c0。

这不是gaia.cs.umass.edu的以太网地址。

![image-20220403035709154](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403035709154.png)

由该图可以知道，应该是路由器拥有这个以太网地址。





#### 1.4  Give the hexadecimal value for the two-byte Frame type field. What upper layer protocol does this correspond to?

如图：

![image-20220403040013296](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403040013296.png)

0x0800。这是IPv4协议。





#### 1.5  How many bytes from the very start of the Ethernet frame does the ASCII “G” in “GET” appear in the Ethernet frame?

如图：

![image-20220403040259384](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403040259384.png)

可以看到，第五行的“47”对应于“GET”中的“G”，在这个字节之前一共有64+2 = 66个字节。





#### 1.6   What is the value of the Ethernet source address? Is this the address of your computer, or of gaia.cs.umass.edu (Hint: the answer is *no*). What device has this as its Ethernet address?

如图：

![image-20220403040916039](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403040916039.png)

Cisco_59:98:c0，即84:b8:02:59:98:c0。

既不是我的电脑的地址，也不睡gaia.cs.umass.edu的地址。

路由器拥有这个以太网地址。





#### 1.7  What is the destination address in the Ethernet frame? Is this the Ethernet address of your computer?

![image-20220403041048531](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403041048531.png)

Apple_5c:09:d4，即a4:83:e7:5c:09:d4。

是。





####1.8  Give the hexadecimal value for the two-byte Frame type field. What upper layer protocol does this correspond to?

如图：

![image-20220403041424123](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403041424123.png)

0x0800。这是IPv4协议。





####1.9  How many bytes from the very start of the Ethernet frame does the ASCII “O” in “OK” (i.e., the HTTP response code) appear in the Ethernet frame?

如图：

![image-20220403041833990](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403041833990.png)

可以看到，第五行的“4f”对应于“Ok”中的“O”，在这个字节之前一共有64+15 = 79个字节。







### 2.  The Address Resolution Protocol

#### 2.1   Write down the contents of your computer’s ARP cache. What is the meaning of each column value?

如图：

![image-20220403042537067](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403042537067.png)

第一列（括号里的）是IP地址。

第二列（at之后的）是物理地址（MAC地址）

第三列（on en0 ifscope）表示这个ARP表项来自en0这个网卡





#### 2.2  What are the hexadecimal values for the source and destination addresses in the Ethernet frame containing the ARP request message?

如图：

![image-20220403050009310](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403050009310.png)

源地址：Apple_5c:09:d4，即a4:83:e7:5c:09:d4。

目的地址：广播Broadcast，即ff:ff:ff:ff:ff:ff。





#### 2.3  Give the hexadecimal value for the two-byte Ethernet Frame type field. What upper layer protocol does this correspond to?

如图：

![image-20220403050128288](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403050128288.png)

0x0806。ARP协议。





#### 2.4  Download the ARP specification from ftp://ftp.rfc-editor.org/in-notes/std/std37.txt. A readable, detailed discussion of ARP is also at http://www.erg.abdn.ac.uk/users/gorry/course/inet-pages/arp.html.

如图：

![image-20220403154331922](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403154331922.png)

##### a) How many bytes from the very beginning of the Ethernet frame does the ARP *opcode* field begin?

如上图。

第二行的00 01是Opcode，表示request。

在这之前一共有20个字节。



#####b) What is the value of the *opcode* field within the ARP-payload part of the Ethernet frame in which an ARP request is made?

同样如上图，可以看到，value = 1（0x0001）。



##### c) Does the ARP message contain the IP address of the sender?

同样如上图。

可以看到，包括sender的MAC address和IP address。



#####d) Where in the ARP request does the “question” appear – the Ethernet address of the machine whose corresponding IP address is being queried?

首先根据Opcode我们可以看到，这个ARP message是 request类型，是一个请求。

第二点，这个ARP message的destination是0xffffffffffff，是一个broadcast。

第三点，可以看到报文中Target MAC address是0x000000000000，说明Target Mac address未知，所以应该是请求MAC address的request message。





#### 2.5  Now find the ARP reply that was sent in response to the ARP request.

如图：

![image-20220403155827640](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403155827640.png)

##### a) How many bytes from the very beginning of the Ethernet frame does the ARP *opcode* field begin?

如图：

![image-20220403155911749](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403155911749.png)

第二行的00 02是Opcode，表示reply。

在这之前一共20个字节。



##### b) What is the value of the *opcode* field within the ARP-payload part of the Ethernet frame in which an ARP response is made?

同样如上图，可以看到value为2（0x0002）。

##### c) Where in the ARP message does the “answer” to the earlier ARP request appear – the IP address of the machine having the Ethernet address whose corresponding IP address is being queried?

首先根据Opcode我们可以看到，这个ARP message是 reply类型，是一个请求。

第二点，这个ARP message的destination是我们主机的MAC address，是一个unicast。

第三点，可以看到报文中Sender IP address是我们请求的IP address，Target IP address是我们主机的IP address，且Sender和Target的MAC address都不为0，说明是对我们请求的回复，所以应该是reply message。





#### 2.6  What are the hexadecimal values for the source and destination addresses in the Ethernet frame containing the ARP reply message?

如图：

![image-20220403160443285](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403160443285.png)

Source address：Cisco_59:98:c0，即84:b8:02:59:98:c0。

Destination address：Apple_5c:09:d4，即a4:83:e7:5c:09:d4。





####2.7  Open the *ethernet-ethereal-trace-1* trace file in http://gaia.cs.umass.edu/wireshark-labs/wireshark-traces.zip. The first and second ARP packets in this trace correspond to an ARP request sent by the computer running Wireshark, and the ARP reply sent to the computer running Wireshark by the computer with the ARP-requested Ethernet address. But there is yet another computer on this network, as indicated by packet 6 – another ARP request. Why is there no ARP reply (sent in response to the ARP request in packet 6) in the packet trace?

下图是packet 6的内容：

![image-20220403161327187](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403161327187.png)

下图是packet 1的内容：

![image-20220403161402574](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403161402574.png)

可以看到，packet 1的sender ip address是192.168.1.105，而target ip address是192.168.1.1，是一个 request message。packet 2则是192.168.1.1发送给192.168.1.105的reply message。而packet 6的target ip address是192.168.1.117，且sender ip address是192.168.1.104不是我们主机，所以我们主机不会收到packet 6的reply message，只会收到packet 1的reply message。因为request message是broadcast的，所有主机都可以收到，但reply message是unicast的，只有发送request message的主机才可以收到。







### 3.  Extra Credit

#### 3.1  The *arp* command:  `arp -s InetAddr EtherAddr` allows you to manually add an entry to the ARP cache that resolves the IP address *InetAddr* to the physical address *EtherAddr*. What would happen if, when you manually added an entry, you entered the correct IP address, but the wrong Ethernet address for that remote interface?

第一步，如图，我们通过`arp -a`找到我们的arp缓存。

![image-20220403164141988](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403164141988.png)

第二步，我们通过`arp -s InetAddr EtherAddr`修改10.2.64.1的address。

![image-20220403164650439](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403164650439.png)

第三步，我们通过`arp -d 10.2.64.1 ifscope en0`删掉正确的映射缓存（macOS）。

![image-20220403164735653](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403164735653.png)

第四步，让我们打开网页看看：

![image-20220403164831509](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403164831509.png)

可以看到，由于修改了ARP缓存，我们已经无法浏览网页。解释如下：

![image-20220403035709154](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403035709154.png)

根据这个图我们可以看到，MAC帧是从我们的主机发送到主机所在子网的路由器，再通过路由器向外发送，我们修改的10.2.64.1可能是子网的路由器的ip地址，由于其对应的MAC地址缓存被修改，导致MAC帧无法正确发送到路由器，也就无法向外发送。





####3.2  What is the default amount of time that an entry remains in your ARP cache before being removed. You can determine this empirically (by monitoring the cache contents) or by looking this up in your operation system documentation. Indicate how/where you determined this value.

不同的系统有不同的ARP cache保存时间。

经过查阅资料知道了三种查看refresh time的方式：

* Windows系统：可以通过`netsh interface ipv4 show interfaces`得到各个接口信息，再通过`netsh interface ipv4 show interface <interface ID from step 1> `查看每一个接口的reachable time，示例如图：

![image-20220403172750451](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403172750451.png)

可以看到可访问时间为39500ms。

* Ubuntu系统：可以通过`cat/proc/sys/net/ipv4/neigh/default/gc_stale_time`查看，默认值一般是60，表示60s。

如图：

![image-20220403173215633](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403173215633.png)

* macOS系统：据说和OpenBSD的arp相同，我没有查到macOS的文档，但OpenBSD中明确指出了arp缓存timeout的时间。如图：

![image-20220403173345874](/Users/wangxinhao/Documents/大三下/计网/lab/lab3/分析Ethernet-and-ARP_1900011102_王新昊/image-20220403173345874.png)

正常来说是20min。

