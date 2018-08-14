using System.Text;
using System.ComponentModel;
using System.Net.Sockets;
using System.Net;

/*使用方法
httptest = new HttpTest(int.Parse(httpcount.Text), this.httphost.Text,int.Parse(this.httpport.Text), this.httppath.Text, this.httpmethod.Text, this.httpcontent.Text);
httptest .start(); 
*/ 
namespace DepthCharge
{
    class HttpTest
    {
        const string NewLine = "\r\n";
        bool run = false;
        BackgroundWorker worker;
        int count;
        string host;
        int port;
        string path;
        string method;
        string content;
        public HttpTest(int count, string host, int port,string path, string method, string content)
        {
           this.count=count;
           this.host = host;
           this.port = port;
           this.path = path;
           this.method = method;
           this.content = content;
        }
 
 
        public void start()
        {
            worker = new BackgroundWorker();
            worker.DoWork += new DoWorkEventHandler(doWork);
            worker.RunWorkerAsync();
            worker.WorkerSupportsCancellation = true;
        }
        public void stop()
        {
            run = false;
            worker.CancelAsync();
        }
        private void doWork(object sender, DoWorkEventArgs e)
        {
            run = true;
            StringBuilder strb = new StringBuilder();
            strb.Append(method + " " + this.path + NewLine);
            strb.Append("HOST: " + this.host + NewLine);
            strb.Append("User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.75 Safari/535.7" + NewLine);
            strb.Append("Accept: */*" + NewLine);
            strb.Append("Connection: keep-alive" + NewLine);
            strb.Append("Referer: http://" + this.host + NewLine);
            strb.Append("HOST: " + this.host + NewLine);
            strb.Append("Accept-Encoding: gzip,deflate" + NewLine);
            strb.Append("Accept-Language: zh-CN,zh;q=0.8" + NewLine);
            if ("POST".Equals(method))
            {
                strb.Append("Content-Length: " + System.Text.Encoding.ASCII.GetBytes(content.ToString()).Length + NewLine);
            }
            strb.Append("Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3" + NewLine);
            strb.Append(NewLine);
            if ("POST".Equals(method))
            {
                strb.Append(content);
            }
            byte[] buf = System.Text.Encoding.ASCII.GetBytes(strb.ToString());
            for (int i = count; i > 0 && run; --i)
            {
                byte[] recvBuf = new byte[64];
                IPAddress ip = IPAddress.Parse(host);
                System.Net.IPEndPoint endp = new System.Net.IPEndPoint(ip, 80);
                var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                socket.Connect(endp);
                socket.Send(buf);
                socket.Receive(recvBuf, 64, SocketFlags.None);
            }
        }
 
        public bool Running { get{return run;} set{run=value;} }
    }
}
