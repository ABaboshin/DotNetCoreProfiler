using Interception.Attributes;
using System.Runtime.CompilerServices;

namespace Interception.Core
{
    public class DefaultInitializer
    {
        [DefaultInitializer]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static T GetDefault<T>() => default;

        static DefaultInitializer() { }
    }
}
