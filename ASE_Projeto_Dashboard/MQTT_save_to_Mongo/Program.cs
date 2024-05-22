

using MQTTnet.Client;
using MQTTnet;
using ASE_Dashboard.Models;
using Newtonsoft.Json;
using System.Text;
using ASE_Dashboard.Services;

class Program
{

    static string broker = "192.168.160.49";
    static int port = 1883;
    static string clientId = Guid.NewGuid().ToString();
    static string topic = "sensor/tc74"; // part that matters
    static MongoDBService m = new ();

    static void Main()
    {
        Thread[] thread = { new Thread(new ThreadStart(Worker))};

        // Do some work in the main thread
        for (int i = 0; i < 1; i++)
        {
            thread[i].Start();
        }
        // Do some work in the main thread
        for (int i = 0; i < 1; i++)
        {
            thread[i].Join();
        }
        // Wait for the worker thread to finish
        while (true)
            Thread.Sleep(100);
    }



    static async void Worker()
    {
        // Create a MQTT client factory
        var factory = new MqttFactory();

        // Create a MQTT client instance
        var mqttClient = factory.CreateMqttClient();

        SensorDataTC74 d;
        // Create MQTT client options
        var options = new MqttClientOptionsBuilder()
            .WithTcpServer(broker, port) // MQTT broker address and port
            .WithClientId(clientId)
            .WithCleanSession().WithProtocolVersion(MQTTnet.Formatter.MqttProtocolVersion.V500) // part that matters
            .Build();
        var connectResult = await mqttClient.ConnectAsync(options);
        if (connectResult.ResultCode == MqttClientConnectResultCode.Success)
        {
            Console.WriteLine("Connected to MQTT broker successfully.");

            // Subscribe to a topic
            await mqttClient.SubscribeAsync(topic);

            // Callback function when a message is received
            mqttClient.ApplicationMessageReceivedAsync += e =>
            {
                d = JsonConvert.DeserializeObject<SensorDataTC74>(Encoding.UTF8.GetString(e.ApplicationMessage.PayloadSegment));
                m.SaveDataTC74(d);
                
                return Task.CompletedTask;
            };
        }

        while (true)
            Thread.Sleep(100);
    }
}