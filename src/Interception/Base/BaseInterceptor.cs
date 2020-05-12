using Interception.Base.Extensions;
using System;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Threading.Tasks;

namespace Interception.Base
{
    /// <summary>
    /// base interceptor
    /// </summary>
    public abstract class BaseInterceptor : IInterceptor
    {
        protected IMethodFinder _methodFinder = new MethodFinder();

        protected object[] _parameters;

        protected object _this;

        protected int _mdToken;

        protected long _moduleVersionPtr;

        public void SetThis(object _this)
        {
            Console.WriteLine($"SetThis {_this}");
            this._this = _this;
        }

        public void AddParameter(int num, object value)
        {
            Console.WriteLine($"AddParameter {num} {value}");

            if (value is null)
            {
                Console.WriteLine("value is null");
            }
            else
            {
                Console.WriteLine("value is not null");
            }

            _parameters[num] = value;
        }

        public void SetMdToken(int mdToken)
        {
            Console.WriteLine($"SetMdToken {mdToken}");
            _mdToken = mdToken;
        }

        public void SetModuleVersionPtr(long moduleVersionPtr)
        {
            Console.WriteLine($"SetModuleVersionPtr {moduleVersionPtr}");
            _moduleVersionPtr = moduleVersionPtr;
        }

        public object GetParameter(int num)
        {
            return _parameters[num];
        }

        public void SetArgumentNumber(int number)
        {
            _parameters = new object[number];
        }

        public virtual object Execute()
        {
            var method = FindMethod();
            var isAsync = method.IsReturnTypeTask();

            if (!isAsync)
            {
                return ExecuteSyncInternal();
            }
            else if (!method.IsReturnTypeTaskWithResult())
            {
                return ExecuteAsyncInternal();
            }
            else
            {
                return ExecuteAsyncWithResultInternal();
            }
        }

        protected abstract void ExecuteBefore();

        protected abstract void ExecuteAfter(object result, Exception exception);

        protected virtual MethodInfo FindMethod()
        {
            return (MethodInfo)_methodFinder.FindMethod(_mdToken, _moduleVersionPtr);
        }

        protected object ExecuteSyncInternal()
        {
            var method = FindMethod();

            try
            {
                ExecuteBefore();
                var result = method.Invoke(_this, _parameters);
                ExecuteAfter(result, null);
                return result;
            }
            catch (Exception ex)
            {
                ExecuteAfter(null, ex);
                throw;
            }
        }

        protected async Task ExecuteAsyncInternal()
        {
            Console.WriteLine("ExecuteAsyncInternal");
            var method = FindMethod();

            try
            {
                ExecuteBefore();
                var result = (Task)method.Invoke(_this, _parameters);
                await result;
                ExecuteAfter(result, null);
            }
            catch (Exception ex)
            {
                ExecuteAfter(null, ex);
                throw;
            }
        }

        internal static readonly ModuleBuilder Module;

        static BaseInterceptor()
        {
            var asm = AssemblyBuilder.DefineDynamicAssembly(new AssemblyName("Interception"), AssemblyBuilderAccess.Run);
            Module = asm.DefineDynamicModule("DynamicModule");
        }

        protected object ExecuteAsyncWithResultInternal()
        {
            var method = FindMethod();

            var underilyingType = ((TypeInfo)method.ReturnType).GenericTypeArguments[0];

            var createExecutionDelegateGeneric = typeof(BaseInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(CreateExecutionDelegate)).First();
            var createExecutionDelegate = createExecutionDelegateGeneric.MakeGenericMethod(underilyingType);
            var executionDelegate = createExecutionDelegate.Invoke(this, new object[] { });

            var executeWithMetricsGeneric = typeof(BaseInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(ExecuteWithMetrics)).First();
            var executeWithMetrics = executeWithMetricsGeneric.MakeGenericMethod(underilyingType);

            return executeWithMetrics.Invoke(this, new object[] { executionDelegate });
        }

        private async Task<T> ExecuteWithMetrics<T>(Delegate funcDelegate)
        {
            try
            {
                ExecuteBefore();

                var func = (Func<Task<T>>)funcDelegate;
                var result = await func.Invoke();

                ExecuteAfter(result, null);

                return result;
            }
            catch (Exception ex)
            {
                ExecuteAfter(null, ex);
                throw;
            }
        }

        private Delegate CreateExecutionDelegate<T>()
        {
            var method = FindMethod();

            Func<Task<T>> func = () => {
                return (Task<T>)method.Invoke(_this, _parameters);
            };

            return func;
        }
    }
}
