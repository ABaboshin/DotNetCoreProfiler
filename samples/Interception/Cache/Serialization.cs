using System.IO;
using System.Runtime.Serialization.Formatters.Binary;

namespace Interception.Cache
{
    public static class Serialization
    {
        public static byte[] ToByteArray(this object obj)
        {
            if (obj == null)
            {
                return null;
            }
            var binaryFormatter = new BinaryFormatter();
            using (var memoryStream = new MemoryStream())
            {
                binaryFormatter.Serialize(memoryStream, obj);
                return memoryStream.ToArray();
            }
        }

        public static object FromByteArray(this byte[] byteArray)
        {
            if (byteArray == null)
            {
                return null;
            }

            var binaryFormatter = new BinaryFormatter();
            using (var memoryStream = new MemoryStream(byteArray))
            {
                return binaryFormatter.Deserialize(memoryStream);
            }
        }
    }
}
