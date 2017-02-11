# AManager-for-hard-drive-agents
Socket programming--a manager written in Java, manages the agents implemented in c that are responsible for telling manager abt corresponding hard drives' info like whether hard drives are broken, their info, and etc.


####  READ ME  #### 
There are two files - one named UDPServer.java(Manager), the other named client.c(Agent)
The test running environment is under the MacOS terminal.
Before run it, plz compile them. gcc -o client client.c  & javac UDPServer.java for your convenience.
When run client.c in mac terminal, it needs input like ./client 127.0.0.1 7777
                                                        ./client <IP> <Port>
When run UDPServer.java in mac terminal, it only needs input like java UDPServer 7777
                                                        java UDPServer <Port>



####  Specification  ####
In UDPServer, there are 3 functional separate threads: (Output info. will be described below)
      1.BeaconListener--used to receive UDP msg & add new agent to list & If it receives the same ID agent but
                        with different StartUpTime, just update its received time and inform users that such
                        agent resurrects. If the agent already been deleted from list, then it will inform users
                        such agent re added to the agent list.
      2.AgentMonitor  --used to check whether there are some agents offline for larger than 15(can be defined)
                        secs. Then it will be deleted from list. Every 5 sec(can be defined), such thread will
                        run to check offline agent.
      3.ClientAgent   --used to send TCP command msg to agent side, when UDP msg from agent is found a new agent.
                        to ask for agent's OS info and its local time.
BTW, in UDPServer, I use CopyOnWriteArrayList(<BACON>) which is similar with Arraylist to store info passed from
      agent. Such structure won't meet exceptions like ConcurrentModificationException when I use multithread to
      iterate items and do some operations on Arraylist. And use "public static" to make such ArrayList be shared
      by all classes.


In client.c (Agent), I use the main thread for sending the UDP msg to manager per 5 secs.(Although professor ask
      for creating two threads: one for sending udp, one for receiving TCP from manager and response, I use main
      thread to do sending udp part. There is no distinction. Just for saving little time)
      And I send a string that contains info such as IP, ID, StartUpTime, and Port to manager. Manager will know
      how to tell these agents by looking at IP & port.
