import java.io.*;
import java.net.*;
import java.lang.*;
import java.util.*;
import java.sql.*;
import java.math.*;
import java.util.ArrayList;
import java.nio.charset.Charset;
import java.util.concurrent.CopyOnWriteArrayList;

public class UDPServer
{
   public static String sentence;
   public static List<BACON> list = new CopyOnWriteArrayList<BACON>();
   // arraylist is pass by reference thus you could use it in another class. by adding public static

   public static void main(String args[]) throws Exception
      {
        if (args.length != 1) {
            System.err.println("Usage: java EchoServer <port number>");
            System.exit(1);
        }
         DatagramSocket serverSocket = new DatagramSocket(Integer.parseInt(args[0]));
            byte[] receiveData = new byte[1024];
            byte[] sendData = new byte[1024];
            while(true)
               {
                 byte[] buffer = new byte[1024];
                 DatagramPacket receivePacket = new DatagramPacket(buffer, buffer.length);
                 //DatagramSocket ds = new DatagramSocket(2345);
                 serverSocket.receive(receivePacket);
                 byte[] data = receivePacket.getData();
                 String sentence = new String(data, 0, receivePacket.getLength());
                 System.out.println("Port " +  receivePacket.getPort()  + /*
                              " on "  +  receivePacket.getAddress()  +    */
                              " sent this message:");
                 //System.out.println(sentence+"length:"+sentence.length());



                  //serverSocket.receive(receivePacket);
                  long timeReceived = System.currentTimeMillis() / 1000l;
                  //sentence = new String( receivePacket.getData());
                  //System.out.println(timeReceived);
                  //System.out.println("RECEIVED: " + sentence);
                  //delete_broken_Agent_from_list(); //1.thread to run;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                  new BeaconListener(sentence,timeReceived).start(); // when there is a
                  //addAgent(sentence,timeReceived); //2. thread to run with while to receive function
                  new AgentMonitor().start(); //delete broken agent
                  //setAgent_UDPmsgReceivedTime(sentence,timeReceived);// write in 2.thread
                  //for (BACON agent : list) {
                    //  System.out.println("id:"+agent.getBaconID() +"time:"+agent.lastBaconReceivedTime());
                  //}

                  InetAddress IPAddress = receivePacket.getAddress();
                  int port = receivePacket.getPort();
                  String capitalizedSentence = sentence.toUpperCase();
                  sendData = capitalizedSentence.getBytes();
                  DatagramPacket sendPacket =
                  new DatagramPacket(sendData, sendData.length, IPAddress, port);
                  serverSocket.send(sendPacket);
               }

      }

}

/*-------------------------------BeaconListener Thread-------------------------*/
/*-------------------------------BeaconListener Thread-------------------------*/
/*-------------------------------BeaconListener Thread-------------------------*/
class BeaconListener /*(String sentence, long timeReceived)*/ extends Thread{
  UDPServer udpserver = new UDPServer();
  AgentMonitor monitor = new AgentMonitor();
  String sentence;
  long timeReceived;
  BeaconListener(String sentence,long timeReceived){
    this.sentence = sentence;
    this.timeReceived = timeReceived;
  }

  public void run(){
      addAgent(sentence,timeReceived);
  }
  public void addAgent(String sentence, long timeReceived){ //ok in the thread
    String baconMsg = sentence;
    //System.out.println(baconMsg);
    String[] bacon = baconMsg.split(","); // ID,StartUpTime,IP,CmdPort  只用，分割好使  $$不好使
    String ID = bacon[0]; //System.out.println(bacon[0]);
    String StartUpTime = bacon[1];//System.out.println(bacon[1]);
    String IP = bacon[3];//System.out.println(IP);
    String CmdPort = bacon[2];
    int old_new_agent = checkAgent_exist_ornot(ID,StartUpTime);
    //System.out.println(IP);
    if (old_new_agent == 1) { //agent id & StartUpTime does not exist in agent list----> so new agent
      if (check_whether_agent_restart(ID,StartUpTime) == 1) {
          System.out.println("agent ID: "+ID+"resurrects");
           //把startuptime时间设回到agent list 里面 不会再提示resurrect,写在check_whether_agent_restart里面了
          setAgent_UDPmsgReceivedTime(sentence,timeReceived); //port no. 懒得reset了
      }else{
          if(check_deleted_then_restart(ID)==1){
              System.out.println("The agent ID: "+ID+" has been deleted and then restarts, add to agent list");
          }else{
              System.out.println("New agent Stored in manager list,\n id :" + ID+" || StartUpTime(sec since 1970.1.1):"+StartUpTime);
          }
          int sizeList = udpserver.list.size();
          udpserver.list.add(new BACON());
          BACON agent = (BACON)udpserver.list.get(sizeList);
          agent.setBaconID(ID);
          agent.setBaconIP(IP);
          agent.setBaconStartUpTime(StartUpTime);
          agent.setBaconPort(CmdPort);
          agent.setBaconReceivedTime(Long.toString(timeReceived));
      //System.out.println(list.get(sizeList).lastBaconReceivedTime());
        sizeList++;

      /*--------if a new agent-------In BeaconListener, spawn a agent clientthread -------*/
      //byte[] bytes = agent.getBaconPort().getBytes();
      //String port = new String(bytes, Charset.forName("UTF-8"));
      //System.out.println("Port number is:"+agent.getBaconPort()+"port length:"+agent.getBaconPort().length());
          new ClientAgent(Integer.parseInt(agent.getBaconPort()),IP).start();
      }
    }else{
      setAgent_UDPmsgReceivedTime(sentence,timeReceived);
    }
  }
  public  int checkAgent_exist_ornot(String ID,String StartUpTime){ // ok in thread
    for (BACON agent : udpserver.list) {
         if (agent.getBaconID().equals(ID) && agent.getBaconStartUpTime().equals(StartUpTime)) return 0;
    }
    return 1;
  }

  public int check_whether_agent_restart(String ID,String StartUpTime){
    for (BACON agent : udpserver.list) {
         if (agent.getBaconID().equals(ID) && !agent.getBaconStartUpTime().equals(StartUpTime)){
            agent.setBaconStartUpTime(StartUpTime);
            return 1;
          }
    }
    return 0;
  }

  public int check_deleted_then_restart(String ID){
    for (BACON agent : monitor.deletelist) {
         if (agent.getBaconID().equals(ID)) return 1;
    }
    return 0;
  }

  public void setAgent_UDPmsgReceivedTime(String sentence,long timeReceived){
    String baconMsg = sentence;
    String[] bacon = baconMsg.split(",");
    for (int i = 0; i < udpserver.list.size(); i++) {
        if (udpserver.list.get(i).getBaconID().equals(bacon[0])/* && udpserver.list.get(i).getBaconStartUpTime().equals(bacon[1])*/){
            udpserver.list.get(i).setBaconReceivedTime(Long.toString(timeReceived));
            break;}
    }
  }

}


/*-------------------------------AgentMonitor Thread-------------------------*/
/*-------------------------------AgentMonitor Thread-------------------------*/
/*-------------------------------AgentMonitor Thread-------------------------*/
class AgentMonitor extends Thread
{
  public static List<BACON> deletelist = new CopyOnWriteArrayList<BACON>();
  UDPServer udpserver = new UDPServer();
  public void run(){
      while(true){
          try{
            delete_broken_Agent_from_list();
            //System.out.println("run delete!");
            Thread.sleep(50000); //2 sec for checking whether there is a broken hard drive.
          }
          catch(InterruptedException e)
          {
            System.out.println("my thread interrupted");
          }
      }
  }
  public void delete_broken_Agent_from_list(){ //complete
    for (int i = 0; i < udpserver.list.size(); i++) {
         //System.out.println(System.currentTimeMillis() / 1000l - Long.parseLong(udpserver.list.get(i).lastBaconReceivedTime()));
         int sizeList = deletelist.size();
         if (udpserver.list.get(i).lastBaconReceivedTime()!=null &&!udpserver.list.get(i).lastBaconReceivedTime().isEmpty()){
           if ((System.currentTimeMillis() / 1000l - Long.parseLong(udpserver.list.get(i).lastBaconReceivedTime())) > 15 ){
              System.out.println("already delete one broken drive with StartUpTime:"+udpserver.list.get(i).getBaconStartUpTime() +"from agent list");
              deletelist.add(new BACON());
              BACON new1 = (BACON)deletelist.get(sizeList);
              new1.setBaconID(udpserver.list.get(i).getBaconID());
              new1.setBaconStartUpTime(udpserver.list.get(i).getBaconStartUpTime());
              udpserver.list.remove(i);
              /* when there is a time period > 5 sec without receiving any udp msg from the particular agent.
                  we assume that it's broken. Then we send msg back to agent to order for changing a drive.
              */
              //System.out.println(deletelist.get(sizeList).getBaconStartUpTime());
            }
         }
    }
  }
}


class ClientAgent extends Thread
{
  public int portno;
  public String ip4;
  ClientAgent(int CmdPort, String IP){
      portno = CmdPort;
      ip4 = IP;
      //System.out.println(portno);
  }
  public void run(){
    String sentence;
    try{
      //int port = Integer.parseInt(portno);
      //System.out.println(port);

      for(int i=0; i<2; i++){
          Socket clientSocket = new Socket("localhost", portno);
          BufferedReader reader = new BufferedReader(
  				    new InputStreamReader(clientSocket.getInputStream()));//收os info
    	    PrintStream pstream = new PrintStream(clientSocket.getOutputStream());//传cmd
          if(i == 0){
              pstream.println("GetLocalOS()");
              while ((sentence = reader.readLine()) != null)
              {
                  System.out.println("The new agent OS info: "+sentence);
              }
          }else{
              pstream.println("GetLocalTime()");
              while ((sentence = reader.readLine()) != null)
              {
                  System.out.println("The new agent's local time is: "+sentence);
              }
          }
        //clientSocket.close();
      }
    }
    catch(Exception e){
      System.err.println("Error:" + e);
    }
  }
}



//-------------------------------BACON class-----------------------------------
//-------------------------------BACON class-----------------------------------
//-------------------------------BACON class-----------------------------------
class BACON {
  String ID;
  String StartUpTime;
  String IP;
  String CmdPort;
  String bacon_received_time;

  void setBaconID(String id){
      this.ID = id;
  }
  void setBaconIP(String ip){
      this.IP = ip;
  }
  void setBaconPort(String port){
      this.CmdPort = port;
  }
  void setBaconStartUpTime(String time){
      this.StartUpTime = time;
  }
  void setBaconReceivedTime(String rtime){
      this.bacon_received_time = rtime;
  }
  String getBaconID(){
      return this.ID;
  }
  String getBaconIP(){
      return this.IP;
  }
  String getBaconStartUpTime(){
      return this.StartUpTime;
  }
  String getBaconPort(){
      return this.CmdPort;
  }
  String lastBaconReceivedTime(){
      return this.bacon_received_time;
  }

}
