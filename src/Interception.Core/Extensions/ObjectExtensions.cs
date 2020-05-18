using System.Reflection;

namespace Interception.Core.Extensions
{
    public static class ObjectExtensions
    {
        public static bool TryGetPropertyValue<T>(this object obj, string propertyName, out T value)
        {
            if (obj != null)
            {
                var objType = obj.GetType();
                var propertyInfo = objType.GetProperty(propertyName, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.IgnoreCase | BindingFlags.NonPublic);
                if (propertyInfo != null)
                {
                    value = (T)propertyInfo.GetValue(obj);
                    return true;
                }
            }

            value = default;
            return false;
        }

        public static bool TryGetFieldValue<T>(this object obj, string propertyName, out T value)
        {
            if (obj != null)
            {
                var objType = obj.GetType();
                var propertyInfo = objType.GetField(propertyName, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.IgnoreCase | BindingFlags.NonPublic);
                if (propertyInfo != null)
                {
                    value = (T)propertyInfo.GetValue(obj);
                    return true;
                }
            }

            value = default;
            return false;
        }

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
