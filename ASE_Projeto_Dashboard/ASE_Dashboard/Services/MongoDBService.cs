using MongoDB.Bson;
using MongoDB.Driver.Core.Configuration;
using MongoDB.Driver;
using ASE_Dashboard.Models;

namespace ASE_Dashboard.Services
{
    public class MongoDBService
    {

        private MongoClient client;
        public MongoDBService() {
            client = new MongoClient("mongodb://192.168.160.28:27017");
        }

        public IQueryable<SensorDataTC74> GetAllDataTC74()
        {
            var collection = client.GetDatabase("ase").GetCollection<SensorDataTC74>("tc74_data");
            return collection.AsQueryable();
        }
    }
}
