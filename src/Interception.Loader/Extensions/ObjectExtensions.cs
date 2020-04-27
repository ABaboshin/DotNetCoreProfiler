using System.Reflection;

namespace Interception.Extensions
{
    public static class ObjectExtensions
    {
        public static T GetPropertyValue<T>(this object obj, string propertyName)
        {
            if (obj != null)
            {
                var objType = obj.GetType();
                var propertyInfo = objType.GetProperty(propertyName, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.IgnoreCase | BindingFlags.NonPublic);
                if (propertyInfo != null)
                {
                    return (T)propertyInfo.GetValue(obj);
                }
            }

            return default;
        }
    }
}
