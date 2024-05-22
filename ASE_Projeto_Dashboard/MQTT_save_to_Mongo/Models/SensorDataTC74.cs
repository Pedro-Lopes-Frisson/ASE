using MongoDB.Bson.Serialization.Attributes;
using MongoDB.Bson;
using System.Text.Json.Serialization;

namespace ASE_Dashboard.Models
{
    public class SensorDataTC74
    {
        public ObjectId Id { get; set; }
        
        [BsonElement("temperature")]
        public float Temperature { get; set; }

        [BsonElement("sensor_id")]
        public string SensorID { get; set; }

        [BsonElement("timestamp")]
        public string Timestamp { get; set; }

    }
}
