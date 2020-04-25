using System.Runtime.CompilerServices;
using System.Threading.Tasks;

namespace Interception.StackExchangeRedis
{
    public static class TaskExtensions
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ConfiguredValueTaskAwaitable<T> ForAwait<T>(this in ValueTask<T> task) => task.ConfigureAwait(false);
    }
}
